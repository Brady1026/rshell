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
    struct stat *statp = new struct stat;
    int readdirerror = errno;
    while ((direntp = readdir(dirp)))
    {
        if (stat(direntp->d_name, statp) == -1)
        {
            perror("stat");
            exit(EXIT_FAILURE);
        }
        cout << statp->st_mode << '\t';
        cout << statp->st_nlink << '\t';
        cout << statp->st_size << '\t';
        cout << statp->st_mtime << '\t';
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
    delete statp;
    return 0;
}
