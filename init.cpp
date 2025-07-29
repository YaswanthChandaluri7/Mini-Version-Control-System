#include <iostream>
#include <sys/stat.h>  
#include <fcntl.h>     
#include <unistd.h>    
#include <cstring>     
#include <cerrno>  
#include "header.h"    
using namespace std;

bool directoryexists(const string &dirName) {
    struct stat info;
    if (stat(dirName.c_str(), &info) != 0) {
        return false;
    } 
    else if (info.st_mode & S_IFDIR) {
        return true;
    }
    return false;
}


bool createdirectory(const string &dirName) {
 
    if (mkdir(dirName.c_str(), 0755) != 0) {
        if (errno == EEXIST) {
            cout << "Error: " << dirName << " directory already exists." << endl;
        }
         else {
            cerr << "Error: Unable to create directory " << dirName << " - " << strerror(errno) << endl;
        }
        return false;
    }
    return true;
}


void initializeHEAD(const string &filePath) {
    int fd = open(filePath.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0644);  
    if (fd == -1) {
        cerr << "Error: Unable to create HEAD file - " << strerror(errno) << endl;
        return;
    }
    
    const char* headContent = "ref: refs/heads/master\n";  
    write(fd, headContent, strlen(headContent));  
    close(fd); 
}


void initializeconfig(const string &filePath) {
    int fd = open(filePath.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0644);  
    if (fd == -1) {
        cerr << "Error: Unable to create config file - " << strerror(errno) << endl;
        return;
    }
    
    const char* configContent = "[core]\n"
                                "\trepositoryformatversion = 0\n"
                                "\tfilemode = true\n"
                                "\tbare = false\n";
    write(fd, configContent, strlen(configContent));  
    close(fd);  
}

void initializeIndex(const string &filePath) {
    int fd = open(filePath.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0644);  

    if (fd == -1) {
        cerr << "Error: Unable to create index file - " << strerror(errno) << endl;
        return;
    }
    close(fd); 
}


void repository() {
    
    string repoDir = ".mygit";

    // check if the repository already exists
    if (directoryexists(repoDir)) {
        cout << "Error: Repository already initialized." << endl;
        return;
    }

    // create the .mygit directory
    if (!createdirectory(repoDir)) {
        return;  
    }

    // create necessary subdirectories: objects, refs, and refs/heads
     if (createdirectory(repoDir + "/objects") &&createdirectory(repoDir + "/refs") &&createdirectory(repoDir + "/refs/heads")) {

        // Initialize the HEAD file
        initializeHEAD(repoDir + "/HEAD");

        // Initialize the config file
        initializeconfig(repoDir + "/config");

        // Initialize the index file
        initializeIndex(repoDir + "/index");

        std::cout << "Initialized empty MyGit repository in " << repoDir << std::endl;
    }
}


