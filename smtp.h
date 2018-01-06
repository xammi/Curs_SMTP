#ifndef SMTP_H
#define SMTP_H

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <regex.h>


enum SmtpStatus { NEW, READY, NEED_SENDER, NEED_RECEIVER, GET_DATA };

typedef struct {
    enum SmtpStatus status;
} SmtpState;


SmtpState* make_states(int amount);
void destroy_states(SmtpState *states);

void handle_welcome(SmtpState *state, char *output);
void handle_request(SmtpState *state, char *input, char *output);

int check_regex(char *regex, char *src, char *param);
int check_command(char *command, char *src);
char *smtp_response(int code);


#endif // SMTP_H
