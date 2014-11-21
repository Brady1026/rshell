#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <vector>

using namespace std;

const char operators[] = "><|";

struct command
{
    vector<string> arguments, infiles, outfiles;
    vector<int> outtype;
    bool pipeout;
    bool pipein;
};

void commandgen(vector<string> *tokens, vector<command> *commands)
{
    command temp;
    temp.pipein = false;
    temp.pipeout = false;
    command empty = temp;
    vector<string>::iterator tokit = tokens->begin();
    enum {none, fileread, filenew, fileappend, pipe} tokentype = none;
    bool inpipe = false;

    while(tokit < tokens->end())
    {
        if(inpipe == true)
        {
            temp = empty;
            temp.pipein = true;
            inpipe = false;
        }
        if(tokit->compare(">") == 0)
        {
            tokentype = filenew;
        }
        else if(tokit->compare(">>") == 0)
        {
            tokentype = fileappend;
        }
        else if(tokit->compare("<") == 0)
        {
            tokentype = fileread;
        }
        else if(tokit->compare("|") == 0)
        {
            tokentype = pipe;
        }
        else
        {
            if(tokentype == fileread)
                temp.infiles.push_back(*tokit);
            else if(tokentype == filenew)
            {
                temp.outfiles.push_back(*tokit);
                temp.outtype.push_back(0);
            }
            else if(tokentype == fileappend)
            {
                temp.outfiles.push_back(*tokit);
                temp.outtype.push_back(1);
            }
            else if(tokentype == pipe)
            {
                temp.pipeout = true;
                inpipe = true;
                commands->push_back(temp);
                tokentype = none;
                continue;
            }
            else if(tokentype == none)
            {
                temp.arguments.push_back(*tokit);
            }
            tokentype = none;
        }
        tokit++;
    }
    commands->push_back(temp);
    return;
}

void parsetokens(vector<string> *tokens)
{
    string temp;
    vector<string>::iterator tokit = tokens->begin();
    while(tokit < tokens->end())
    {
        for(unsigned i = 0; i < tokit->length(); i++)
        {
            for(unsigned j = 0; operators[j] != '\0'; j++)
            {
                if(tokit->at(i) == operators[j])
                {
                    if(i != 0)
                    {
                        if(tokit->at(i - 1) != operators[j])
                        {
                            temp = tokit->substr(0, i);
                            tokit->erase(0,i);
                            tokit = tokens->insert(tokit, temp);
                            i = 0;
                        }
                    }
                    else if(i == 0)
                    {
                        if(i + 1 < tokit->length())
                        {
                            if(tokit->at(i + 1) != operators[j])
                            {
                                temp = tokit->substr(0, i + 1);
                                tokit->erase(0, i + 1);
                                tokit = tokens->insert(tokit, temp);
                                i = 0;
                            }
                            else if(i + 2 < tokit->length())
                            {
                                temp = tokit->substr(0, i + 2);
                                tokit->erase(0, i + 2);
                                tokit = tokens->insert(tokit, temp);
                                i = 0;
                            }
                        }
                    }

                }
            }
        }
        tokit++;
    }
}

void shell()
{
    string input;
    cout << "$ ";
    getline(cin, input);
    char *cinput = new char[input.size()];
    char delimit[] = " ";
    strcpy(cinput, input.c_str());
    strtok(cinput, "#");
    const char **args = new const char*[input.size()];
    char *temp = strtok(cinput, delimit);
    vector<string> tokens;
    for(unsigned i = 0; temp != NULL; i++)
    {
        tokens.push_back(temp);
        temp = strtok(NULL, delimit);
    }
    parsetokens(&tokens);
    vector<command> commands;
    commandgen(&tokens, &commands);
    unsigned argindex = 0;
    for(unsigned i = 0; i < commands.size(); i++)
    {
        for(unsigned j = 0; j < commands.at(i).arguments.size(); j++)
        {
            args[argindex] = commands.at(i).arguments.at(j).c_str();
            argindex++;
        }
    }
    if(strcmp(args[0], "exit") == 0)
        exit(EXIT_SUCCESS);
    int pid = fork();
    int returnval = 0;
    switch (pid)
    {
        case 0: returnval = execvp(args[0], (char **)args);
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
