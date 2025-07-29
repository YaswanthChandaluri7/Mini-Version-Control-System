#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string>
#include <vector>
#include <cstring>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <openssl/sha.h>
#include <ctime>
#include <map>
#include "header.h"

using namespace std;


// Function to write a file using system calls
void write_file1(const string& path, const string& content) {
    int fd = open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        perror(("Error creating file: " + path).c_str());
        return;
    }
    write(fd, content.c_str(), content.size());
    close(fd);
}

// Function to create a tree object
string create_tree_object(const vector<IndexEntry>& entries) {
    string tree_content;
    for (const auto& entry : entries) {
        tree_content += entry.mode + " " + entry.type + " " + entry.sha + " " + entry.path + "\n";
    }
    string tree_sha = sha1(tree_content);
    
    // Write the tree object to the file
    string tree_dir = ".mygit/objects/" + tree_sha.substr(0, 2);
    string tree_file = tree_dir + "/" + tree_sha.substr(2);
    mkdir(tree_dir.c_str(), 0755);
    write_file(tree_file, tree_content);
    
    return tree_sha;
}

// Parse the index file with error handling
vector<IndexEntry> parseIndexFile() {  
    vector<IndexEntry> entries;
    int fd = open(".mygit/index", O_RDONLY);
    if (fd < 0) {
        perror("Error opening index file");
        return entries;
    }

    char buffer[4096];
    ssize_t bytes_read;
    string index_content;

    // Read the entire file content
    while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
        index_content.append(buffer, bytes_read);
    }
    close(fd);

    if (bytes_read < 0) {
        perror("Error reading index file");
        return entries;
    }

    // Manual parsing without stringstream
    size_t start = 0, end;
    while (start < index_content.size()) {
        IndexEntry entry;

        // Parse mode
        end = index_content.find(' ', start);
        if (end == string::npos) break;
        try {
            entry.mode = (index_content.substr(start, end - start));
        } catch (invalid_argument& e) {
            cerr << "Error parsing mode: " << e.what() << endl;
            break;
        }
        start = end + 1;

        // Parse type
        end = index_content.find(' ', start);
        if (end == string::npos) break;
        try {
            entry.type = (index_content.substr(start, end - start));
        } catch (invalid_argument& e) {
            cerr << "Error parsing type: " << e.what() << endl;
            break;
        }
        start = end + 1;

        // Parse sha
        end = index_content.find(' ', start);
        if (end == string::npos) break;
        entry.sha = index_content.substr(start, end - start);
        start = end + 1;

        // Parse path
        end = index_content.find('\n', start);
        if (end == string::npos) break;
        entry.path = index_content.substr(start, end - start);
        start = end + 1;

        entries.push_back(entry);
    }

    return entries;
}

void commit(const string& message) {
    vector<IndexEntry> index_entries = parseIndexFile();
    map<string, vector<IndexEntry>> directory_entries;

    // Sort entries into directories
    for (const auto& entry : index_entries) {
        size_t last_slash = entry.path.find_last_of('/');
        string dir = last_slash == string::npos ? "" : entry.path.substr(0, last_slash);
        directory_entries[dir].push_back(entry);
    }

    // Generate tree object
    string tree_sha = create_tree_object(index_entries);

    // Retrieve parent SHA directly from the master file if it exists
    string parent_sha = string(40, '0');  
    int head_fd = open(".mygit/refs/heads/master", O_RDONLY);
    if (head_fd >= 0) {
        char buffer[41];
        ssize_t bytes_read = read(head_fd, buffer, 40);
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';
            parent_sha = string(buffer);  
        }
        close(head_fd);
    }

    // Commit metadata
    string timestamp = to_string(time(0));
    string commit_content = "tree " + tree_sha + "\n";
    commit_content += "parent " + parent_sha + "\n";
    commit_content += "message " + message + "\n";
    commit_content += "timestamp " + timestamp + "\n";

    // Hash the commit content
    string commit_sha = sha1(commit_content);
    string commit_dir = ".mygit/objects/" + commit_sha.substr(0, 2);
    string commit_file = commit_dir + "/" + commit_sha.substr(2);
    mkdir(commit_dir.c_str(), 0755);
    write_file(commit_file, commit_content);

    int index_fd = open(".mygit/index", O_WRONLY | O_TRUNC);
    if (index_fd < 0) {
        perror("Error clearing index file");
    } 
    else {
        close(index_fd);
    }

    // Update master with the new commit SHA
    write_file(".mygit/refs/heads/master", commit_sha + "\n");

    cout << "Committed as " << commit_sha << endl;
}
