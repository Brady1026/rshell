#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <csignal>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <vector>

using namespace std;

const char operators[] = "><|;";

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
    enum {none, fileread, filenew, fileappend, pipe, connect, connectand, connector} tokentype = none;
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
        else if(tokit->compare(";") == 0)
        {
            tokentype = connect;
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
            else if(tokentype == connect)
            {
                commands->push_back(temp);
                temp = empty;
                tokentype = none;
                continue;
            }
            else if(tokentype == none)
            {
                temp.arguments.push_back(*tokit);
            }
            tokentype = none;
        }
        ++tokit;
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
        ++tokit;
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
        ++commandno;
    }
    vector<int> pids;
    commandno = 0;
    while(commandno < commands->size())
    {
        int pid = 0;
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
            const char **args = new const char*[size];
            unsigned argindex = 0;
            for(unsigned j = 0; j < commands->at(commandno).arguments.size(); j++)
            {
                args[argindex] = commands->at(commandno).arguments.at(j).c_str();
                ++argindex;
            }
            args[argindex] = NULL;
            if(args[0] == NULL)
            {
                exit(EXIT_SUCCESS);
            }
            if(commands->at(commandno).infiles.size() > 0)
            {
                int fdi;
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
                int fdo;
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
            if(commands->at(commandno).arguments.at(0).compare("cd") == 0)
            {
                if(commands->at(commandno).arguments.size() == 1)
                {
                    if(chdir(getenv("HOME")) == -1)
                    {
                        perror("chdir()");
                        exit(EXIT_FAILURE);
                    }
                }
                else if(chdir(commands->at(commandno).arguments.at(1).c_str()) == -1)
                {
                    perror("chdir()");
                    exit(EXIT_FAILURE);
                }
                char buf[BUFSIZ];
                if(getcwd(buf, sizeof(buf)) == NULL)
                {
                    perror("getcwd()");
                    exit(EXIT_FAILURE);
                }
                if(setenv("PWD", buf, 1) == -1)
                {
                    perror("setenv()");
                    exit(EXIT_FAILURE);
                }
                exit(EXIT_SUCCESS);
            }
            if(commands->at(commandno).arguments.at(0).compare("exit") == 0)
            {
                exit(EXIT_SUCCESS);
            }
            string path = getenv("PATH");
            char *cpath = new char[path.size() + 1];
            strcpy(cpath, path.c_str());
            char delimit[] = ":";
            char *temp = strtok(cpath, delimit);
            char *directory = new char[path.size() + 1];
            strcat(directory, commands->at(commandno).arguments.at(0).c_str());
            args[0] = directory;
            if(execv(args[0], (char **)args) == -1)
            {
                if(errno != ENOENT)
                {
                    perror("execv()");
                    exit(EXIT_FAILURE);
                }
            }
            while(temp != NULL)
            {
                strcpy(directory, temp);
                if(directory[strlen(directory)] != '/')
                {
                    strcat(directory, "/");
                }
                strcat(directory, commands->at(commandno).arguments.at(0).c_str());
                args[0] = directory;
                if(execv(args[0], (char **)args) == -1)
                {
                    if(errno != ENOENT)
                    {
                        perror("execv()");
                        exit(EXIT_FAILURE);
                    }
                }
                temp = strtok(NULL, delimit);
            }
            perror("execv()");
            exit(EXIT_FAILURE);
            delete args;
            delete[] pipefd;
        }
        else
        {
            pids.push_back(pid);
        }
        ++commandno;
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
    delete[] pipefd;
    exit(EXIT_SUCCESS);
}

void execute(vector<command> *commands, int size)
{
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
    for(unsigned i = 0; i < commands->size(); i++)
    {
        if(commands->at(i).arguments.size() == 0)
        {
            return;
        }
        if(commands->at(i).arguments.at(0).compare("cd") == 0)
        {
            if(commands->at(i).arguments.size() == 1)
            {
                if(chdir(getenv("HOME")) == -1)
                {
                    perror("chdir()");
                    exit(EXIT_FAILURE);
                }
            }
            else if(chdir(commands->at(i).arguments.at(1).c_str()) == -1)
            {
                perror("chdir()");
                exit(EXIT_FAILURE);
            }
            char buf[BUFSIZ];
            if(getcwd(buf, sizeof(buf)) == NULL)
            {
                perror("getcwd()");
                exit(EXIT_FAILURE);
            }
            if(setenv("PWD", buf, 1) == -1)
            {
                perror("setenv()");
                exit(EXIT_FAILURE);
            }
            return;
        }
        if(commands->at(i).arguments.at(0).compare("exit") == 0)
        {
            exit(EXIT_SUCCESS);
        }
    }
    return;
}

void shell()
{
    char host[BUFSIZ];
    if(gethostname(host, BUFSIZ) == -1)
    {
        perror("gethostname()");
        exit(EXIT_FAILURE);
    }
    string input;
    cout << getenv("PWD") << endl;
    cout << getenv("LOGNAME") << '@' << host << " $ ";
    getline(cin, input);
    char *cinput = new char[input.size() + 1];
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
    delete[] cinput;
    return;
}

void sign_handl(int signum)
{
    return;
}

int main()
{
    signal(SIGINT, sign_handl);
    while(true)
        shell();
}
