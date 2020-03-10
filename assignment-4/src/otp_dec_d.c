/*** includes ***/
#include <stdio.h>
#include <stdlib.h>


/*** prototypes ***/
int dec(char*, char*, char*);
int main(int, char**);
int _buff_len(char*);


/*** functions ***/
/* dec              decodes cypher text
 * pre-condition:   plain_buff is not malloc'd */
int dec(char * plain_buff, char * enc_buff, char * key_buff)
{
    /* declarations */
    int enc_val;
    int key_idx;
    int key_len;
    int key_val;
    int res;
    int i;

    /* malloc enc_buff */
    plain_buff = malloc((_buff_len(plain_buff) + 2) * sizeof(char));

    /* get key_len */
    key_len = _buff_len(key_buff);

    /* convert plain_buff */
    for (i = 0; i < _buff_len(enc_buff); i++)
    {
        // get key index
        key_idx = i % key_len;

        // get numerical value of encoded char
        enc_val = enc_buff[i] - 'A';

        // get numerical value of key char
        key_val = key_buff[key_idx] - 'A';

        // check for spaces
        if (enc_val == ' ' - 'A') { enc_val = 26; }
        if (key_val == ' ' - 'A') { key_val = 26; }

        // add values
        res = (enc_val + key_val) % 27;

        // put result in enc_buff
        plain_buff[i] = (res == 26) ? (' ') : ((char) (res + 'A'));
    }

    /* return */
    return 0;
}

/* _buff_len
 * pre-condition:   buff is not NULL */
int _buff_len(char* buff)
{
    /* declarations */
    int len = 0;

    /* check NULL */
    if (buff == NULL) { return -1; }

    /* for each char in buff */
    while (buff[len] != '\0' && buff[len] != '\n') { len++; }

    /* return result */
    return len;
}

/*** main ***/
int main(int argc, char ** argv)
{
    /* declarations */
    /* check num args */
    /* convert arg to port */
    /**/
    /**/
}

