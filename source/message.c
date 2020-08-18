//Grupo 17
//André Nicolau 47880
//Alexandre Morgado 49015
//Jorge André 49496

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "message-private.h"
#include "data.h"
#include "entry.h"

void message_destroy(struct message_t *m){
    
    if (m == NULL)
    {
        return;
    }

    if (m->c_type == CT_VALUE)
    {
       data_destroy(m->content.data);

    }else if (m->c_type == CT_ENTRY)
    {
       entry_destroy(m->content.entry);
       
    }else if (m->c_type == CT_KEY)
    {
        free(m->content.key);

    }else if (m->c_type == CT_KEYS)
    {   
        int i = 0;

        while (m->content.keys[i] != NULL)
        {
            free(m->content.keys[i]);
            i++;
        }
        free(m->content.keys);

    }
    
    free(m);
    
}