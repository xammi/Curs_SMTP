#ifndef SMTP_H
#define SMTP_H

#include "inc/utils.h"


enum SmtpStatus {
    NEW,
    READY,
    NEED_SENDER,
    NEED_RECIPIENT,
    NEED_DATA,
    GET_DATA
};

typedef struct {
    char *domain;
    char *sender;

    char **recipients;
    int rec_cnt;
    int rec_size;

    char *message;
    int msg_size;
} SmtpMessage;

SmtpMessage* make_message();
void destroy_message(SmtpMessage *msg);

char* set_domain(SmtpMessage *msg, char *domain);
char* set_sender(SmtpMessage *msg, char *sender);
char* set_recipient(SmtpMessage *msg, char *recipient);
char* append_message(SmtpMessage *msg, char *msgChunk);

int prepare_maildir(char *user_email, char *workdir);
int save_maildir(SmtpMessage *msg);
int save_maildir_for(SmtpMessage *msg, int index);


typedef struct {
    enum SmtpStatus status;
    SmtpMessage *msg;
} SmtpState;


SmtpState* make_states(int amount);
void destroy_states(SmtpState *states);

void handle_welcome(SmtpState *state, char *output);
int handle_request(SmtpState *state, char *input, char *output);

int check_regex(char *regex, char *src, char *param);
int check_command(char *command, char *src);
void smtp_response(int code, char *output);
int check_user(char *user_info, char *full_info);


#endif // SMTP_H
