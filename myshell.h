#ifndef MY_SHELL_H
#define MY_SHELL_H

//All headers needed under the header file
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <fstream>
#include <vector>
#define TOKENVALUES " \t\n"
using namespace std;

//Constant argument size declared here
const int MAXIMUMARGUEMENTS = 30;

//This is a command struct that I use to hold arguments/commands from the user, as well as details
//redirection
typedef struct {

    //This like the argv array you find in main. It's made up of pointers to the user commands,
    //all followed by an extra value at the end that is NULL. This will help mark EOF, as well
    //as be used for exec purposes
    char *argv[MAXIMUMARGUEMENTS + 1];

    //This is a boolean variable that will be True if the the redirection is outwards
    bool isRedirectOut; // true when output direction is >>

    //The following two are file variables, which will be set to nullptr if there in fact no redirection
    //This will be the file argument that will be used to redirect the stdout to
    char *redirectSTDOUT;
    //This will be the file argument that will be used to redirect the stdin from
    char *redirectSTDIN;


} Command ;

//This function will actually pass the given command in the command line and return a struct for the
//given command to be analyzed. It is then freed.
Command *parseInputCommand(const char *inputCL);
//This is used for values returned by the above function.
void executeExternalCommand(Command * inputCommand);
//This function will free all the data that userCommand took up using the malloc function.
void deallocateInputCommand(Command *userCommand);



void CDCommand(Command * inputCommand);
void clearCommand();
void showDirectory(Command * inputCommand);
void printEnvironment(Command *inputCommand);
void echo(Command *inputCommand);
void help();

#endif //PROJECTTWO_MYSHELL_H

