#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <ctype.h> 
//readline includes
#include<readline/readline.h>
#include<readline/history.h>

void quitHandler(int);

int main() {
    int exitInt = 0;
    int sizeOfArray = 1;
    char letter;
    char *inputList = malloc(sizeof(char) * sizeOfArray); 
    char **myArgv = malloc(sizeof(char*) * sizeOfArray);
    char *word;
    int inputCounter = 0;
    int myArgc = 0;
    int x = 0;
    int i = 0;
    int loop2 = 1;
    int letterChecker = 0;

    //part 2 variables
    int isWait = 1;
    int pid, i2;
    pid_t childPID;
    int status, saved_status;
    signal(SIGINT, quitHandler);

    //part 3 variables
    int numOfPipes = 0;
    int counter = 0;
    int pipeArrayLocationSize = 10;
    int *pipeLocations = malloc(sizeof(int) * pipeArrayLocationSize);

    //history section
    int historyMaxSize = 10;
    char *readHistory = malloc(sizeof(char) * 100);
    char *historyArray;
    FILE *historyFile;
    historyFile = fopen(".myhistory", "r");
    if (historyFile == NULL) {
        printf("Couldn't not open history file. Making new one.\n");
        historyFile = fopen(".myhistory", "w+");
    }
    stifle_history(historyMaxSize); //set size of history list
    int count1 = 0;
    int letterTriggerForHistory = 0;
    int m = 0;

    //read in from file and add to history array
    char historyLetter;
    int historyReadCounter = 0;
    int historyTrigger = 1;
    int historyCurrentSize = 0;
    while ((historyLetter = fgetc(historyFile))!= EOF) {

        readHistory[historyReadCounter] = historyLetter;

        if (historyLetter == '\n') {
            readHistory[historyReadCounter] = NULL;
            //printf("%c", historyLetter);
            add_history(readHistory);
            historyReadCounter = 0;
            historyCurrentSize += 1;
        } else {
            historyReadCounter += 1;
        }
        
        
    }
    fclose(historyFile);

    //main program loop
    while (!exitInt) {

        //reset variables
        int f_des[2];
        int f_des2[2];
        if (pipe(f_des) == -1) {
            perror("Pipe");
            exit(1);
        }
        if (pipe(f_des2) == -1) {
            perror("Pipe_2");
            exit(1);
        }

        loop2 = 1;
        myArgc = 0;
        inputCounter = 0;
        letterChecker = 0;
        isWait = 1;
        sizeOfArray = 1;
        numOfPipes = 1;
        counter = 0;
        pipeLocations[0] = -1;
        pipeArrayLocationSize = 10;
        pipeLocations = realloc(pipeLocations, sizeof(int) * 1);
        pipeLocations = realloc(pipeLocations, sizeof(int) * pipeArrayLocationSize);
        historyTrigger = 1;

        //print out prompt
        //printf("Enter Command: ");
        historyArray = readline("Enter Command: ");
        count1 = 0;
        letterTriggerForHistory = 0;
        while (historyArray[count1] != '\0') {
            if (isalpha(historyArray[count1])) {
                letterTriggerForHistory = 1;
            }
            count1 += 1;
        }

        if ((strlen(historyArray) > 0) && (letterTriggerForHistory)) {
            add_history(historyArray);
            historyCurrentSize += 1;
        }

        //read in string and store in one string
        m = 0;
        while (historyArray[m] != '\0') {
            inputList[inputCounter] = historyArray[m];

            //checks to make sure there is a letter in the input. So that if space is only entered it wont crash
            if (isalpha(inputList[inputCounter])) {
                letterChecker = 1;
            }

            inputCounter++;

            //if it reaches the end of the array resize the array for more space
            if (inputCounter >= sizeOfArray) {
                sizeOfArray += sizeOfArray;
                inputList = realloc(inputList, sizeOfArray);
            }
            m++;

        }

        if (inputList[inputCounter-1] == '&') {
            isWait = 0;
            inputList[inputCounter-1] = ' ';
        }


        //if the line is empty then do not run next while loop
        if (inputCounter == 0) {
            loop2 = 0;
        }

        //add on \0 to the string
        inputList[inputCounter] = '\0';
        
        //split up the inputList into words and put into the myArgv array structure
        myArgc = 0;
        sizeOfArray = 1;
        word = strtok(inputList, " ");

        while (loop2 && letterChecker) {

            if (strcmp(word, "|") == 0) {
                pipeLocations[numOfPipes] = myArgc;
                numOfPipes += 1;
                if (numOfPipes > pipeArrayLocationSize) {
                    pipeArrayLocationSize = pipeArrayLocationSize * 2;
                    pipeLocations = realloc(pipeLocations, sizeof(int) * pipeArrayLocationSize);
                }
            }

            myArgv[myArgc] = word;
            myArgc++;

            //get next word from input
            word = strtok(NULL, " ");

            //if it reaches the end of the array resize the array for more space
            if (myArgc >= sizeOfArray) { 
                sizeOfArray += sizeOfArray;
                myArgv = realloc(myArgv, sizeof(char*) * sizeOfArray);
            }

            //check for end of string
            if (word == NULL) {
                loop2 = 0;
            }
        }

        numOfPipes -= 1;

        //add NULL Pointer to end of the array
        myArgv[myArgc] = NULL;

        //check for exit
        if (myArgc > 0) {
            if (myArgc == 1) {
                //check for exit in argv 0
                if ((strcmp(myArgv[0], "exit")) == 0) {
                    exitInt = 1;
                }
            }
        }

        //check for history
        if (myArgc > 0) {
            if (myArgc >= 1) {
                //check for exit in argv 0
                if ((strcmp(myArgv[0], "history")) == 0) {

                    HIST_ENTRY **printHistoryToTerminal = history_list();

                    if (myArgc > 1) {
                        if (atoi(myArgv[1]) < historyMaxSize) {

                            int tmpInt;
                            
                            if (historyCurrentSize > historyMaxSize) {
                                tmpInt = historyMaxSize - atoi(myArgv[1]);

                                for (int a = 0; a < atoi(myArgv[1]); a++) {
                                    printf("%i %s\n", (tmpInt+1), printHistoryToTerminal[tmpInt]->line);
                                    tmpInt += 1;
                                }

                            } else {
                                tmpInt = historyCurrentSize - atoi(myArgv[1]);

                                if (tmpInt < 0) {
                                    for (int a = 0; printHistoryToTerminal[a] != NULL; a++) {
                                        printf("%i %s\n", (a+1), printHistoryToTerminal[a]->line);
                                    }

                                } else {
                                    for (int a = 0; a < atoi(myArgv[1]); a++) {

                                        printf("%i %s\n", (tmpInt+1), printHistoryToTerminal[tmpInt]->line);
                                        tmpInt += 1;

                                    }
                                }
                            }
                            
                        }else {
                            for (int a = 0; printHistoryToTerminal[a] != NULL; a++) {
                                printf("%i %s\n", (a+1), printHistoryToTerminal[a]->line);
                            }
                        }
                    } else {
                        for (int a = 0; printHistoryToTerminal[a] != NULL; a++) {
                            printf("%i %s\n", (a+1), printHistoryToTerminal[a]->line);
                        }
                    }

                    

                    historyTrigger = 0;
                } else if (strcmp(myArgv[0], "erase") == 0) {
                    if (strcmp(myArgv[1], "history") == 0) {
                        clear_history();
                        historyTrigger = 0;
                        historyCurrentSize = 0;
                    }
                }
            }
        }

        //make pipes NULL
        if (numOfPipes > 0) {
            for (int a = 1; a <= numOfPipes; a++) {
                myArgv[pipeLocations[a]] = NULL;
            }
        }

        //execvp
        if (!exitInt) {

            if (historyTrigger) {
                //check to see if array is not empty
                if (myArgc > 0) {

                    if (numOfPipes > 0) {
                        for (int a = 0; a <= numOfPipes; a++) {

                            pid = fork();
                            if (pid == -1) {
                                perror("error at fork\n");
                                exit(1);
                            } else if (pid == 0) {

                                if (a == 0) {
                                    //printf("- First Command\n");
                                    close(f_des[0]);
                                    dup2(f_des[1], fileno(stdout));
                                    //close(f_des[0]);
                                    //close(f_des[1]);
                                } else if (a >= numOfPipes) {

                                    //printf("- Last Command\n");
                                        
                                    if (a % 2 == 0) {
                                        close(f_des2[1]);
                                        dup2(f_des2[0], fileno(stdin));
                                    } else {
                                        close(f_des[1]);
                                        dup2(f_des[0], fileno(stdin));
                                    }
                                } else {
                                    //printf("- Middle Command\n");

                                    if (a % 2 == 0) {
                                        close(f_des[0]);
                                        dup2(f_des[1], fileno(stdout));
                                        close(f_des2[1]);
                                        dup2(f_des2[0], fileno(stdin));
                                    } else {
                                        close(f_des2[0]);
                                        dup2(f_des2[1], fileno(stdout));
                                        close(f_des[1]);
                                        dup2(f_des[0], fileno(stdin));
                                    }
                                        
                                }
                                    
                                //child processes
                                execvp(myArgv[pipeLocations[a]+1], &myArgv[pipeLocations[a]+1]);
                                perror("Error with input");
                                exit(i2);

                            }

                            if (a % 2 != 0) {
                                //close(f_des[1]);
                                close(f_des2[1]);
                            } else {
                                close(f_des[1]);
                            }
                            //close(f_des[1]);
                            //close(f_des2[1]);
                                
                            if (a >= numOfPipes) {
                                close(f_des[0]);
                                close(f_des2[0]);
                            }

                            while ((childPID = wait(&saved_status)) != -1) {
                                //continue after execvp ends
                            }

                        }

                    } else {
                        pid = fork();
                        if (pid == -1) {
                            perror("error at fork\n");
                            exit(1);
                        } else if (pid == 0) {

                            //child processes
                            execvp(myArgv[0], &myArgv[0]);
                            perror("Error with input");
                            exit(i2);
                        }

                    }
                    
                    if (isWait) {
                        while ((childPID = wait(&saved_status)) != -1) {
                            //continue after child ends
                        }
                    }

                    close(f_des[0]);
                    close(f_des[1]);

                }
            }

        }

    }

    historyFile = fopen(".myhistory", "w");

    //save history array
    HIST_ENTRY **historyListPrintToFile = history_list();

    for (int a = 0; historyListPrintToFile[a] != NULL; a++) {
        fprintf(historyFile, "%s\n", historyListPrintToFile[a]->line);
    }

    free(inputList);
    free(myArgv);
    free(pipeLocations);
    free(historyArray);
    
    fclose(historyFile);

    printf("Good Bye!\n");
    
    return 0;
}

void quitHandler (int theInt) {
    printf("\nPlease enter 'exit' to quit. \n");
    printf("Enter Command: ");
    fflush(stdout);
}


