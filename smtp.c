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

    regmatch_t matchedGroups[1];
    char *cursor = src;
    res_code = regexec(&regexObj, cursor, 1, matchedGroups, 0);
    if (res_code != 0) {
        printf("No match with %s\n", regex);
        return 1;
    }
    strncpy(param, cursor + matchedGroups[0].rm_so, 100);
    param[matchedGroups[0].rm_eo] = '\0';

    regfree(&regexObj);
    return 0;
}

int check_command(char *command, char *src) {
    return strncmp(src, command, strlen(command));
}

void smtp_response(int code, char *output) {
    char *response = "";
    if (code == 221) {
        response = "221 Bye\r\n";
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
                strncpy(output, "250-smtp.maxim.ru\r\n250-PIPELINING\r\n250-8BITMIME\r\n", 255);
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

                // check that sender is known

                set_sender(state->msg, sender);
                smtp_response(250, output);
                state->status = NEED_RECIPIENT;
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

int save_maildir(SmtpMessage *msg) {
    const char *mail_file_name = "/Users/maksimkislenko/Desktop/mail.txt";
    FILE *mail_file = fopen(mail_file_name, "w");
    if (mail_file == NULL) {
        printf("Can not open '%s'\n", mail_file_name);
        return -1;
    }
    fprintf(mail_file, "\n");

    time_t now;
    time(&now);
    fprintf(mail_file, "Received: by %s with SMTP; %Y-%m-%d %H:%M:%S\n", msg->domain, gmtime(now));
    fprintf(mail_file, "Message-Id: <%s>", "005f01c27055$b0be7c80$6df155c2@max.ru\n");
    fprintf(mail_file, "From: <%s>\n", msg->sender);
    fprintf(mail_file, "To: <%s>\n", msg->recipients[0]);

    fprintf(mail_file, "Cc: ");
    for (int I = 1; I < msg->rec_cnt; I++) {
        fprintf(mail_file, "<%s>", msg->recipients[I]);
        if (I + 1 < msg->rec_cnt) {
            fprintf(mail_file, ",");
        }
    }
    fprintf(mail_file, "\n");

    char subject[200];
    strncpy(subject, msg->message, strchr(msg->message, '\n'));
    fprintf(mail_file, "Subject: %s\n", subject);

    fprintf(mail_file, "Content-Type: text/plain; charset=koi8-r\n");
    fprintf(mail_file, "Content-Transfer-Encoding: 8bit\n");
    fprintf(mail_file, "\n\n");
    fprintf(mail_file, "%s\n\n", msg->message);

    fclose(mail_file);
    return 0;
}
