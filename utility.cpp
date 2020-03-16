#include "myshell.h"

char ErrorMessage[30] = "An error has occurred\n";

//This function should free up space used by Command, and is opposite logic to allocating space function,
void deallocateInputCommand (Command *userCommand) {
    for (int i = 0; i < MAXIMUMARGUEMENTS; i++) {
        //Ignore if NULL, as that marks the end of the args list anyways.
        if ((*userCommand).argv[i] == nullptr)
            break;
        //free each instance of the command
        free((*userCommand).argv[i]);
    }

    //Here, I just free up redirection occurences (standard ins/outs), as well as a final free of command at the end
    if ((*userCommand).redirectSTDIN) {
        free((*userCommand).redirectSTDIN);
    }
    if ((*userCommand).redirectSTDOUT) {
        free((*userCommand).redirectSTDOUT);
    }

    free(userCommand);
}// end of deallocateInputCommand

//Here, I use my own version of strdup in order to stop running if there is an issue.
//On the linux manual, it said that strdup() "r"eturns NULL if insufficient memory was available", which is what
//I check for in the inner if statement.
static char *stringDuplicate(const char *stringOne) {
    char *returnValue = strdup(stringOne);
    //If NULL is indeed returned, I can't give anymore memory.
    if (returnValue == nullptr) {
        cerr <<ErrorMessage<<endl;
        //I exit with 127 because the command can't be found due to lack of space
        exit(127);
    }
    return returnValue;
}//end of stringDuplicate

/* Allocate an empty command, terminating on allocation failure. */
//Here, I use opposite logic of deallocating space function. It stops if the return value is NULL
static Command *commandAllocationSpace () {
    //I used calloc instead of malloc so that I can have the space filled with blanks by default.
    void *returnValue = calloc(1, sizeof(Command));
    //error handling
    if (returnValue == nullptr) {
        cerr <<ErrorMessage<<endl;
        //I exit with 127 because the command can't be found due to lack of space
        exit(127);
    }
    return (Command *) returnValue;
}//end of commandAllocationSpace

//Function to go ahead and read/parse commands
Command *parseInputCommand(const char *inputCL) {

    //error handling to see if the command is blank
    if (inputCL == nullptr){
        cerr <<ErrorMessage<<endl;
        //I exit with 127 because that's the eit code if command can't be found/is just wrong
        exit(127);
    }
    //Otherwise, I allocate space for a new command.
    Command *returnValue = commandAllocationSpace();

    /* Index into result->args of next argument. */
    int argc = 0;

    //Here I make a local copy of the command for further tokenization.
    char *commandBuffer = stringDuplicate(inputCL);
    //These token values were pre defined in the header file.
    char *token = strtok(commandBuffer, TOKENVALUES);

    while (token != nullptr) {
        //Then the first step I check is if the command involves redirection of some sort.
        //The first one I check is out redirection
        if (token[0] == '>') {
            //Error handling
            if ((*returnValue).redirectSTDOUT) {
                cerr <<ErrorMessage<<endl;
                exit(127);
            }
                //Continue on if the command is not terminated
            else if (token[1] != '\0') {
                int wordPos = 1;
                //Here I check if the character in the command contians >, and if it does, continue on
                //and make the program realize it's in redirect out mode,
                if (token[wordPos] == '>') {
                    wordPos++;
                    (*returnValue).isRedirectOut = true;
                }
                //If it's not a >, once again copy the command locally starting at whatever index WordPos is in
                if (token[wordPos] != '\0') {
                    //Making sure there is enough space, as well as copying everything.
                    (*returnValue).redirectSTDOUT = stringDuplicate(token + wordPos);
                }
                //Here I check if the token is not a NULL, then apply same logic as above
                else if ((token = strtok(nullptr, TOKENVALUES)) != nullptr) {
                    (*returnValue).redirectSTDOUT = stringDuplicate(token);
                }
            }
                //Otherwise, for the output file, I get the next command
            else if ((token = strtok(nullptr, TOKENVALUES)) != nullptr) {
                (*returnValue).redirectSTDOUT = stringDuplicate(token);
            }
                //Otherwise, if the > is at the end of the command, then that is not allowed
            else {
                cerr <<ErrorMessage<<endl;
                exit(127);
            }
        }
            //Here, I use the exact same logic and error handling as the last one, except this is for
            //redirecting in
        else if (token[0] == '<') {
            if ((*returnValue).redirectSTDIN) {
                cerr <<ErrorMessage<<endl;
                exit(127);
            } else if (token[1] != '\0') {
                /* <file (one word) */
                (*returnValue).redirectSTDIN = stringDuplicate(token + 1);
            } else if ((token = strtok(nullptr, TOKENVALUES)) != nullptr) {
                (*returnValue).redirectSTDIN = stringDuplicate(token);
            } else {
                cerr <<ErrorMessage<<endl;
                exit(127);
            }
        }
            //Otherwise, there is no redirection at all
        else {
            /* Not a redirection, must be an argument. Make sure
             * there is room in the args array for both this arg
             * and a sentinel NULL pointer. */
            //Here I do some error handling to make sure the user has not put too many arguments in the command.
            //There must also be space at the end for the nullptr
            if (argc >= MAXIMUMARGUEMENTS) {
                cerr <<ErrorMessage<<endl;
                exit(127);
            }
            //update/conitnue
            (*returnValue).argv[argc++] = stringDuplicate(token);
        }
        //Update/grab the next token.
        token = strtok(nullptr, TOKENVALUES);
    }
    //free up usage space then return.
    free(commandBuffer);
    return returnValue;
}

//This is the function for changing directory
void CDCommand(Command * inputCommand) {
    //Default set the directory to null
    char * directory = nullptr;
    //Here I use a retrieved code value
    int rc = 0;

    // The input given after cd is what I set to the directory with this if statement
    // after next argument after 'cd' is dir name
    if ((*inputCommand).argv[1]) {
        directory = inputCommand->argv[1];
    }
        //Otherwise, if no arguments were actually typed in after cd, I just grab the HOME environment variable
    else {
        directory = getenv("HOME");
    }

    //Here, I just use some error handling to make sure that the retrieval value fo changing a directory was successful,
    //and valid (i.e. not a negative value). I do this using chdir().
    if ((rc = chdir(directory)) < 0) {
        cerr <<ErrorMessage<<endl;
    }
}

//I use this printf statement as provided by TA, for when clr or clear is entered
void clearCommand() {
    printf("\033[H\033[2J");
}

// to run 'dir' command to show list of files for given dir
void showDirectory(Command * inputCommand) {
    Command localCommand;
    static char list[] = "ls";
    static char cwd[] = ".";
    localCommand.argv[0] = list;
    //Here I check if the directory is provided by the user or not. If it is not, by default it
    //will be pwd
    localCommand.argv[1] = (*inputCommand).argv[1] ? (*inputCommand).argv[1] : cwd; // set default dir to current when no dir path is provided
    //set last argument to NULL
    localCommand.argv[2] = nullptr;

    //This will set appendMode to true if there is >>
    localCommand.isRedirectOut = (*inputCommand).isRedirectOut;
    //This will will cause the output's redirection
    localCommand.redirectSTDOUT = (*inputCommand).redirectSTDOUT; // redirect output stream
    //This will will cause the output's redirection
    localCommand.redirectSTDIN = (*inputCommand).redirectSTDIN;
    //Finally, execute.
    executeExternalCommand(&localCommand);
}

//This function will print the environment
void printEnvironment(Command *inputCommand) {
    //standard out stream
    ofstream fileOut;
    //This statement handles outgoing outputs
    if ((*inputCommand).redirectSTDOUT) { // when output stream is redirected to file
        //Same logic as echo function for opening. It then opens a file, and if the redirection is out, it will append it to the end of it, or truncate otherwise
        fileOut.open((*inputCommand).redirectSTDOUT, (*inputCommand).isRedirectOut ? ofstream::app : ofstream::trunc);
        //Error handling if unable to open a file for whatever reason.
        if (!fileOut.is_open()) {
            cerr <<ErrorMessage<<endl;
            return ;
        }
    }
    //Here I just double check if the ostream is cout or the given fileOut, and set it if provided.
    ostream &outStream = ((*inputCommand).redirectSTDOUT == nullptr) ? cout : fileOut;
    //I use the built-in environment variable to copy it to a local one.
    extern char **environ;
    char *environment = *environ;
    //This for loop finally prints out all the environment variables
    for (int i = 1; environment; i++) {
        //print line
        outStream << environment << endl;
        //update local variable to print the next actual line of built-in environ
        environment = *(environ + i);
    }
}

//Function to print messages
void echo(Command *inputCommand) {
    ofstream fileOut;
    //This if statement checks if the given command is to be redirected out.
    if ((*inputCommand).redirectSTDOUT) {
        //Same logic as environment function for opening. It then opens a file, and if the redirection is out, it will append it to the end of it, or truncate otherwise
        fileOut.open((*inputCommand).redirectSTDOUT, (*inputCommand).isRedirectOut ? ofstream::app : ofstream::trunc);

        //Error handling for unable to open the file to output.
        if (!fileOut.is_open()) {
            cerr <<ErrorMessage<<endl;
            return ;
        }
    }
    //Here I just double check if the ostream is cout or the given fileOut, and set it if provided.
    ostream &outStream = ((*inputCommand).redirectSTDOUT == nullptr) ? cout : fileOut;

    int i=0;
    //Finally, I print it all out, separated by spaces, starting from first argument
    while ((*inputCommand).argv[++i]) {
        outStream << (*inputCommand).argv[i] << " ";
    }
    outStream << endl;
}


