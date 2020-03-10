/*** includes ***/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>


/*** definitions ***/
#define BUFF_MAX 1000
#define MAX_CHILDREN 5


/*** prototypes ***/
int enc(char**, char*, char*);
int start_listening(int);
int spawn_child();
int send_message(int, char*, int);
int receive_message(int, char*, int);
int main(int, char**);
int _buff_len(char*);
void e_error(const char*);
void error(const char*);


/*** functions ***/
/* e_error - function used for reporting startup issues */
void e_error(const char * msg)
{
    perror(msg);
    exit(1);
}

/* error - function used for reporting runtime issues */
void error(const char * msg)
{
    perror(msg);
}

/* enc              encodes plain-text into cypher text
 * pre-condition:   enc_buff is malloc'd */
int enc(char ** enc_buff, char * plain_buff, char * key_buff)
{
    /* declarations */
    int plain_val;
    int key_idx;
    int key_len;
    int key_val;
    int res;
    int i;

    /* malloc enc_buff */
    // (*enc_buff) = malloc((_buff_len(plain_buff) + 2) * sizeof(char));
    memset((*enc_buff), '\0', BUFF_MAX);//_buff_len(plain_buff) + 2);

    /* get key_len */
    key_len = _buff_len(key_buff);

    /* convert plain_buff */
    for (i = 0; i < _buff_len(plain_buff); i++)
    {
        // get key index
        // key_idx = i % key_len;
        key_idx = i;

        // get numerical value of plain text char
        plain_val = plain_buff[i] - 'A';

        // get numerical value of key char
        key_val = key_buff[key_idx] - 'A';

        // check for spaces
        if (plain_val == ' ' - 'A') { plain_val = 26; }
        if (key_val   == ' ' - 'A') { key_val   = 26; }

        // add values
        res = (plain_val + key_val) % 27;

        // put result in enc_buff
        (*enc_buff)[i] = (res == 26) ? (' ') : ((char) (res + 'A'));
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

/* send_message - sends a message on an open socket */
int send_message(int socketFD, char * buffer, int length)
{
    // printf("CLIENT: Sending \"%s\" to server\n", buffer);
    int charsWritten = send(socketFD, buffer, length, 0);
    if (charsWritten < 0) error("SERVER: ERROR writing to socket");
    if (charsWritten < length) printf("SERVER: WARNING: Not all data written to socket!\n");
    return 0;
}

/* receive_message - receive a message on a socket into a buffer */
int receive_message(int establishedConnectionFD, char * buffer, int length)
{
    memset(buffer, '\0', BUFF_MAX);
    int charsRead = recv(establishedConnectionFD, buffer, length, 0);
    // printf("SERVER: Received \"%s\" from client\n", buffer);
    if (charsRead < 0) error("SERVER: ERROR reading from socket"); return 1;
    return 0;
}


/*** main ***/
int main(int argc, char ** argv)
{
    /* declarations */
    int listenSocketFD, establishedConnectionFD, charsRead;
    struct sockaddr_in serverAddress, clientAddress;
    socklen_t sizeOfClientInfo;
    int port;

    /* check num args */
    if (argc <= 1)
        { e_error("Please provide desired port...\n"); }

    /* convert arg to port */
    if ((port = atoi(argv[1])) == 0)
        { error("Invalid port or intentional port 0...\n"); }

    // set up the address struct for this process (the server)
    memset((char *) &serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
    serverAddress.sin_family = AF_INET; // Create a network-capable socket
    serverAddress.sin_port = htons(port); // Store the port number
    serverAddress.sin_addr.s_addr = INADDR_ANY; // Any address is allowed for connection to this process

    // Set up the socket
    listenSocketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
    if (listenSocketFD < 0) e_error("ERROR opening socket");

    // allow reuse
    int optval = 1;
    if (setsockopt(listenSocketFD, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval) < 0)
      { e_error("ERROR on setting socket options"); }

    // enable the socket to begin listening (bind)
    if (bind(listenSocketFD, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0) // Connect socket to port
        { e_error("ERROR on binding"); }

    // listen on socket
    listen(listenSocketFD, 5);

    /* DAEMON LOOP SETUP */
    int i = 0;
    int j = 0;
    int pid = -1;
    int keep_going = 1;
    int num_children = 0;
    int child_pids[MAX_CHILDREN];
    memset(child_pids, -1, MAX_CHILDREN);

    /* DAEMON LOOP */
    while (keep_going)
    {
        /* check if any children have finished */
        // for each child PID
        for(i = 0; i < num_children; i++)
        {
            // get status of children
            pid_t cpid = 0;
            cpid = waitpid(child_pids[i], NULL, WNOHANG);

            // if child has exited
            if (cpid != 0)
            {
                // shift everything after back to overwrite
                for(j = i; j < num_children - 1; j++)
                {
                    child_pids[j] = child_pids[j + 1];
                }

                // decrement
                num_children--;
                i--;
            }
        }

        /* only accept connections if an open spot is available */
        if (num_children >= 5)
        {
            continue;
        }

        /* wait for and accept a connection */
        // get the size of the address for the client that will connect
        sizeOfClientInfo = sizeof(clientAddress);

        // accept a connection, blocking if one is not available until one is
        establishedConnectionFD = accept(listenSocketFD,
                                         (struct sockaddr *) &clientAddress,
                                         &sizeOfClientInfo);

        // check if there was an error on accepting
        if (establishedConnectionFD < 0)
        {
            error("ERROR on accept");
            continue;
        }

        // fork child
        pid = fork();

        // if parent
        if (pid > 0)
        {
            // add child pid to array of children pids
            child_pids[num_children] = pid;

            // increment number of children
            num_children++;

            continue;
        }
        // else if child
        else if (pid == 0)
        {
            // declare buffers
            char identity_buff[BUFF_MAX];
            char plain_buff[BUFF_MAX];
            char key_buff[BUFF_MAX];
            char * enc_buff = malloc(BUFF_MAX * sizeof(char));

            // check if from correct program
            int is_correct_program = 0;
            receive_message(establishedConnectionFD, identity_buff, BUFF_MAX);
            is_correct_program = (strstr(identity_buff, "otp_enc") != NULL);

            // received identity, send confirmation
            if (is_correct_program) { send_message(establishedConnectionFD, "got id", strlen("got id")); }
            else { send_message(establishedConnectionFD, "bad id", strlen("bad id")); }

            // if correct
            while (is_correct_program)
            {
                // get section of plaintext (or end of message) from client
                receive_message(establishedConnectionFD, plain_buff, BUFF_MAX);
                // if (wow == 1) { printf("\nERROR\n"); printf(plain_buff); printf("\n\n"); }

                // check for end of message communication
                if (strstr(plain_buff, "# end of message") != NULL
                    || strcmp(plain_buff, "") == 0)
                {
                    // communication is done
                    // printf("end received in message \"%s\"\n", plain_buff);
                    is_correct_program = 0;
                    break;
                }
                else
                {
                    // received plaintext, send confirmation
                    char conf[] = "got pt";
                    send_message(establishedConnectionFD, conf, strlen(conf));
                }

                // get section of key from client
                receive_message(establishedConnectionFD, key_buff, BUFF_MAX);

                // encode plaintext
                enc(&enc_buff, plain_buff, key_buff);

                // send encoded message back to client
                send_message(establishedConnectionFD, enc_buff, BUFF_MAX);

                // cleanup child memory
                // free(plain_buff);
                // free(key_buff);
                // free(enc_buff
            }

            // stop child from looping
            keep_going = 0;

            // free enc_buff
            free(enc_buff);

            // close the existing socket which is connected to the client
            close(establishedConnectionFD);
        }
    }

    // Close the listening socket
    close(listenSocketFD);

    // Return
    return 0;
}
