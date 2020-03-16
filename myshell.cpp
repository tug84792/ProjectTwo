#include "myshell.h"

vector <string> environmentPaths;
char errorMessage[30] = "An error has occurred\n";

// Here, I use enum, just like in the last lab, to determine what kind of command it is.
enum CommandType {
            //If the user just simply presses on enter
            BLANK,
            //Changing directory
            CD,
            //Clearing the screen
            CLEAR,
            //Directory
            DIR,
            //Getting the environment
            ENVIRON,
            //Echoing the consequent tet
            ECHO,
            //Asking for help/manual
            HELP,
            //Pausing the unix until the user presses enter
            PAUSE,
            //Shutting dow the shell
            QUIT,
            //All other commands, including piping, forking, redirection, etc.
            EXTERNAL
};

//This function will take in the command, and will return the approrpriate numeric value
CommandType getCommandType(const char* inputCommand) {
    // If the input is blank, do nothing and return.
    if (inputCommand == nullptr) {
        return BLANK;
    }
    //Here I convert the C input as char into a string
    string stringCommand(inputCommand);
    if (stringCommand == "cd") {
        return CD;
    }

    if (stringCommand == "clear" || stringCommand == "clr") {
        return CLEAR;
    }
    if (stringCommand == "dir") {
        return DIR;
    }
    if (stringCommand == "env" || stringCommand == "environ") {
        return ENVIRON;
    }
    if (stringCommand == "echo") {
        return ECHO;
    }
    if (stringCommand == "man" || stringCommand == "help") {
        return HELP;
    }
    if (stringCommand == "pause") {
        return PAUSE;
    }
    if (stringCommand == "quit" || stringCommand == "exit") {
        return QUIT;
    }
    return EXTERNAL;
}

//This function will do all the extra things when it comes to not running an internal command
void executeExternalCommand(Command * inputCommand) {
    //Make a child process
    pid_t pid = fork();
    //Error checking for a child process not working
    if (pid < 0) {
        cerr <<errorMessage<<endl;
        return;
    }
    //In case it's the parent process
    if (pid > 0) {
        //Here I use an int for status checking
        int processStatus;

        //Error handling in case waiting fails for whatever reason.
        if (waitpid(pid, &processStatus, 0) < 0) { // parent must wait for child to finish
            cerr << errorMessage;
            return;
        }

            //For the next few statements, I used various built-in process completion status keywords
            //This checks if a nonzero value is returned, indicating a successful child
        else if (WIFEXITED(processStatus)) {

            int rc = WEXITSTATUS(processStatus);
            //If rc is indeed positive, and the exit status of the child is succesfull, it reports that to the parent.
            if (rc) { // print the return code its non-zero
                cerr << errorMessage << endl;
            }
            return;
        }

            //Here, I do some more error handling. The if condition checks if there was a signal that was not handled
            //for the child process
        else if (WIFSIGNALED(processStatus)) {
            //If there was an unhandled signal, this will return the value of the ignored signal, in the variable signal.
            int signal = WTERMSIG(processStatus);
            cerr << errorMessage <<  "Command was killed by signal: " << strsignal(signal) << endl;
            return;
        }
    }
        //Child Process is running
    else if (pid == 0) {
        //Initialize in/out FDs
        int inFileDescriptor = -1, outFileDescriptor = -1;
        //This statement handles if the input stream is redirected to a file.
        //If it is, it will then open the file in order to read the commands
        if ((*inputCommand).redirectSTDIN ) {
            //Here, I have some error handling if I can't open the file in Read mode
            if ((inFileDescriptor = open((*inputCommand).redirectSTDIN, O_RDONLY)) < 0) {
                cerr << errorMessage <<endl;
                exit(0);
            }
        }
            //This if statement is basically the same as the last, except it has to do with writing
            //the output to a file.
        else if ((*inputCommand).redirectSTDOUT) {
            //Here, I create the file if it hasn't already been created, and make it in writing mode
            int givenFlags = O_CREAT | O_WRONLY;
            //This if statement handles instances where >> is used in the command line.
            if ((*inputCommand).isRedirectOut) { // when '>>' used for redirection then append
                //If it is, I append it to the end of the file
                givenFlags |= O_APPEND;
            }
                //Otherwise, for cases of >, I truncate the existing file
            else {
                givenFlags |= O_TRUNC;
            }
            //This if statement error handles in case I can't do redirection out for whatever reason,
            //using 0666 for reading and writing permission modes
            if ((outFileDescriptor = open((*inputCommand).redirectSTDOUT, givenFlags, 0666)) < 0 ){
                cerr << errorMessage << endl;
                exit(0);
            }
        }

        //Here, I deal with output files.
        if (outFileDescriptor > 0) {
            //Error handling, if I was not able to execute for whatever reason
            //(redirection to standard in return code is negative)
            if (dup2(outFileDescriptor , 1) < 0) {
                cerr << errorMessage << endl;
                exit(0);
            } else {
                //Otherwise, I close the FD redirected to standard out
                close(outFileDescriptor);
            }
        }
        //This if statement has to do with input file
        if (inFileDescriptor > 0) {
            //Same error handling
            if (dup2(inFileDescriptor , 0) < 0) { // redirect to stdin
                cerr << errorMessage << endl;
                exit(0);
            } else {
                //Otherwise, I close the FD redirected to standard in
                close(inFileDescriptor);
            }
        }
        //Here, I will find the path of my program
        string pathOfBinary;
        for (const string &givenPath: environmentPaths) {
            //Here I store the path along with the the first argument in the argv array in bin
            string localBin = givenPath + (*inputCommand).argv[0];
            //Here I check using the methid access if the process can access the given path. If it can, 0 is returned,
            //and it's in execution mode (X-OK).
            if (access(localBin.c_str(), X_OK) == 0) {
                //Update original path if it is
                pathOfBinary = localBin;
                break;
            }
        }
        //This allows us to use the in/output file with STDIN/OUT. And thus, we will exeucte that command in the process.
        //Error handling.
        if (execv(pathOfBinary.c_str(), (*inputCommand).argv) < 0) {
            cerr << errorMessage << endl;
            exit(0);
        }
        exit(0);
    }
}
//This function reads in manual.txt
void help() {
    static char extraHelp[] = "more";
    static char manual[] = "readme";

    Command command;
    //Here I set the indivisual arguments for the argv when help is called
    command.argv[0] = extraHelp;
    //Here is the actual manual arg
    command.argv[1] = manual;
    //Terminating with NULL
    command.argv[2] = nullptr;

    //Here I reset the STDIN/OUT and then execute the command.
    command.redirectSTDIN = command.redirectSTDOUT = nullptr;
    executeExternalCommand(&command);
}
int main(int argc, char * argv[]) {

    string commandLine,inFileName, bashShell = "myshell> ";
    //by default the shell should be in normak, interactive mode.
    bool isSTDIN = false;

    //After finding emplace_back function on GNU library, I use it to attach to the end of the
    //environmentPaths vector, especially since it allocates/deallocates automatically.
    environmentPaths.emplace_back("/bin/");
    environmentPaths.emplace_back("/usr/bin/");
    // identify if myshell is invoked in batch mode or interative mode
    // by counting the number of argument passed
    //In order to determine if the user uses the batch file or not, I determine the number
    //of arguments passed to main.
    if (argc == 1) {
        //Is in normal shell mode
        isSTDIN = true;
    } else if (argc == 2) {
        //Is in batch mode
        inFileName = argv[1];
    } else {
        //Error handling
        cerr << errorMessage << endl;
    }

    ifstream fileIn;
    //Here I make sure that the file for batch is not empty and opens
    if (!inFileName.empty()) {
        //Open the file passed to read
        fileIn.open(inFileName);
        //Error handling in case the file does not open
        if (!fileIn.is_open()) {
            cerr << errorMessage << endl;
            return 1;
        }
    }
    //Here I check if the arguments for main make it either the
    //input stream/normal mode, or batch mode.
    istream &input = isSTDIN ? cin : fileIn;

    //If it's in normal mode, keep printing the prompt.
    if (isSTDIN) {
        cout << bashShell;
    }
    //Read the input a line at a time
    while(getline(input, commandLine)) {
        //Here I use this if statement to filter out empty lines
        if (!commandLine.empty()) {
            //Once here, I parse the command and utilize the struct Command.
            Command *inputCommand = parseInputCommand(commandLine.c_str()); // parse the command and and get Command struct filled
            //Finally, I check the first argument passed in a line, and make use of my enum list to pass
            //and return what kind of command the user has typed in.
            CommandType commandValue = getCommandType((*inputCommand).argv[0]);

            if (commandValue == CD){
                CDCommand(inputCommand);
            } else if (commandValue == CLEAR) {
                clearCommand();
            } else if (commandValue == DIR ){
                showDirectory(inputCommand);
            } else if (commandValue == ENVIRON){
                printEnvironment(inputCommand);
            } else if (commandValue == ECHO){
                echo(inputCommand);
            } else if (commandValue == HELP){
                help();
            } else if (commandValue == PAUSE){
                cin.get();
            } else if (commandValue == EXTERNAL){
                executeExternalCommand(inputCommand);
            } else if (commandValue == QUIT){
                exit(0);
            }

            //final step is to deallocated used up space.
            deallocateInputCommand(inputCommand);
        }
        //Keep printing the prompt
        if (isSTDIN) {
            cout << bashShell;
        }
    }
    return 0;
}

