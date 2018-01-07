#include "smtp.h"
#include <time.h>


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
    strcpy(output, "220 smtp.maxim.ru ready\r\n");
    state->status = READY;
}

int check_regex(char *regex, char *src, char *param) {
    regex_t regexObj;
    int res_code = regcomp(&regexObj, regex, REG_EXTENDED);
    if (res_code != 0) {
        printf("Could not compile regex %s\n", regex);
        return 1;
    }

    regmatch_t matchedGroups[2];
    char *cursor = src;
    res_code = regexec(&regexObj, cursor, 2, matchedGroups, 0);
    if (res_code != 0) {
        printf("No match with %s\n", regex);
        return 1;
    }
    if (matchedGroups[1].rm_so >= 0) {
        int len = matchedGroups[1].rm_eo - matchedGroups[1].rm_so;
        strncpy(param, cursor + matchedGroups[1].rm_so, len);
        param[len] = '\0';
    }
    else {
        param[0] = '\0';
    }

    regfree(&regexObj);
    return 0;
}

int check_command(char *command, char *src) {
    return strncmp(src, command, strlen(command));
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
        }
        state->status = READY;
        smtp_response(250, output);
    }
    else if (check_command("QUIT", input) == 0) {
        if (state->msg != NULL) {
            destroy_message(state->msg);
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
                strncpy(output, "250-smtp.maxim.ru\r\n250-PIPELINING\r\n250-8BITMIME\r\n250-VRFY\r\n", 255);
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
    msg->domain = (char *) malloc(sizeof(char) * strlen(domain));
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
    msg->sender = (char *) malloc(sizeof(char) * strlen(sender));
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
            printf("realloc() failed\n");
            return NULL;
        }
    }

    msg->recipients[msg->rec_cnt] = (char *) malloc(sizeof(char) * strlen(recipient));
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
            printf("realloc() failed\n");
            return NULL;
        }
    }
    strcat(msg->message, msgChunk);
    return msg->message;
}

int check_user(char *user_info, char *full_info) {
    const char *info_file_name = "/Users/maksimkislenko/smtp_env/smtp_server/userinfo";
    FILE *info_file = fopen(info_file_name, "r");
    if (info_file == NULL) {
        printf("Can not open userinfo file!\n");
        return -1;
    }
    char info[1024];

    while (1) {
        int eof = fgets(info, 1024, info_file);
        if (eof != NULL) {
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

int prepare_maildir(char *user_email, char *workdir) {
    char username[100];
    int N = (int)(strchr(user_email, '@') - user_email);
    strncpy(username, user_email, N);
    username[N] = '\0';

    int len = strlen(workdir);
    if (workdir[len - 1] != '/') {
        workdir[len] = '/';
        workdir[len + 1] = '\0';
    }
    strncat(workdir, username, 255);

    struct stat sb;
    int res_code;

    if (stat(workdir, &sb) == 0 && ! S_ISDIR(sb.st_mode)) {
        res_code = mkdir(workdir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        if (res_code != 0) {
            return -1;
        }
    }
    strcat(workdir, "/Maildir");
    if (stat(workdir, &sb) == 0 && ! S_ISDIR(sb.st_mode)) {
        res_code = mkdir(workdir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        if (res_code != 0) {
            return -1;
        }
    }
    strcat(workdir, "/new");
    if (stat(workdir, &sb) == 0 && ! S_ISDIR(sb.st_mode)) {
        res_code = mkdir(workdir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        if (res_code != 0) {
            return -1;
        }
    }
    workdir[strlen(workdir) - 4] = '\0';
    strcat(workdir, "/tmp");
    if (stat(workdir, &sb) == 0 && ! S_ISDIR(sb.st_mode)) {
        res_code = mkdir(workdir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    }
    return res_code;
}

void create_unique_id(char *unique_id) {
    uuid_t uuid;
    uuid_generate_time(uuid);

    char uuid_str[37];
    uuid_unparse_lower(uuid, uuid_str);
    strncpy(unique_id, uuid_str, 37);
}

int save_maildir(SmtpMessage *msg) {
    char unique_id[255];
    create_unique_id(unique_id);

    char workdir_buf[1024];
    strcpy(workdir_buf, "/Users/maksimkislenko/Desktop/mail");
    prepare_maildir(msg->recipients[0], workdir_buf);
    strcat(workdir_buf, unique_id);

    FILE *mail_file = fopen(workdir_buf, "w");
    if (mail_file == NULL) {
        printf("Can not open '%s'\n", workdir_buf);
        return -1;
    }
    fprintf(mail_file, "\n");

    time_t now;
    time(&now);

    fprintf(mail_file, "Received: by %s with SMTP; %s\n", "smtp.max.ru", asctime(localtime(&now)));
    fprintf(mail_file, "Message-Id: <%s>\n", unique_id);
    fprintf(mail_file, "From: <%s>\n", msg->sender);
    fprintf(mail_file, "To: <%s>\n", msg->recipients[0]);
    fprintf(mail_file, "Date: %s\n", asctime(localtime(&now)));

    if (msg->rec_cnt > 1) {
        fprintf(mail_file, "Cc: ");
        for (int I = 1; I < msg->rec_cnt; I++) {
            fprintf(mail_file, "<%s>", msg->recipients[I]);
            if (I + 1 < msg->rec_cnt) {
                fprintf(mail_file, ",");
            }
        }
        fprintf(mail_file, "\n");
    }

    char subject[200];
    strncpy(subject, msg->message, (int)(strchr(msg->message, '\n') - msg->message));
    fprintf(mail_file, "Subject: %s\n", subject);

    fprintf(mail_file, "Content-Type: text/plain; charset=koi8-r\n");
    fprintf(mail_file, "Content-Transfer-Encoding: 8bit\n");
    fprintf(mail_file, "\n\n");
    fprintf(mail_file, "%s\n\n", msg->message);

    fclose(mail_file);
    return 0;
}
