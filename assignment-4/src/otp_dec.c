/*** includes ***/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>


/*** definitions ***/
#define BUFF_MAX 100


/*** prototypes ***/
int check_and_read_file(char*, char***, int*, int**);
int send_message(int, char*, int);
int receive_message(int, char*, int);
int cleanup(char***, int*, int**);
int main(int, char**);
int _buff_len(char*);
void error(const char*);
void e_error(const char*, int);


/*** functions ***/
/* check_and_read_file - checks files have no bad chars and
 *                       break into BUFF_MAX sized chunks
 * pre-conditions      - file_name is initialized (null-terminated)
 *                       buffers is an un-malloc'd array of strings
 *                       num_buffs is a pointer to an initialized integer
 *                       buff_lens is an unitialized array of ints
 */
int check_and_read_file(char * file_name, char *** buffers,
                        int * num_buffs, int ** buff_lens)
{
    /* declarations */
    FILE * in_file;
    int size_buffs;
    char c;
    int i;

    /* open file */
    in_file = fopen(file_name, "r");
    if (in_file == NULL) { return 1; }

    /* initial malloc's and values */
    (*buffers) = malloc(1 * sizeof(char *));
    (*buffers)[0] = malloc(BUFF_MAX * sizeof(char));
    (*buff_lens) = malloc(1 * sizeof(int));
    (*num_buffs) = 1;
    size_buffs = 1;
    (*buff_lens)[0] = 0;

    /* read the file char by char */
    while( (c = getc(in_file)) != EOF && c != '\n' )
    {
        // if invalid character has been given
        if (c != ' ' && (c < 'A' || c > 'Z'))
        {
            fprintf(stderr, "invalid char = \'%c\'\n", c);
            cleanup(buffers, num_buffs, buff_lens);
            fclose(in_file);
            return 2;
        }

        // add character to end of current buffer
        (*buffers)[(*num_buffs) - 1][(*buff_lens)[(*num_buffs) - 1]] = c;
        (*buff_lens)[(*num_buffs) - 1]++;

        // if buffer is at capacity
        if ((*buff_lens)[(*num_buffs) - 1] == BUFF_MAX - 1)
        {
            // add null-termination
            (*buffers)[(*num_buffs) - 1][(*buff_lens)[(*num_buffs) - 1]] = '\0';
            (*buff_lens)[(*num_buffs) - 1]++;

            // if buffers need to grow in size
            if ((*num_buffs) == size_buffs)
            {
                char ** buffers_backup = malloc(2 * size_buffs * sizeof(char *));
                int * buff_lens_backup = malloc(2 * size_buffs * sizeof(int));
                for (i = 0; i < size_buffs; i++)
                {
                    buffers_backup[i] = (*buffers)[i];
                    buff_lens_backup[i] = (*buff_lens)[i];
                }
                size_buffs *= 2;
                free((*buffers));
                free((*buff_lens));
                (*buffers) = buffers_backup;
                (*buff_lens) = buff_lens_backup;
            }

            // malloc new buffer
            (*buffers)[(*num_buffs)] = malloc(BUFF_MAX * sizeof(char));

            // setup next buffer
            (*buff_lens)[(*num_buffs)] = 0;
            (*num_buffs)++;
        }
    }

    // add null-termination
    (*buffers)[(*num_buffs) - 1][(*buff_lens)[(*num_buffs) - 1]] = '\0';
    (*buff_lens)[(*num_buffs) - 1]++;

    // close file
    fclose(in_file);

    // return
    return 0;
}

/* send_message - sends a message on an open socket */
int send_message(int socketFD, char * buffer, int length)
{
    // printf("CLIENT: Sending \"%s\" to server\n", buffer);
    int charsWritten = send(socketFD, buffer, length, 0);
    if (charsWritten < 0) error("CLIENT: ERROR writing to socket");
    if (charsWritten < length) error("CLIENT: WARNING: Not all data written to socket!");
    return 0;
}

/* receive_message - receive a message on a socket into a buffer */
int receive_message(int establishedConnectionFD, char * buffer, int length)
{
    memset(buffer, '\0', BUFF_MAX);
    int charsRead = recv(establishedConnectionFD, buffer, length, 0);
    // printf("CLIENT: Received \"%s\" from client\n", buffer);
    if (charsRead < 0) { error("CLIENT: ERROR reading from socket"); return 1; }
    return 0;
}

/* cleanup - free memory used in file opening */
int cleanup(char *** buffers, int * num_buffs, int ** buff_lens)
{
    int i;

    for(i = 0; i < (*num_buffs); i++)
    {
        free((*buffers)[i]);
    }

    free((*buffers));
    free((*buff_lens));
}

/* e_error - function used for reporting startup issues */
void e_error(const char * msg, int status)
{
    // perror(msg);
    fprintf(stderr, "%s\n", msg);
    exit(status);
}

/* error - function used for reporting runtime issues */
void error(const char * msg)
{
    // perror(msg);
    fprintf(stderr, "%s\n", msg);
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
    // Declarations
    int socketFD, port, charsWritten, charsRead, i;
    struct sockaddr_in serverAddress;
    struct hostent* serverHostInfo;
    char * host = "localhost";

    // Check usage
    if (argc != 4)
    {
        fprintf(stderr,"USAGE: %s ciphertext key port\n", argv[0]);
        exit(0);
    }

    // Set up the server address struct
    memset((char *) &serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
    port = atoi(argv[3]); // Get the port number, convert to an from a string
    serverAddress.sin_family = AF_INET; // Create a network-capable socket
    serverAddress.sin_port = htons(port); // Store the port number
    serverHostInfo = gethostbyname(host); // Convert the machine name into a form of address

    if (serverHostInfo == NULL) { e_error("Error! No such host!", 2); }

    memcpy( (char *) &serverAddress.sin_addr.s_addr,
            (char *) serverHostInfo->h_addr,
            serverHostInfo->h_length); // Copy in the address

    // Set up the socket
    socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
    if (socketFD < 0) error("Error opening socket!");


    /* Open files and get messages */
    char ** enc_buffers;
    char ** key_buffers;
    int * enc_buff_lens;
    int * key_buff_lens;
    int num_enc_buffs = 0;
    int num_key_buffs = 0;
    int result;

    // Get plaintext from files
    result = check_and_read_file(argv[1], &enc_buffers,
                                 &num_enc_buffs, &enc_buff_lens);

    // Check plaintext file errors
    if (result == 1)      { e_error("Unable to open ciphertext file!", 1); }
    else if (result == 2) { e_error("Invalid character in ciphertext!", 1); }

    // Get key from files
    result = check_and_read_file(argv[2], &key_buffers,
                                 &num_key_buffs, &key_buff_lens);

    // Check key file errors
    if (result != 0)
    {
        cleanup(&enc_buffers, &num_enc_buffs, &enc_buff_lens);
        if (result == 1)      { e_error("Unable to open key file!", 1); }
        else if (result == 2) { e_error("Invalid character in key!", 1); }
    }

    // Check that sizes of messages are the same
    if (num_enc_buffs > num_key_buffs
        || ((num_enc_buffs == num_key_buffs)
            && (enc_buff_lens[num_enc_buffs - 1] > key_buff_lens[num_key_buffs - 1])))
    {
        cleanup(&enc_buffers, &num_enc_buffs, &enc_buff_lens);
        cleanup(&key_buffers, &num_key_buffs, &key_buff_lens);
        e_error("Key file is shorter than ciphertext!", 1);
    }

    // Connect to server
    if (connect(socketFD,
                (struct sockaddr *) &serverAddress,
                sizeof(serverAddress)) < 0) // Connect socket to address
    {
        cleanup(&enc_buffers, &num_enc_buffs, &enc_buff_lens);
        cleanup(&key_buffers, &num_key_buffs, &key_buff_lens);
        e_error("Error connecting!", 2);
    }


    /* Send messages */
    char plain_buff[BUFF_MAX];

    // send identity of program
    send_message(socketFD, "otp_dec", strlen("otp_dec"));

    // wait for confirmation of receiving identity
    receive_message(socketFD, plain_buff, 10);
    int good_id = (strstr(plain_buff, "got id") != NULL);

    if (good_id)
    {
        // send each buffer of plaintext and key (always more or equal buffs of key)
        for (i = 0; i < num_enc_buffs; i++)
        {
            // send plaintext buffer
            send_message(socketFD, enc_buffers[i], enc_buff_lens[i]);

            // wait for confirmation of receiving plaintext
            receive_message(socketFD, plain_buff, 10);
            if (strstr(plain_buff, "got ct") == NULL) { error("error!"); break; }

            // send key buffer
            send_message(socketFD, key_buffers[i], key_buff_lens[i]);

            // receive the encoded message (also confirmation for key)
            receive_message(socketFD, plain_buff, BUFF_MAX);

            // output (part of) encoded message
            printf(plain_buff);
        }

        // Send confirmation of end of transmition
        send_message(socketFD, "# end of message", strlen("# end of message"));
    }
    else
    {
        close(socketFD);
        cleanup(&enc_buffers, &num_enc_buffs, &enc_buff_lens);
        cleanup(&key_buffers, &num_key_buffs, &key_buff_lens);
        e_error("Connection refused by server (bad id)!", 2);
    }

    // add newline to output
    printf("\n");

    // close the socket
    close(socketFD);

    // cleanup
    cleanup(&enc_buffers, &num_enc_buffs, &enc_buff_lens);
    cleanup(&key_buffers, &num_key_buffs, &key_buff_lens);

    // return
    return 0;
}
