#include <fcntl.h>      
#include <unistd.h>     
#include <sys/stat.h>   
#include <zlib.h>       
#include <openssl/sha.h> 
#include <string>
#include <vector>
#include <iostream>
#include <cstdio>
#include <sys/types.h>
#include <dirent.h> 
#include "header.h"
using namespace std;


bool ishidden(const string& name) {
    return !name.empty() && name[0] == '.';
}

string relativepath(const string& base_path, const string& full_path) {
    if (full_path.find(base_path) == 0) { 
        string relative_path = full_path.substr(base_path.size());
        if (!relative_path.empty() && relative_path[0] == '/') {
            relative_path = relative_path.substr(1);  
        }
        return relative_path;
    } else {
        // cerr << "Error: base_path is not a prefix of full_path" << endl;
        return full_path; 
    }
}


// entry function to add files or directories to the staging area
void addtostagingarea(const vector<string>& paths) {
    int index_fd = open(".mygit/index", O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (index_fd < 0) {
        perror("Error opening index file");
        return;
    }

    for (const string& path : paths) {
        if (is_directory(path)) {
            directorytoindex(path,index_fd, "");
        } else {
            filetoindex(path, index_fd, "");
        }
    }

    close(index_fd);
}


bool is_directory(const string& path) {
    struct stat statbuf;
    if (stat(path.c_str(), &statbuf) != 0) return false;
    return S_ISDIR(statbuf.st_mode);
}

// function to get file mode as a string (e.g., "100644" for regular file)
string get_file_mode(const string& file_path) {
    struct stat st;
    stat(file_path.c_str(), &st);
    return (st.st_mode & S_IXUSR) ? "100755" : "100644";
}


void filetoindex(const string& base_path,int index_fd, const string& file_path) {

    string relative_file_path = relativepath(base_path, file_path);

    // string file_sha = calculate_compressed_sha(file_path);

    string filecontent=readfilecontent(file_path);

    string file_sha=sha1(filecontent);

    writeblobobject(filecontent, file_sha);

    string file_mode = get_file_mode(file_path);
    string index_entry = file_mode + " "+"blob" + " " + file_sha + " " + relative_file_path + "\n";

    cout<<index_entry<<endl;

    write(index_fd, index_entry.c_str(), index_entry.size());
}

// Recursive function to add all files in a directory to the index
void directorytoindex(const string& base_path,int index_fd, const string& dir_path) {
    DIR* dir = opendir(dir_path.c_str());
    if (!dir) {
        perror(("Error opening directory: " + dir_path).c_str());
        return;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        string name = entry->d_name;
        
        
        if (ishidden(name) || name == "." || name == "..") continue;

        string full_path = dir_path + "/" + name;
        if (entry->d_type == DT_DIR) {
            // recursively add subdirectory
            directorytoindex(base_path,index_fd, full_path);
        }
         else if (entry->d_type == DT_REG) {
            filetoindex(base_path,index_fd, full_path);
        }
    }
    closedir(dir);
}