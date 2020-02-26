/*** includes ***/
#include <stdio.h>
#include <stdlib.h>


/*** prototypes ***/
int main(int, char**);


/*** main ***/
int main(int argc, char ** argv)
{
    /* declarations */
    char * key_buff;
    int len;
    int tmp;
    int i;

    /* check for arg */
    if (argc <= 1)
    {
        printf("Please include desired length of key...\n");
        return 1;
    }

    /* get length of key */
    if ((len = atoi(argv[1])) <= 0)
    {
        printf("Please include a positive integer...\n");
        return 2;
    }

    /* create array */
    key_buff = malloc((len + 2) * sizeof(char));

    /* seed rand */
    srand(time(0));

    /* fill with random chars */
    for (i = 0; i < len; i++)
    {
        tmp = rand() % 27;
        if (tmp == 26) { tmp = ' ' - 'A'; } // make char after Z into space
        key_buff[i] = (char) (tmp + 'A');
    }

    /* include newline and null */
    key_buff[len] = '\n';
    key_buff[len+1] = '\0';

    /* print result */
    printf(key_buff);

    /* free memory */
    free(key_buff);

    /* return */
    return 0;
}
