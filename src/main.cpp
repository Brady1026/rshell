#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

using namespace std;

void shell()
{
    string input;
    cout << "$ ";
    getline(cin, input);
    char *cinput = new char[input.size()];
    char delimit[] = " ";
    strcpy(cinput, input.c_str());
    strtok(cinput, "#");
    char **args = new char*[input.size()];
    char *temp = strtok(cinput, delimit);
    unsigned i = 0;
    args[i] = temp;
    for(i = 0; temp != NULL; i++)
    {
        args[i] = temp;
        temp = strtok(NULL, delimit);
    }
    args[i] = temp;
    if(strcmp(args[0], "exit") == 0)
        exit(EXIT_SUCCESS);
    int pid = fork();
    int returnval = 0;
    switch (pid)
    {
        case 0: returnval = execvp(args[0], args);
                break;
        case -1: perror("fork()");
                 exit(EXIT_FAILURE);
                 break;
        default: if(waitpid(-1, NULL, 0) == -1)
                 {
                     perror("waitpid()");
                     exit(EXIT_FAILURE);
                 }
                 break;
    }
    delete cinput;
    delete args;
    if (returnval == -1) 
    {
        perror("execvp()");
        exit(EXIT_FAILURE);
    }
    return;
}

int main()
{
    while(true)
        shell();
}
