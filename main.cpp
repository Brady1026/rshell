#include <iostream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

using namespace std;

int main()
{
    string input;
    char *test[2];
    cout << "$ ";
    cin >> input;
    //cout << "You typed: " << input << endl;
    char cinput[input.length()];
    strcpy(cinput, input.c_str());
    test[0] = cinput;
    test[1] = NULL;
    execvp(cinput, test);
    return 0;
}
