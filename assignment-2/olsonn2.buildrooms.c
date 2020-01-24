/* includes */
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

/* prototypes */
int roomConsSatisfyReqs(char ***, int, int, int);
int getNumCons(char ***, int);
int main();

/* helper functions */
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

int getNumCons(char *** roomConsStart, int roomIdx)
{
    int count = -1;
    while(**(*(roomConsStart+roomIdx)+(++count)) != '\n');
    return count;
}

/* main */
int main()
{
    // seed random number
    time_t t;
    srand((unsigned) time(&t));

    // parameters
    int numRooms = 7;
    int minCons = 3;
    int maxCons = 6;
    int typeLenMax = 10;
    int nameLenMax = 8;
    int numNames = 10;
    
    // vars
    int i;
    int j;
    char ** currName;
    
    // create list of possible names
    char ** nameStart = malloc(numNames * sizeof(char*));
    for (i = 0; i < numNames; i++) { *(nameStart + i) =  malloc((nameLenMax) * sizeof(char)); }
    char ** nameEnd = nameStart;

    /*currName = nameStart;
    sprintf(*(nameEnd++), "name1");
    sprintf(*(nameEnd++), "name2");
    sprintf(*(nameEnd++), "name3");
    sprintf(*(nameEnd++), "name4");
    sprintf(*(nameEnd++), "name5");
    sprintf(*(nameEnd++), "name6");
    sprintf(*(nameEnd++), "name7");
    sprintf(*(nameEnd++), "name8");
    sprintf(*(nameEnd++), "name9");
    sprintf(*(nameEnd++), "name10"); */

    /*for (i = 0; i < numNames; i++)
    {
        sprintf(*(nameEnd++), "name%d", i);
    }*/

    
    char *names[] = {  "name1", "name2", "name3", "name4", "name5",
                      "name6", "name7", "name8", "name9", "name10"  };

    for (i = 0; i < numNames; i++) { sprintf(*(nameStart+i), "%s", names[i]); }
    /*
    nameStart = names;*/
    currName = nameStart;

    // create and assign rooms
    char **  roomNamesStart = malloc( numRooms * sizeof(char*) );
    char **  roomTypesStart = malloc( numRooms * sizeof(char*) );
    char *** roomConsStart  = malloc( numRooms * sizeof(char*) );

    for (i = 0; i < numRooms; i++)
    {   
        *(roomNamesStart + i) = malloc( (nameLenMax) * sizeof(char) );
        *(roomTypesStart + i) = malloc( (typeLenMax) * sizeof(char) );
        *(roomConsStart  + i) = malloc( (maxCons + 1) * sizeof(char*) );
        
        for (j = 0; j < maxCons + 1; j++)
        {
            *(*(roomConsStart + i) + j) = malloc( (nameLenMax) * sizeof(char) );
        }
    }
    
    char *** currRoomCons = roomConsStart;
    char **  currRoomName = roomNamesStart;
    char **  currRoomType = roomNamesStart;

    for (i = 0; i < numRooms; i++)
    {
    printf("\n\n");
    for (j = 0; j < numNames - 1; j++)
    {
        printf("%s, ", *(currName+j));
    }
    printf("%s\n", *(currName + numNames - 1));

        // choose room name
        int nameIdx = (rand() % (numNames--));
        printf("requesting name %d\n which is name \"%s\"\n\n", nameIdx, *(currName+nameIdx));
        strcpy(*currRoomName, *(currName + nameIdx));
        if (nameIdx != 0) {
            strcpy(*(currName + nameIdx), *currName);
            strcpy(*(currName), *(currRoomName));
        }
        currName++;
        currRoomName++;

        // add newline caps to signify no connections yet
        ***(currRoomCons++) = '\n';

        // assign type
        *(currRoomType++) = (i == 0) ? ("START_ROOM") : (
                                (i == numRooms) ? ("END_ROOM") : ("MID_ROOM") );
    }

    // assign connections
    int randRoom1;
    int randRoom2;
    char ** finalConPtr1;
    char ** finalConPtr2;

    while (!roomConsSatisfyReqs(roomConsStart, minCons, maxCons, numRooms))
    {
        // get random rooms to connect
        while (  getNumCons(roomConsStart, (randRoom1 = rand() % numRooms)) > maxCons );
        while ( (getNumCons(roomConsStart, (randRoom2 = rand() % numRooms)) > maxCons)
                || (randRoom2 == randRoom1) );

        // add connections
        finalConPtr1 = *(roomConsStart + randRoom1) + getNumCons(roomConsStart, randRoom1);
        finalConPtr2 = *(roomConsStart + randRoom2) + getNumCons(roomConsStart, randRoom2);
        strcpy(*finalConPtr1, *(roomNamesStart + randRoom2));
        strcpy(*finalConPtr2, *(roomNamesStart + randRoom1));

        // add new newline final string (has been incremented)
        **(finalConPtr1 + 1) = '\n';
        **(finalConPtr2 + 1) = '\n';
    }

    // free possible names
    for (i = 0; i < numNames; i++) { free(*(nameStart + i)); }
    free(nameStart);

    // get pid
    char pidStr[10];
    sprintf(pidStr, "%d", getpid());
    
    // generate directory path
    char outDir[80];
    strcpy(outDir, "./rooms.");
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
        currRoomFP = fopen("w", outFilePath);

        // write name to file
        fprintf(currRoomFP, "ROOM NAME: %s\n", *(currRoomName++));

        // write connections to file
        j = 1;
        while (***currRoomCons != '\n')
        {
            fprintf(currRoomFP, "CONNECTION %d: %s\n", j, *(*currRoomCons)++);
            j++;
        }

        // move to next room's connections
        currRoomCons++;

        // write room type to file
        fprintf(currRoomFP, "ROOM TYPE: %s\n", *(currRoomType++));

        // close file
        fclose(currRoomFP);
    }

    // free stuff
    for (i = 0; i < numRooms; i++)
    {   
        for (j = 0; j < maxCons; j++)
        {
            free(*(*(roomConsStart + i) + j));
        }

        free(*(roomNamesStart + i));
        free(*(roomTypesStart + i));
        free(*(roomConsStart + i)); 
    }
    free(roomConsStart);
    free(roomNamesStart);
    free(roomTypesStart);
}
