#include "inc/smtp.h"
#include "inc/logger.h"

#include <assert.h>


SmtpState* make_states(int amount) {
    SmtpState* states = (SmtpState*) malloc(sizeof(SmtpState) * amount);
    if (states == NULL) {
        return NULL;
    }
    for (int I = 0; I < amount; I++) {
        states[I].status = NEW;
        states[I].msg = NULL;
    }
    return states;
}

void destroy_states(SmtpState* states) {
    if (states != NULL) {
        if (states->msg != NULL) {
            destroy_message(states->msg);
        }
        free(states);
    }
}

void handle_welcome(SmtpState *state, char *output) {
    const char *app_name;
    config_lookup_string(&cfg, "smtp.name", &app_name);

    strcpy(output, "220 ");
    strcat(output, app_name);
    strcat(output, " ready\r\n");
    state->status = READY;
}

int check_regex(char *regex, char *src, char *param) {
    regex_t regexObj;
    int res_code = regcomp(&regexObj, regex, REG_EXTENDED | REG_ICASE);
    if (res_code != 0) {
        write_log("Could not compile regex");
        printf("Could not compile regex %s\n", regex);
        return 1;
    }

    regmatch_t matchedGroups[2];
    char *cursor = src;
    res_code = regexec(&regexObj, cursor, 2, matchedGroups, 0);
    if (res_code != 0) {
        write_log("No match found");
        printf("No match with %s\n", regex);
        return 1;
    }

    strpart(src, param, matchedGroups[1]); 
    if (VERBOSE) {
        printf("Extracted %s\n", param);
    }

    regfree(&regexObj);
    return 0;
}

int check_command(char *command, char *src) {
    return strncasecmp(src, command, strlen(command));
}

void smtp_response(int code, char *output) {
    char *response = "";
    if (code == 221) {
        response = "221 Closing transmission channel\r\n";
    }
    else if (code == 250) {
        response = "250 OK\r\n";
    }
    else if (code == 354) {
        response = "354 Start mail input; end with <CRLF>.<CRLF>\r\n";
    }
    else if (code == 451) {
        response = "451 Requested action aborted: error in processing\r\n";
    }
    else if (code == 455) {
        response = "455 Server unable to accommodate parameters\r\n";
    }
    else if (code == 500) {
        response = "500 Invalid command\r\n";
    }
    else if (code == 501) {
        response = "501 Invalid argument\r\n";
    }
    else if (code == 502) {
        response = "502 Not implemented\r\n";
    }
    else if (code == 503) {
        response = "503 Bad sequence of commands\r\n";
    }
    else if (code == 550) {
        response = "550 Sender unknown\r\n";
    }
    else if (code == 552) {
        response = "552 Requested mail action aborted: exceeded storage allocation\r\n";
    }
    strncpy(output, response, 255);
}

int handle_request(SmtpState *state, char *input, char *output) {
    if (state->status == GET_DATA) {
        if (check_command(".\r\n", input) == 0) {
            int res_code = save_maildir(state->msg);
            if (res_code == 0) {
                state->status = READY;
                smtp_response(250, output);
            }
            else {
                smtp_response(451, output);
            }
        }
        else {
            append_message(state->msg, input);
        }
        return 0;
    }

    if (check_command("NOOP", input) == 0) {
        smtp_response(250, output);
    }
    else if (check_command("RSET", input) == 0) {
        if (state->msg != NULL) {
            destroy_message(state->msg);
            state->msg = NULL;
        }
        state->status = READY;
        smtp_response(250, output);
    }
    else if (check_command("QUIT", input) == 0) {
        if (state->msg != NULL) {
            destroy_message(state->msg);
            state->msg = NULL;
        }
        smtp_response(221, output);
        return 1;
    }
    else if (check_command("HELO", input) == 0) {
        if (state->status == READY) {
            char domain[100];

            if (check_regex("HELO ([a-zA-Z0-9\.]+)", input, domain) == 0) {
                if (state->msg != NULL) {
                    destroy_message(state->msg);
                }
                state->msg = make_message();
                set_domain(state->msg, domain);
                smtp_response(250, output);
                state->status = NEED_SENDER;
            }
            else {
                smtp_response(501, output);
            }
        }
        else {
            smtp_response(503, output);
        }
    }
    else if (check_command("EHLO", input) == 0) {
        if (state->status == READY || state->status == NEED_SENDER) {
            char domain[100];

            if (check_regex("EHLO ([a-zA-Z0-9\.]+)", input, domain) == 0) {
                if (state->msg != NULL) {
                    destroy_message(state->msg);
                }
                state->msg = make_message();
                set_domain(state->msg, domain);

                const char *app_name;
                config_lookup_string(&cfg, "smtp.name", &app_name);

                strcpy(output, "250-");
                strcat(output, app_name);
                strcat(output, "\r\n250-PIPELINING\r\n250-8BITMIME\r\n250-VRFY\r\n");
                state->status = NEED_SENDER;
            }
            else {
                smtp_response(501, output);
            }
        }
        else {
            smtp_response(503, output);
        }
    }
    else if (check_command("MAIL FROM", input) == 0) {
        if (state->status == NEED_SENDER) {
            char sender[100];

            if (check_regex("MAIL FROM:<([a-zA-Z0-9\.@\-_]+)>", input, sender) == 0) {
                if (check_user(sender, NULL) == 0) {
                    set_sender(state->msg, sender);
                    smtp_response(250, output);
                    state->status = NEED_RECIPIENT;
                }
                else {
                    smtp_response(550, output);
                }
            }
            else {
                smtp_response(501, output);
            }
        }
        else {
            smtp_response(503, output);
        }
    }
    else if (check_command("RCPT TO", input) == 0) {
        if (state->status == NEED_RECIPIENT || state->status == NEED_DATA) {
            char recipient[100];

            if (check_regex("RCPT TO:<([a-zA-Z0-9\.@\-_]+)>", input, recipient) == 0) {

                if (state->msg->rec_cnt < 10) {
                    set_recipient(state->msg, recipient);
                    smtp_response(250, output);
                    state->status = NEED_DATA;
                }
                else {
                    smtp_response(455, output);
                }
            }
            else {
                smtp_response(501, output);
            }
        }
        else {
            smtp_response(503, output);
        }
    }
    else if (check_command("DATA", input) == 0) {
        if (state->status == NEED_DATA) {
            state->status = GET_DATA;
            smtp_response(354, output);
        }
        else {
            smtp_response(503, output);
        }
    }
    else if (check_command("VRFY", input) == 0) {
        if (state->status == READY || state->status == NEED_SENDER) {
            char user_info[255];
            if (check_regex("VRFY ([a-zA-Z0-9\.@\-_]+)", input, user_info) == 0) {
                char full_info[1024];
                if (check_user(user_info, full_info) == 0) {
                    strcpy(output, "250 ");
                    strncat(output, full_info, 1020);
                }
                else {
                    smtp_response(550, output);
                }
            }
            else {
                smtp_response(501, output);
            }
        }
        else {
            smtp_response(503, output);
        }
    }
    else {
        smtp_response(500, output);
    }
    return 0;
}

//-------------------------------------------------------------------------------------------------

SmtpMessage *make_message() {
    SmtpMessage *msg = (SmtpMessage *) malloc(sizeof(SmtpMessage));
    if (msg == NULL) {
        return NULL;
    }

    msg->domain = NULL;
    msg->sender = NULL;
    msg->recipients = (char **) malloc(sizeof(char *) * 2);
    if (msg->recipients == NULL) {
        free(msg);
        return NULL;
    }
    msg->rec_size = 2;
    msg->rec_cnt = 0;

    msg->message = (char *) malloc(sizeof(char) * 100);
    if (msg->message == NULL) {
        free(msg->recipients);
        free(msg);
        return NULL;
    }

    msg->msg_size = 100;
    return msg;
}

void destroy_message(SmtpMessage *msg) {
    if (msg != NULL) {
        if (msg->domain != NULL) {
            free(msg->domain);
        }
        if (msg->sender != NULL) {
            free(msg->sender);
        }
        if (msg->message != NULL) {
            free(msg->message);
        }
        if (msg->recipients != NULL) {
            for (int I = 0; I < msg->rec_cnt; I++) {
                free(msg->recipients[I]);
            }
            free(msg->recipients);
        }
        free(msg);
    }
}

char* set_domain(SmtpMessage *msg, char *domain) {
    if (msg->domain != NULL) {
        free(msg->domain);
    }
    msg->domain = (char *) malloc(sizeof(char) * (strlen(domain) + 2));
    if (msg->domain == NULL) {
        return NULL;
    }
    strcpy(msg->domain, domain);
    return msg->domain;
}

char* set_sender(SmtpMessage *msg, char *sender) {
    if (msg->sender != NULL) {
        free(msg->sender);
    }
    msg->sender = (char *) malloc(sizeof(char) * (strlen(sender) + 2));
    if (msg->sender == NULL) {
        return NULL;
    }
    strcpy(msg->sender, sender);
    return msg->sender;
}

char* set_recipient(SmtpMessage *msg, char *recipient) {
    if (msg->rec_cnt == msg->rec_size) {
        msg->rec_size *= 2;
        msg->recipients = (char **) realloc(msg->recipients, sizeof(char *) * msg->rec_size);

        if (msg->recipients == NULL) {
            write_log("realloc() failed in set_recipient");
            printf("realloc() failed in set_recipient\n");
            return NULL;
        }
    }

    msg->recipients[msg->rec_cnt] = (char *) malloc(sizeof(char) * (strlen(recipient) + 2));
    if (msg->recipients[msg->rec_cnt] == NULL) {
        return NULL;
    }

    strcpy(msg->recipients[msg->rec_cnt], recipient);
    msg->rec_cnt += 1;
    return msg->recipients[msg->rec_cnt];
}

char* append_message(SmtpMessage *msg, char *msgChunk) {
    if (strlen(msg->message) + strlen(msgChunk) > msg->msg_size) {
        msg->msg_size *= 2;
        msg->message = (char *) realloc(msg->message, sizeof(char) * msg->msg_size);

        if (msg->message == NULL) {
            write_log("realloc() failed in append_message");
            printf("realloc() failed in append_message\n");
            return NULL;
        }
    }
    strcat(msg->message, msgChunk);
    return msg->message;
}

int check_user(char *user_info, char *full_info) {
    const char *info_file_name;
    config_lookup_string(&cfg, "smtp.userinfo", &info_file_name);

    FILE *info_file = fopen(info_file_name, "r");
    if (info_file == NULL) {
        write_log("Can not open userinfo file");
        printf("Can not open userinfo file!\n");
        return -1;
    }
    char info[1024];

    while (1) {
        int eof = fgets(info, 1024, info_file);
        if (eof != 0) {
            char *found = strstr(info, user_info);
            if (found != NULL) {
                if (full_info != NULL) {
                    strncpy(full_info, info, 1024);
                }
                fclose(info_file);
                return 0;
            }
        }
        else {
            break;
        }
    }
    fclose(info_file);
    return -1;
}

//-------------------------------------------------------------------------------------------------

int save_maildir(SmtpMessage *msg) {
    int res_code = 0;
    for (int I = 0; I < msg->rec_cnt; I++) {
        res_code = save_maildir_for(msg, I);
        if (res_code != 0) {
            write_log("Message not saved on disk\n");
            printf("Message not saved on disk\n");
        }
        write_log("Message saved in maildir\n");
    }
    return 0;
}

int prepare_maildir(char *user_email, char *workdir) {
    char username[100];
    strncchr(user_email, username, '@');
    append_path(workdir, username, 1024);

    int res_code;
    res_code = mkdir_ifno(workdir);
    if (res_code != 0) {
        printf("Can not mkdir for %s\n", workdir);
        return -1;
    }
    append_path(workdir, "Maildir", 1024);
    res_code = mkdir_ifno(workdir);
    if (res_code != 0) {
        printf("Can not mkdir for %s\n", workdir);
        return -1;
    }
    append_path(workdir, "new", 1024);
    res_code = mkdir_ifno(workdir);
    if (res_code != 0) {
        printf("Can not mkdir for %s\n", workdir);
        return -1;
    }

    workdir[strlen(workdir) - 4] = '\0';
    append_path(workdir, "tmp", 1024);
    res_code = mkdir_ifno(workdir);
    return res_code;
}

int save_maildir_for(SmtpMessage *msg, int index) {
    char unique_id[255];
    create_unique_id(unique_id);

    char *maildir;
    config_lookup_string(&cfg, "smtp.maildir", &maildir);

    char workdir_buf[1024];
    strcpy(workdir_buf, maildir);
    int res_code = prepare_maildir(msg->recipients[index], workdir_buf);
    if (res_code != 0) {
        return res_code;
    }
    append_path(workdir_buf, unique_id, 1024);

    FILE *mail_file = fopen(workdir_buf, "w");
    if (mail_file == NULL) {
        write_log("Can not open maildir file\n");
        printf("Can not open '%s'\n", workdir_buf);
        return -1;
    }
    fprintf(mail_file, "\n");

    char now[40];
    formatted_now(now, 40);

    char *app_name;
    config_lookup_string(&cfg, "smtp.name", &app_name);

    fprintf(mail_file, "Received: by %s with SMTP; %s\n", app_name, now);
    fprintf(mail_file, "Message-Id: <%s>\n", unique_id);
    fprintf(mail_file, "From: <%s>\n", msg->sender);
    fprintf(mail_file, "To: <%s>\n", msg->recipients[index]);
    fprintf(mail_file, "Date: %s\n", now);

    if (msg->rec_cnt > 1) {
        fprintf(mail_file, "Cc: ");
        for (int I = 0; I < msg->rec_cnt; I++) {
            if (I != index) {
                fprintf(mail_file, "<%s>", msg->recipients[I]);
                if (I + 1 < msg->rec_cnt) {
                    fprintf(mail_file, ",");
                }
            }
        }
        fprintf(mail_file, "\n");
    }

    char subject[200];
    strncchr(msg->message, subject, '\n');
    fprintf(mail_file, "Subject: %s\n", subject);

    fprintf(mail_file, "Content-Type: text/plain; charset=koi8-r\n");
    fprintf(mail_file, "Content-Transfer-Encoding: 8bit\n");
    fprintf(mail_file, "\n\n");
    fprintf(mail_file, "%s\n", msg->message);

    fclose(mail_file);

    char new_name_buf[1024];
    strncpy(new_name_buf, workdir_buf, 1024);
    replace_path(new_name_buf, -2, "new");

    res_code = rename(workdir_buf, new_name_buf);

    write_log("Written maildir file\n");
    return res_code;
}
