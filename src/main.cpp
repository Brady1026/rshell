#include <iostream>
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
    char *test[2];
    cout << "$ ";
    cin >> input;
    //cout << "You typed: " << input << endl;
    char cinput[40];
    strcpy(cinput, input.c_str());
    test[0] = cinput;
    test[1] = NULL;
    int returnval = execvp(cinput, test);
    if (returnval == -1) 
    {
        cerr << "Error " << errno << " calling execvp" << endl;
        return 1;
    }
    return 0;
}
