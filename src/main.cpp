#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

using namespace std;

int main()
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
    for(unsigned i = 0; temp != NULL; i++)
    {
        args[i] = temp;
        temp = strtok(NULL, delimit);
    }
    int returnval = execvp(args[0], args);
    if (returnval == -1) 
    {
        perror("execvp()");
        return 1;
    }
    return 0;
}
