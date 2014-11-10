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
    char *test[4];
    cout << "$ ";
    getline(cin, input);
    char cinput[40];
    char delimit[] = " ";
    strcpy(cinput, input.c_str());
    strtok(cinput, "#");
    test[0] = strtok(cinput, delimit);
    test[1] = strtok(NULL, delimit);
    test[2] = strtok(NULL, delimit);
    test[3] = '\0';
    int returnval = execvp(cinput, test);
    if (returnval == -1) 
    {
        perror("execvp()");
        return 1;
    }
    return 0;
}
