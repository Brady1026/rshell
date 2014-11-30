#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
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

void exec_children(vector<command> *commands, int size)
{
    unsigned commandno = 0;
    int *pipefd = new int[size];
    vector<int> pipefdstoclose;
    while(commandno < commands->size())
    {
        if(commands->at(commandno).pipeout == true)
        {
            if(pipe(pipefd + (2 * commandno)) == -1)
            {
                perror("pipe()");
                exit(EXIT_FAILURE);
            }
            pipefdstoclose.push_back(pipefd[2 * commandno]);
            pipefdstoclose.push_back(pipefd[(2 * commandno) + 1]);
        }
        commandno++;
    }
    vector<int> pids;
    commandno = 0;
    int pid = 0;
    while(commandno < commands->size())
    {
        pid = fork();
        if(pid == -1)
        {
            perror("fork()");
            exit(EXIT_FAILURE);
        }
        else if(pid == 0)
        {
            if(commands->at(commandno).pipeout == true)
            {
                if(dup2(pipefd[(2 * commandno) + 1], 1) == -1)
                {
                    perror("dup2()");
                    exit(EXIT_FAILURE);
                }
            }
            if(commands->at(commandno).pipein == true)
            {
                if(dup2(pipefd[2 * (commandno - 1)], 0) == -1)
                {
                    perror("dup2()");
                    exit(EXIT_FAILURE);
                }
            }
            for(unsigned i = 0; i < pipefdstoclose.size(); i++)
            {
                if(close(pipefdstoclose.at(i)) == -1)
                {
                    perror("close()");
                    exit(EXIT_FAILURE);
                }
            }
            int fdi, fdo;
            const char **args = new const char*[size];
            unsigned argindex = 0;
            for(unsigned j = 0; j < commands->at(commandno).arguments.size(); j++)
            {
                args[argindex] = commands->at(commandno).arguments.at(j).c_str();
                argindex++;
            }
            args[argindex] = NULL;
            if(commands->at(commandno).infiles.size() > 0)
            {
                for(unsigned i = 0; i < commands->at(commandno).infiles.size(); i++)
                {
                    fdi = open(commands->at(commandno).infiles.at(i).c_str(), O_RDONLY);
                    if(fdi == -1)
                    {
                        perror("open()");
                        exit(EXIT_FAILURE);
                    }
                }
                if(dup2(fdi, 0) == -1)
                {
                    perror("dup2()");
                    exit(EXIT_FAILURE);
                }
            }
            if(commands->at(commandno).outfiles.size() > 0)
            {
                for(unsigned i = 0; i < commands->at(commandno).outfiles.size(); i++)
                {
                    if(commands->at(commandno).outtype.at(i) == 0)
                    {
                        fdo = open(commands->at(commandno).outfiles.at(i).c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
                        if(fdo == -1)
                        {
                            perror("open()");
                            exit(EXIT_FAILURE);
                        }
                    }
                    else if(commands->at(commandno).outtype.at(i) == 1)
                    {
                        fdo = open(commands->at(commandno).outfiles.at(i).c_str(), O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
                        if(fdo == -1)
                        {
                            perror("open()");
                            exit(EXIT_FAILURE);
                        }
                    }
                }
                if(dup2(fdo, 1) == -1)
                {
                    perror("dup2()");
                    exit(EXIT_FAILURE);
                }
            }
            int returnvalue = execvp(args[0], (char **)args);
            if (returnvalue == -1)
            {
                perror("execvp()");
                exit(EXIT_FAILURE);
            }
            delete args;
            delete[] pipefd;
        }
        else
        {
            pids.push_back(pid);
        }
        commandno++;
    }
    for(unsigned i = 0; i < pipefdstoclose.size(); i++)
    {
        if(close(pipefdstoclose.at(i)) == -1)
        {
            perror("close()");
            exit(EXIT_FAILURE);
        }
    }
    for(unsigned i = 0; i < pids.size(); i++)
    {
        if(waitpid(pids.at(i), NULL, 0) == -1)
        {
            perror("waitpid()");
            exit(EXIT_FAILURE);
        }
    }
    exit(EXIT_SUCCESS);
}

void execute(vector<command> *commands, int size)
{
    const char **args = new const char*[size];
    unsigned argindex = 0;
    for(unsigned i = 0; i < commands->size(); i++)
    {
        for(unsigned j = 0; j < commands->at(i).arguments.size(); j++)
        {
            args[argindex] = commands->at(i).arguments.at(j).c_str();
            argindex++;
        }
    }
    if(strcmp(args[0], "exit") == 0)
        exit(EXIT_SUCCESS);
    int pid = fork();
    switch (pid)
    {
        case 0: exec_children(commands, size);
                break;
        case -1: perror("fork()");
                 exit(EXIT_FAILURE);
                 break;
        default: if(waitpid(pid, NULL, 0) == -1)
                 {
                     perror("waitpid()");
                     exit(EXIT_FAILURE);
                 }
                 break;
    }
    delete args;
    return;
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
    execute(&commands, input.size());
    return;
}

int main()
{
    while(true)
        shell();
}
