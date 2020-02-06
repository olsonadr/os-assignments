/*** includes ***/
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>


/*** prototypes ***/
int roomConsSatisfyReqs(char ***, int, int, int);
int getNumCons(char ***, int);
int isValidCon(char ***, char **, int, int, int, int);
int conAlreadyExists(char ***, int, char *);
int main();


/*** helper functions ***/
/* function:    roomConsStatisfyReqs
 * arguments:   char *** roomConsStart - desc
 *              int minCons - desc
 *              int maxCons - desc
 *              int numRooms - desc
 * pre-cond:    desc
 * post-cond:   desc
 * returns:     desc
 */
int roomConsSatisfyReqs(char *** roomConsStart,
                        int minCons, int maxCons,
                        int numRooms)
{
    int i;
    int currNumCons;
    for (i = 0; i < numRooms; i++)
    { 
        currNumCons = getNumCons(roomConsStart, i);
        if (currNumCons < minCons || currNumCons > maxCons) { return 0; }
    }
    return 1;
}


/* function:    getNumCons
 * arguments:   char *** roomConsStart - desc
 *              int roomIdx - desc
 * pre-cond:    desc
 * post-cond:   desc
 * returns:     desc
 */
int getNumCons(char *** roomConsStart, int roomIdx)
{
    int count = -1;
    while(**(*(roomConsStart+roomIdx)+(++count)) != '\n');
    return count;
}


/* function:    conAlreadyExists
 * arguments:   char *** roomConsStart - desc
 *              int roomIdx1 - desc
 *              char * name - desc
 * pre-cond:    desc
 * post-cond:   desc
 * returns:     desc
 */
int conAlreadyExists(char *** roomConsStart, int roomIdx1, char * name)
{
    int count = -1;
    while(**(*(roomConsStart+roomIdx1)+(++count)) != '\n')
        { if (strcmp(*(*(roomConsStart+roomIdx1)+count), name) == 0) { return 1; } }
    return 0;
}


/* function:    isValidCon
 * arguments:   char *** roomConsStart - desc
 *              char *** roomNamesStart - desc
 *              int roomIdx1 - desc
 *              int roomIdx2 - desc
 *              int minCons - desc
 *              int maxCons - desc
 * pre-cond:    desc
 * post-cond:   desc
 * returns:     desc
 */
int isValidCon(char *** roomConsStart, char ** roomNamesStart,
                int roomIdx1, int roomIdx2, int minCons, int maxCons)
{
    return roomIdx1 != roomIdx2
            &&  !(getNumCons(roomConsStart, roomIdx1) >= maxCons)
            &&  !(getNumCons(roomConsStart, roomIdx1) >= maxCons)
            &&  !(conAlreadyExists(roomConsStart, roomIdx1, *(roomNamesStart+roomIdx2)));
}


/*** main ***/
/* function:    main
 * pre-cond:    desc
 * post-cond:   desc
 * returns:     desc
 */
int main()
{
    // parameters
    int numRooms    = 7;
    int minCons     = 3;
    int maxCons     = 6;
    int typeLenMax  = 10;
    int nameLenMax  = 8;
    int numNames    = 10;
    
    // variables to come
    char ** currName;
    time_t t;
    int i;
    int j;
    
    // seed random number
    srand((unsigned) time(&t));
    
    // create list of possible names
    char ** nameStart = malloc(numNames * sizeof(char*));
    for (i = 0; i < numNames; i++) { *(nameStart + i) =  malloc((nameLenMax) * sizeof(char)); }
    char *names[] = {  "name1", "name2", "name3", "name4", "name5",
                      "name6", "name7", "name8", "name9", "name10"  };
    for (i = 0; i < numNames; i++) { sprintf(*(nameStart+i), "%s", names[i]); }
    currName = nameStart;

    // create and assign rooms
    char **  roomNamesStart = malloc( numRooms * sizeof(char*) );
    char **  roomTypesStart = malloc( numRooms * sizeof(char*) );
    char *** roomConsStart  = malloc( numRooms * sizeof(char*) );

    for (i = 0; i < numRooms; i++)
    {   *(roomNamesStart + i) = malloc( (nameLenMax) * sizeof(char) );
        *(roomTypesStart + i) = malloc( (typeLenMax + 1) * sizeof(char) );
        *(roomConsStart  + i) = malloc( (maxCons + 1) * sizeof(char*) );
        
        for (j = 0; j < maxCons + 1; j++)
        {   *(*(roomConsStart + i) + j) = malloc( (nameLenMax) * sizeof(char) );  } }
    
    char *** currRoomCons = roomConsStart;
    char **  currRoomName = roomNamesStart;
    char **  currRoomType = roomNamesStart;

    for (i = 0; i < numRooms; i++)
    {   // choose room name
        int nameIdx = (rand() % (numNames - i));
        strcpy(*(roomNamesStart + i), *(currName + nameIdx));
        if (nameIdx != 0) {
            strcpy(*(currName + nameIdx), *currName);
            strcpy(*(currName), *(roomNamesStart + i));
        }
        currName++;

        // add newline caps to signify no connections yet
        for (j = 0; j < maxCons + 1; j++) { sprintf(*(*(currRoomCons + i) + j), "%c", '\n'); }
        
        // assign type
        strcpy(*(roomTypesStart + i), 
                ((i == 0) ? ("START_ROOM") : ((i == numRooms - 1)
                                                ? ("END_ROOM") : ("MID_ROOM") )));  }

    // assign connections
    int randRoom1;
    int randRoom2;
    char ** finalConPtr1;
    char ** finalConPtr2;

    while (!roomConsSatisfyReqs(roomConsStart, minCons, maxCons, numRooms))
    {   // get random rooms to connect
        while (!isValidCon(roomConsStart, roomNamesStart,
                            (randRoom1 = rand() % numRooms),
                            (randRoom2 = rand() % numRooms),
                            minCons, maxCons));
        
        // add connections
        finalConPtr1 = *(roomConsStart + randRoom1) + getNumCons(roomConsStart, randRoom1);
        finalConPtr2 = *(roomConsStart + randRoom2) + getNumCons(roomConsStart, randRoom2);
        strcpy(*finalConPtr1, *(roomNamesStart + randRoom2));
        strcpy(*finalConPtr2, *(roomNamesStart + randRoom1));

        // add new newline final string (has been incremented)
        sprintf(*(finalConPtr1 + 1), "%c", '\n');
        sprintf(*(finalConPtr2 + 1), "%c", '\n'); }

    // get pid
    char pidStr[10];
    sprintf(pidStr, "%d", getpid());
    
    // generate directory path
    char outDir[80];
    strcpy(outDir, "./olsonn2.rooms.");
    strcat(outDir, pidStr);

    // make output directory
    mkdir(outDir, 0777);

    // write files
    FILE * currRoomFP;
    char outFilePath[80];

    for (i = 0; i < numRooms; i++)
    {
        // open file
        sprintf(outFilePath, "%s/room.%d.%s", outDir, i, pidStr);
        currRoomFP = fopen(outFilePath, "w");

        // write name to file
        fprintf(currRoomFP, "ROOM NAME: %s\n", *(roomNamesStart + i));

        // write connections to file
        j = 0;
        while (**(*(roomConsStart + i) + j) != '\n')
        {
            fprintf(currRoomFP, "CONNECTION %d: %s\n", j+1, *(*(roomConsStart + i) + j));
            j++;
        }

        // write room type to file
        fprintf(currRoomFP, "ROOM TYPE: %s\n", *(roomTypesStart + i));

        // close file
        fclose(currRoomFP);
    }

    // free stuff
    for (i = 0; i < numRooms; i++)
    {   for (j = 0; j < maxCons + 1; j++)
            { free(*(*(roomConsStart + i) + j)); }
        free(*(roomNamesStart + i));
        free(*(roomTypesStart + i));
        free(*(roomConsStart + i));  }
    for (i = 0; i < numNames; i++) { free( *(nameStart + i) ); }
    free(nameStart);
    free(roomConsStart);
    free(roomNamesStart);
    free(roomTypesStart);
}
