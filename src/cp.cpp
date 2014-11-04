#include <iostream>
#include <cstdlib>
#include <fstream>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include "Timer.h"

using namespace std;

int methodone(char *argv[])
{
    ifstream input;
    ofstream output;
    int f = open(argv[2], O_WRONLY | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
    if (f < 0)
    {
        perror("Desired output file already exists");
        exit(1);
    }
    if (close(f) == -1)
    {
        perror("Close");
        exit(1);
    }
    f = open(argv[1], O_RDONLY);
    if (f == -1 )
    {
        perror("Desired input file cannot be opened");
        exit(1);
    }
    if (close(f) == -1)
    {
        perror("Close");
        exit(1);
    }
    input.open(argv[1]);
    if (input.is_open())
    {
        output.open(argv[2]);
        if (output.is_open())
        {
            while (input.good())
            {
                char c = input.get();
                if (input.good())
                    output.put(c);
            }
            output.close();
            input.close();
        }
    }
    else
    {
        cerr << "Desired input file cannot be opened!" << endl;
        exit(1);
    }
    return 0;
}

int methodtwo(char *argv[])
{
    char buffer[1];
    int temp;
    int temp2;
    int input = open(argv[1], O_RDONLY);
    if (input == -1)
    {
        perror("Desired input file cannot be opened");
        exit(1);
    }
    else
    {
        temp = read(input, buffer, 1);
        if (temp == -1)
        {
            perror("read()");
            exit(1);
        }
        int output = open(argv[2], O_WRONLY | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
        if (output == -1)
        {
            perror("Desired output file already exists");
            exit(1);
        }
        while (temp > 0)
        {
            if (temp == -1)
            {
                perror("read()");
                exit(1);
            }
            temp2 = write(output, buffer, temp);
            if (temp2 == -1)
            {
                perror("write()");
                exit(1);
            }
            temp = read(input, buffer, 1);
        }
        if (temp == -1)
        {
            perror("read()");
            exit(1);
        }
        if (close(output) == -1)
        {
            perror("close");
            exit(1);
        }
    }
    if (close(input) == -1)
    {
        perror("Close");
        exit(1);
    }
    return 0;
}

int methodthree(char *argv[])
{
    char buffer[BUFSIZ];
    int temp;
    int temp2;
    int input = open(argv[1], O_RDONLY);
    if (input == -1)
    {
        perror("Desired input file cannot be opened");
        exit(1);
    }
    else
    {
        temp = read(input, buffer, BUFSIZ);
        if (temp == -1)
        {
            perror("read()");
            exit(1);
        }
        int output = open(argv[2], O_WRONLY | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
        if (output == -1)
        {
            perror("Desired output file already exists");
            exit(1);
        }
        while (temp > 0)
        {
            if (temp == -1)
            {
                perror("read()");
                exit(1);
            }
            temp2 = write(output, buffer, temp);
            if (temp2 == -1)
            {
                perror("write()");
                exit(1);
            }
            temp = read(input, buffer, BUFSIZ);
        }
        if (temp == -1)
        {
            perror("read()");
            exit(1);
        }
        if (close(output) == -1)
        {
            perror("close");
            exit(1);
        }
    }
    if (close(input) == -1)
    {
        perror("Close");
        exit(1);
    }
    return 0;
}

int main(int argc, char *argv[])
{
    if (argc < 3 || argc > 4)
    {
        cerr << "incorrect arguments" << endl;
        return 1;
    }
    if (argc == 3)
    {
        methodthree(argv);
        return 0;
    }
    Timer t;
    double wTime, uTime, sTime;
    if (argc == 4)
    {
        t.start();
        methodone(argv);
        t.elapsedTime(wTime, uTime, sTime);
        cout << "Method one: " << endl;
        cout << "Elapsed wallclock time: " << wTime << endl;
        cout << "Elapsed user time: " << uTime << endl;
        cout << "Elapsed system time: " << sTime << endl;
        if (remove(argv[2]) != 0)
        {
            perror("remove");
            exit(1);
        }
        t.start();
        methodtwo(argv);
        t.elapsedTime(wTime, uTime, sTime);
        cout << "Method two: " << endl;
        cout << "Elapsed wallclock time: " << wTime << endl;
        cout << "Elapsed user time: " << uTime << endl;
        cout << "Elapsed system time: " << sTime << endl;
        if (remove(argv[2]) != 0)
        {
            perror("remove");
            exit(1);
        }
        t.start();
        methodthree(argv);
        t.elapsedTime(wTime, uTime, sTime);
        cout << "Method three: " << endl;
        cout << "Elapsed wallclock time: " << wTime << endl;
        cout << "Elapsed user time: " << uTime << endl;
        cout << "Elapsed system time: " << sTime << endl;
    }
    return 0;
}
