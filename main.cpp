#include <iostream>
#include <sys/stat.h>  
#include <fcntl.h>     
#include <unistd.h>    
#include <cstring>    
#include <cerrno> 
#include "header.h"
using namespace std;

int main(int argc, char* argv[]) {
    
   
    if (argc < 2) {
        cout << "Usage: ./mygit <command> [options]" << endl;
        return 1;
    }

    string command = argv[1];

    if (command == "init") {
        repository();  
    } 
    else if (command == "hash-object") {
        bool writeblob = false;

        
        if (argc > 2 && string(argv[2]) == "-w") {
            writeblob = true;
        }

        // Ensure that the file path argument is provided
        if ((writeblob && argc < 4) || (!writeblob && argc < 3)) {
            cout << "Usage: ./mygit hash-object [-w] <file>" << endl;
            return 1;
        }

        string filePath = writeblob ? argv[3] : argv[2];

        hashobject(filePath, writeblob);  
    } 
     else if (command == "cat-file") {
        if (argc < 4) {
            cerr << "Usage: ./mygit cat-file <flag> <file_sha>" << endl;
            return 1;
        }

        string flag = argv[2];
        string fileSha = argv[3];

        if (flag == "-p" || flag == "-s" || flag == "-t") {
            catFile(flag, fileSha);
        } 
        else {
            cerr << "Error: Unknown flag " << flag << endl;
            return 1;
        }
    }
    else if (command == "add") {
        
        if (argc < 3) {
            cerr << "Error: No files or directories specified for `add`." << endl;
            return 1;
        }

        int index_fd = open(".mygit/index", O_WRONLY | O_CREAT | O_APPEND, 0644);
        if (index_fd < 0) {
            perror("Error opening index file");
            return 1;
        }

        // Collect files and directories to add
        for (int i = 2; i < argc; ++i) {
            string path = argv[i];
            if (is_directory(path)) {
                directorytoindex(path,index_fd,path); 
            } 
            else {
                filetoindex(".", index_fd, path); 
            }
        }

        close(index_fd);
    }
    else if(command == "write-tree"){
        if (argc < 2) {
            cerr << "Usage: ./mygit <command>" << std::endl;
            return 1;
        }
        write_tree();
    }
    else if(command=="ls-tree"){

        if (argc < 3) {
            cerr << "Usage: ./mygit ls-tree [--name-only] <tree_sha>" << endl;
            return 1;
        }

        bool name_only = false;
        string tree_sha;

        
        if (strcmp(argv[2], "--name-only") == 0) {
            name_only = true;
            tree_sha = argv[3];
        } 
        else {
            tree_sha = argv[2];
        }

        
        ls_tree(tree_sha, name_only);
    }
    else if(command=="commit"){
        string message = (argc > 2 && string(argv[2]) == "-m") ? argv[3] : "commit";
        commit(message);
    }
    else if (command == "log") {
        log(); 
    }
    else if(command=="checkout"){
        string checksha=argv[2];
        checkout(checksha);
    }
     
    else {
        cout << "Unknown command: " << command << endl;
        cout << "Usage: ./mygit <init|hash-object>" << endl;
        return 1;
    }

    return 0;
}