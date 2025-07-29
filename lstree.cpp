#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string>
#include <iostream>
#include <vector>
#include <cstring>
#include "header.h"
using namespace std;


string get_object_content(const string& sha) {
    string dir_path = ".mygit/objects/" + sha.substr(0, 2);
    string file_path = dir_path + "/" + sha.substr(2);

    int fd = open(file_path.c_str(), O_RDONLY);
    if (fd < 0) {
        perror(("Error opening object file: " + file_path).c_str());
        return "";
    }

    // Read file content in chunks
    char buffer[4096];
    string content;
    ssize_t bytes_read;
    while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
        content.append(buffer, bytes_read);
    }
    close(fd);

    return content;
}

// function to parse and display a tree object content recursively
void display_tree_content(const string& tree_content, bool name_only, const string& prefix ) {
    size_t pos = 0;
    while (pos < tree_content.size()) {
        // read mode 
        size_t space_pos = tree_content.find(' ', pos);
        string mode = tree_content.substr(pos, space_pos - pos);
        pos = space_pos + 1;

        // read type (blob or tree)
        space_pos = tree_content.find(' ', pos);
        string type = tree_content.substr(pos, space_pos - pos);
        pos = space_pos + 1;

        // read SHA
        space_pos = tree_content.find(' ', pos);
        string sha = tree_content.substr(pos, space_pos - pos);
        pos = space_pos + 1;

        // read name
        size_t newline_pos = tree_content.find('\n', pos);
        string name = tree_content.substr(pos, newline_pos - pos);
        pos = newline_pos + 1;

        string full_name = prefix.empty() ? name : prefix + "/" + name;

        if (type == "tree") {
            if (name_only) {
                cout << full_name << endl;
            } 
            else {
                cout << mode << " " << type << " " << sha << " " << full_name << endl;
            }

            // recursively retrieve and display the content of the subdirectory (tree)
            string sub_tree_content = get_object_content(sha);
            if (!sub_tree_content.empty()) {
                display_tree_content(sub_tree_content, name_only, full_name);
            }
        } 
        else if (type == "blob") {
            if (name_only) {
                cout << full_name << endl;
            } 
            else {
                cout << mode << " " << type << " " << sha << " " << full_name << endl;
            }
        }
    }
}


void ls_tree(const string& tree_sha, bool name_only) {
    string tree_content = get_object_content(tree_sha);
    if (tree_content.empty()) {
        cerr << "Error: Tree object not found for SHA " << tree_sha << endl;
        return;
    }

  
    display_tree_content(tree_content, name_only,"");
}


