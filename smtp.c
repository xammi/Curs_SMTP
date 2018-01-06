#include "smtp.h"


SmtpState* make_states(int amount) {
    SmtpState* states = (SmtpState*) malloc(sizeof(SmtpState) * amount);
    if (states == NULL) {
        return NULL;
    }
    for (int I = 0; I < amount; I++) {
        states[I].status = NEW;
    }
    return states;
}

void destroy_states(SmtpState* states) {
    if (states != NULL) {
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

char *smtp_response(int code) {
    if (code == 250) {
        return "250 OK\r\n";
    }
    else if (code == 500) {
        return "500 Invalid command\r\n";
    }
    else if (code == 501) {
        return "501 Invalid argument\r\n";
    }
    else if (code == 502) {
        return "502 Not implemented\r\n";
    }
    else if (code == 503) {
        return "503 Bad sequence of commands\r\n";
    }
    return "";
}

void handle_request(SmtpState *state, char *input, char *output) {
    if (state->status == READY) {
        char domain[100];

        if (check_command("HELO", input) == 0) {
            if (check_regex("HELO ([a-zA-Z0-9\.]+)", input, domain) == 0) {
                printf("Domain=%s\n", domain);
                strcpy(output, smtp_response(250));
                state->status = NEED_SENDER;
            }
            else {
                strcpy(output, smtp_response(501));
            }
        }
        else if (check_command("EHLO", input) == 0) {
            if (check_regex("EHLO ([a-zA-Z0-9\.]+)", input, domain) == 0) {
                printf("Domain=%s\n", domain);
                strcpy(output, "250-smtp.maxim.ru\r\n250-PIPELINING\r\n250-8BITMIME\r\n");
                state->status = NEED_SENDER;
            }
            else {
                strcpy(output, smtp_response(501));
            }
        }
        else {
            strcpy(output, smtp_response(500));
        }
    }
    else if (state->status == NEED_SENDER) {
        char sender[100];

        if (check_command("MAIL FROM", input) == 0) {
            if (check_regex("MAIL FROM:<([a-zA-Z0-9\.@\-_]+)>", input, sender) == 0) {
                printf("Sender=%s\n", sender);
                strcpy(output, smtp_response(250));
                state->status = NEED_RECEIVER;
            }
            else {
                strcpy(output, smtp_response(501));
            }
        }
        else {
            strcpy(output, smtp_response(500));
        }
    }
    else if (state->status == NEED_RECEIVER) {
        char receiver[100];

        if (check_command("RCPT TO", input) == 0) {
            if (check_regex("RCPT TO:<([a-zA-Z0-9\.@\-_]+)>", input, sender) == 0) {

                // check receiver's count

                printf("Receiver=%s\n", receiver);
                strcpy(output, smtp_response(250));
                state->status = GET_DATA;
            }
            else {
                strcpy(output, smtp_response(501));
            }
        }
        else {
            strcpy(output, smtp_response(500));
        }
    }
    else if (state->status == GET_DATA) {

    }
}
