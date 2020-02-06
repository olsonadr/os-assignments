/*** includes ***/
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/*** structs ***/
struct game {
    char * rooms_path;

    char ** room_names;
    char ** room_types;
    char *** room_cons;

    char * input_buff;

    int max_name_len;

    int num_rooms;
    int * num_cons;

    int curr_room;
    int num_steps;
    
    char ** path;
    int path_len;
    int path_size;
    
    pthread_mutex_t lock;
};

/*** thread process prototype ***/
void* time_check(void*);

/*** interface prototypes ***/
int get_newest_rooms(char*, char*, char*);
int setup_game(struct game*, char*);
int free_game(struct game*);
int step_game(struct game*);
int print_stats(struct game*);
int main();

/*** hidden prototypes ***/
int _print_curr_room(struct game*);
int _find_parameters(char*, int*, int*, int*, int**);
int _find_num_rooms(char*, int*);
int _add_to_path(struct game*, char*);
int _grow_path(struct game*);
int _get_input();


/*** thread process ***/
void* time_check(void * arg)
{
    // mutex lock stuff
    pthread_mutex_t * lock = arg;
    pthread_mutex_lock(lock);

    // vars
    char time_buff[50];
    struct tm * local;
    FILE * fp;
    time_t t;

    // get local time
    time(&t);
    local = localtime(&t);

    strftime(time_buff, sizeof(time_buff), "%I:%M%P, %A, %B %d, %Y\n\0", local);

    // open file for writing
    fp = fopen("currentTime.txt", "w+");

    // write time to file
    fprintf(fp, "%s", time_buff);
    
    // move pointer to start of file
    rewind(fp);

    // print contents
    fgets(time_buff, sizeof(time_buff), fp);
    printf(" %s\n", time_buff);

    // close file
    fclose(fp);

    // unlock mutex
    pthread_mutex_unlock(lock);

    return NULL;
}

/*** interface helpers ***/
int get_newest_rooms(char * newest_dir,      /* where to put path to newest */
                     char * root_dir,        /* where dirs are held, no ending / */
                     char * regex_pattern)   /* regex pattern of dirs */
{
    // declarations
    DIR *d;
    struct dirent *dir;
    char curr_dir[40];
    regex_t regex_p;
    struct stat sb;
    struct timespec newest_time;
    int first = 1;

    // compile regex
    if (regcomp(&regex_p, regex_pattern, 0) != 0)
    {
        printf("Regex compilation error!");
        return 1;
    }

    // open directory where possible rooms dirs are held
    d = opendir(root_dir);

    // open directory to choose most recent rooms dir
    while (d && (dir = readdir(d)))
    {
        if (regexec(&regex_p, dir->d_name, 0, NULL, 0) == 0)
        {
            sprintf(curr_dir, "%s/%s", root_dir, dir->d_name);
            stat(curr_dir, &sb);
            
            if (first || sb.st_mtim.tv_sec > newest_time.tv_sec)
            {
                first = 0;
                newest_time = sb.st_mtim;
                sprintf(newest_dir, "%s", dir->d_name);
            }
        }
    }
    if (!d)
    {
        printf("Directory opening error!");
        return 2;
    }

    // cleanup
    closedir(d);
    regfree(&regex_p);

    // return
    return 0;
}


/* pre-conditions:  rooms_path is null-terminated and non-empty
 *                  g is not setup and has been allocated (not its members)
 */
int setup_game(struct game * g, char * rooms_path)
{
    // parameters
    int max_rooms_path_len = 80;
    int max_type_len = 0;
    g->max_name_len = 0;
    int i = 0;
    int j = 0;

    // init mutex lock
    if (pthread_mutex_init(&(g->lock), NULL) != 0) { return 1; printf("AAHAHHHHH\n"); }

    // determine num rooms
    _find_num_rooms(rooms_path, &(g->num_rooms));

    // assign misc
    g->num_steps = 0;
    g->path_len = 0;
    g->path_size = g->num_rooms;

    // allocate memory for num_cons
    g->num_cons = malloc(g->num_rooms * sizeof(int));

    // determine parameters
    _find_parameters(rooms_path, &(g->max_name_len), &max_type_len, &(g->num_rooms), &(g->num_cons));
    
    // assign rooms_path
    g->rooms_path = malloc(max_rooms_path_len * sizeof(char));
    sprintf(g->rooms_path, "%s", rooms_path);

    // allocate memory
    g->input_buff = malloc((g->max_name_len+1) * sizeof(char));
    g->path = malloc(g->path_size * sizeof(char*));
    g->room_names = malloc(g->num_rooms * sizeof(char*));
    g->room_types = malloc(g->num_rooms * sizeof(char*));
    g->room_cons = malloc(g->num_rooms * sizeof(char*));
    for (i = 0; i < g->num_rooms; i++)
    {
        g->path[i] = malloc((g->max_name_len+1) * sizeof(char));
        g->room_names[i] = malloc((g->max_name_len+1) * sizeof(char));
        g->room_types[i] = malloc((max_type_len+1) * sizeof(char));
        g->room_cons[i] = malloc((g->num_cons[i]) * sizeof(char*));
        for (j = 0; j < g->num_cons[i]; j++)
        {
            g->room_cons[i][j] = malloc((g->max_name_len+1) * sizeof(char));
        }
    }

    // constants
    char name_prefix[] = "ROOM NAME: ";
    char type_prefix[] = "ROOM TYPE: ";
    char start_type[] = "START_ROOM";
    char con_prefix[] = "CONNECTION ";

    // variables
    struct dirent *ep;
    int buff_max_len = 40;
    char * ind_room_path = malloc((buff_max_len + 1) * sizeof(char));
    char * buffer = malloc((buff_max_len + 1) * sizeof(char));
    FILE * room_file;
    DIR *dp;
    i = 0;
    j = 0;

    // open rooms root dir
    dp = opendir(rooms_path);

    // for each file in the rooms_path
    while (ep = readdir(dp))
    {
        // ignore . and ..
        if (strcmp(ep->d_name, ".") == 0) continue;
        else if (strcmp(ep->d_name, "..") == 0) continue;

        // open the file (for each line)
        sprintf(ind_room_path, "%s/%s", rooms_path, ep->d_name);
        if ((room_file = fopen(ind_room_path, "rw")) == NULL) return 1;
        j = 0;

        while (fgets(buffer, buff_max_len, room_file) != NULL)
        {
            // if line starts with "ROOM NAME: "
            if (strstr(buffer, name_prefix) != NULL)
            { 
                strcpy(g->room_names[i], (buffer + strlen(name_prefix)));
                g->room_names[i][strcspn(g->room_names[i], "\n")] = 0;
            }
            // else if line starts with "ROOM TYPE: "
            else if (strstr(buffer, type_prefix) != NULL)
            {
                strcpy(g->room_types[i], (buffer + strlen(type_prefix)));
                g->room_types[i][strcspn(g->room_types[i], "\n")] = 0;
                if (strstr(buffer, start_type) != NULL) { g->curr_room = i; }
            }
            // else if line starts with "CONNECTION "
            else if (strstr(buffer, con_prefix) != NULL)
            {
                strcpy(g->room_cons[i][j], (buffer + strlen(con_prefix) + 3));
                g->room_cons[i][j][strcspn(g->room_cons[i][j], "\n")] = 0;
                j++; // next connection if there are more for room
            }
        }
        
        // close file
        fclose(room_file);

        // increment room index
        i++;
    }

    // free stuff and stuff
    free(buffer);
    free(ind_room_path);

    // close dir
    closedir(dp);

    // return
    return 0;
}

int free_game(struct game * g)
{
    // variables
    int i;
    int j;

    // free arrays
    for (i = 0; i < g->num_rooms; i++)
    {
        for (j = 0; j < g->num_cons[i]; j++) { free(g->room_cons[i][j]); }
        free(g->room_names[i]);
        free(g->room_types[i]);
        free(g->room_cons[i]);
    }

    // free path
    for (i = 0; i < g->path_size; i++)
    {
        free(g->path[i]);
    }

    // free misc
    free(g->path);
    free(g->input_buff);
    free(g->rooms_path);
    free(g->num_cons);
    free(g->room_names);
    free(g->room_types);
    free(g->room_cons); 

    // destroy mutex lock
    pthread_mutex_destroy(&(g->lock));
}

/*
 * pre-condition:   game has been setup using setup_game()
 * post-condition:  a single game iteration has occurred, result returned
 * returns:         type of result from iteration
 *                      0 - continue, normal operation
 *                      1 - player won game, exit game loop
 *                      2 - player quit early, exit game loop
 */
int step_game(struct game * g)
{
    // declare local tmps
    pthread_t tid;
    int i;
    int j;
    
    // declare destination and initialize to -1
    int dest_idx = -1;

    // print current room attributes
    _print_curr_room(g);

    // get user input
    fgets(g->input_buff, g->max_name_len + 1, stdin);
    g->input_buff[strcspn(g->input_buff, "\n")] = 0;

    // move line down
    printf("\n");

    // print input
    //printf("INPUT WAS \"%s\"\n\n", g->input_buff);

    // if input is quit or exit
    if (strcmp(g->input_buff, "quit") == 0
            || strcmp(g->input_buff, "Quit") == 0
            || strcmp(g->input_buff, "exit") == 0
            || strcmp(g->input_buff, "Exit") == 0)
    {
        // return 2 
        return 2;
    }

    // if input was time
    if (strcmp(g->input_buff, "time") == 0)
    {
        if (pthread_create(&tid, NULL, &time_check, ((void*) &(g->lock))) != 0) { return 3; }
        pthread_join(tid, NULL);
        return 0;
    }

    // for each connection of the current room
    for (i = 0; i < g->num_cons[g->curr_room]; i++)
    {
        // if the connection matches the input
        if (strcmp(g->input_buff, g->room_cons[g->curr_room][i]) == 0)
        {
            // for each room (find which index this name refers to)
            for (j = 0; j < g->num_rooms; j++)
            {
                if (strcmp(g->room_names[j], g->room_cons[g->curr_room][i]) == 0)
                {
                    // set destination to this idx
                    dest_idx = j;
                    break;
                }
            }

            // break
            break;
        }
    }

    // if the destination is -1
    if (dest_idx == -1)
    {
        // destination not found, print off error
        printf("HUH? I DON’T UNDERSTAND THAT ROOM. TRY AGAIN.\n\n");
    }
    // else
    else
    {
        // add destination to path
        _add_to_path(g, g->room_names[dest_idx]);
        // set curr_room to destination
        g->curr_room = dest_idx;
        // increment step count
        (g->num_steps)++;
        // if curr_room type is "ROOM_END"
        if (strcmp(g->room_types[dest_idx], "END_ROOM") == 0)
        {
            // return 1
            return 1;
        }
    }

    // return 0
    return 0;
}

/*
 * print_stats()        prints the number of steps and path taken
 * pre-conditions       game has been setup
 * post-condittions     stats have been printed
 */
int print_stats(struct game * g)
{
    // declare i
    int i;

    // print first line
    printf("YOU TOOK %d STEPS. YOUR PATH TO VICTORY WAS:\n", g->num_steps);

    // print path
    for (i = 0; i < g->path_len; i++) { printf("%s\n", g->path[i]); }
}

/*** hidden helpers ***/
int _find_parameters(char * rooms_path,
                     int * max_name_len,
                     int * max_type_len,
                     int * num_rooms,
                     int ** num_cons)
{
    // constants
    char name_prefix[] = "ROOM NAME: ";
    char type_prefix[] = "ROOM TYPE: ";
    char con_prefix[] = "CONNECTION ";

    // variables
    struct dirent *ep;
    int buff_max_len = 40;
    char * ind_room_path = malloc(buff_max_len * sizeof(char));
    char * buffer = malloc(buff_max_len * sizeof(char));
    FILE * room_file;
    DIR *dp;
    int i = 0;

    // initial values
    for(i = 0; i < *num_rooms; i++) { (*num_cons)[i] = 0; }
    *max_name_len = 0;
    *max_type_len = 0;
    i = 0;

    // open stuff
    dp = opendir(rooms_path);

    // for each file in the rooms_path
    while (ep = readdir(dp))
    {
        // ignore . and ..
        if (strcmp(ep->d_name, ".") == 0) continue;
        else if (strcmp(ep->d_name, "..") == 0) continue;

        // open the file (for each line)
        sprintf(ind_room_path, "%s/%s", rooms_path, ep->d_name);
        if ((room_file = fopen(ind_room_path, "rw")) == NULL) return 1;

        while (fgets(buffer, buff_max_len, room_file) != NULL)
        {
            // if line starts with "ROOM NAME: "
            if (strstr(buffer, name_prefix) != NULL)
            {
                // find ( (line length) - (length of "ROOM NAME: ") )
                int tmp = strlen(buffer) - strlen(name_prefix);
                // if ^^ is greater than max_name_len
                if (tmp > *max_name_len) *max_name_len = tmp;
            }
            // else if line starts with "ROOM TYPE: "
            else if (strstr(buffer, type_prefix) != NULL)
            {
                // find ( (line length) - (length of "ROOM TYPE: ") )
                int tmp = strlen(buffer) - strlen(type_prefix);
                // if ^^ is greater than max_type_len
                if (tmp > *max_type_len) *max_type_len = tmp;
            }
            else if (strstr(buffer, con_prefix) != NULL)
            {
                ((*num_cons)[i])++;
            }
        }
        
        // close file
        fclose(room_file);

        // increment room index
        i++;
    }

    // free stuff and stuff
    free(buffer);
    free(ind_room_path);

    // close dir
    closedir(dp);

    // return
    return 0;
}

int _find_num_rooms(char * rooms_path,
                    int * num_rooms)
{
    // variables
    struct dirent *ep;
    *num_rooms = 0;
    DIR *dp;
    
    // open stuff
    dp = opendir(rooms_path);

    // for each file in the rooms_path
    while (ep = readdir(dp))
    {
        // ignore . and ..
        if (strcmp(ep->d_name, ".") == 0) continue;
        else if (strcmp(ep->d_name, "..") == 0) continue;

        // increment number of rooms
        (*num_rooms)++;
    }

    // close dir
    closedir(dp);

    // return
    return 0;
}

int _print_curr_room(struct game * g)
{
    int i;

    // first line (current name)
    printf("CURRENT LOCATION: %s\n", g->room_names[g->curr_room]);

    // second line (possible connections)
    printf("POSSIBLE CONNECTIONS: ");
    for (i = 0; i < g->num_cons[g->curr_room]; i++)
    {
        printf(g->room_cons[g->curr_room][i]);
        if (i < g->num_cons[g->curr_room] - 1) { printf(", "); }
        else { printf(".\n"); }
    }

    // third line (input)
    printf("WHERE TO? >");
}

/*
 * add a room of new_room_name to the path taken
 */
int _add_to_path(struct game * g, char * new_room_name)
{
    // add room
    strcpy(g->path[g->path_len], new_room_name);

    // increment len
    (g->path_len)++;

    // check if path needs to grow
    if (g->path_len == g->path_size)
    {
        // double size of path arr
        _grow_path(g);
    }
}

/*
 * double the size of the path member
 */
int _grow_path(struct game * g)
{
    // declare i
    int i;

    // double path size
    g->path_size = 2 * g->path_size;

    // malloc new array, copy elements
    char ** tmp_arr = malloc(g->path_size * sizeof(char*));
    for (i = 0; i < g->path_size; i++)
    {
        tmp_arr[i] = malloc((g->max_name_len+1) * sizeof(char));
        if (i < g->path_len) { strcpy(tmp_arr[i], g->path[i]); }
        if (i < g->path_size / 2) { free(g->path[i]); }
    }

    // free old array
    free(g->path);

    // set g->path to new array
    g->path = tmp_arr;
}


/*** main ***/
int main()
{
    // declarations
    char newest_dir[40];
    int step_result;
    struct game g;
    
    // rooms directory selection parameters
    char regex_pattern[] = "^olsonn2\\.rooms\\..*$";
    char rooms_src_dir[] = ".";

    // get newest directory of rooms
    if (get_newest_rooms(newest_dir, rooms_src_dir, regex_pattern) != 0) { return 1; }

    // setup game using rooms
    sprintf(rooms_src_dir, "%s/%s", rooms_src_dir, newest_dir);
    if (setup_game(&g, rooms_src_dir) != 0) { return 2; }

    // run until game is over
    while ((step_result = step_game(&g)) == 0);

    // check which non-zero return it was
    switch (step_result) {
        case 1:
            printf("YOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\n");
            break;

        case 2:
            printf("YOU HAVE ENDED YOUR JOURNEY EARLY. NICE TRY!\n");
            break;

        default:
            printf("Error stuff! RIP.\n");
            break;
    }

    // print stats
    print_stats(&g);

    // free game memory
    if (free_game(&g) != 0) { }//return 3; }

    // return
    return 0;
}
