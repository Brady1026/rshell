#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <cerrno>

using namespace std;

int main(int argc, char *argv[])
{
    char defaultdir[] = ".";
    char *dirName = defaultdir;
    if (argc == 2)
        dirName = argv[1];
    DIR *dirp;
    if (!(dirp = opendir(dirName)))
    {
        perror("opendir");
        exit(EXIT_FAILURE);
    }
    dirent *direntp;
    int readdirerror = errno;
    while ((direntp = readdir(dirp)))
    {
        cout << direntp->d_name << endl;
    }
    if (readdirerror != errno)
    {
        perror("readdir");
        exit(EXIT_FAILURE);
    }
    if (closedir(dirp) == -1)
    {
        perror("closedir");
        exit(EXIT_FAILURE);
    }
    return 0;
}
