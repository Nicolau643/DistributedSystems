#ifndef _MESSAGE_H
#define _MESSAGE_H

#define OP_BAD      0
#define OP_SIZE		10
#define OP_DEL	   	20
#define OP_GET		30
#define OP_PUT		40
#define OP_GETKEYS	50
#define OP_VERIFY   60
#define OP_ERROR    99

#define CT_BAD    0
#define CT_KEY    10
#define CT_VALUE  20
#define CT_ENTRY  30
#define CT_KEYS   40
#define CT_RESULT 50
#define CT_NONE   60


struct message_t
{
    int opcode;
    int c_type;
    int client_server;
    union content
    {
        struct data_t *data;
        struct entry_t *entry;
        char *key;
        char **keys;
        int result;
        int op_n;
    }content;
    
    
    
};

void message_destroy(struct message_t *m);


#endif