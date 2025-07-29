#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <iostream>
#include <string>
#include <cstring>
#include "header.h"
using namespace std;

// function to read a file's entire content
string read_file_content(const string& file_path) {
    int fd = open(file_path.c_str(), O_RDONLY);
    if (fd < 0) {
        perror(("Error opening file: " + file_path).c_str());
        return "";
    }

    string content;
    char buffer[4096];
    ssize_t bytes_read;
    while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
        content.append(buffer, bytes_read);
    }
    close(fd);
    return content;
}

// function to extract specific field values from commit content
string extract_field(const string& content, const string& field_name) {
    size_t field_pos = content.find(field_name);
    if (field_pos == string::npos) return "";  
    size_t value_start = content.find(' ', field_pos) + 1;
    size_t value_end = content.find('\n', value_start);
    return content.substr(value_start, value_end - value_start);
}

void display_commit_info(const string& tree_sha, const string& commit_content) {
    cout << "Tree SHA: " << tree_sha << endl; 

    string parent_sha = extract_field(commit_content, "parent");
    if (parent_sha.empty()) {
        cout << "Parent SHA: " << string(40, '0') << endl;  
    } else {
        cout << "Parent SHA: " << parent_sha << endl;  
    }

    string message = extract_field(commit_content, "message");
    string timestamp = extract_field(commit_content, "timestamp");

    cout << "Message: " << message << endl;
    cout << "Timestamp: " << timestamp << endl;
    cout << "Committer: You" << endl;  
    cout << endl;  
}

void log() {
    // read the latest commit SHA from the master file
    string master_file = ".mygit/refs/heads/master";
    string latest_commit_sha = read_file_content(master_file);
    if (latest_commit_sha.empty()) {
        cerr << "No commits found in the repository." << endl;
        return;
    }

    // remove any newline characters
    latest_commit_sha.erase(latest_commit_sha.find_last_not_of("\n") + 1);

    string current_sha = latest_commit_sha;

    // stop when parent SHA is 40 zeroes
    const string zero_sha(40, '0');
    
    while (!current_sha.empty()) {
        string commit_dir = ".mygit/objects/" + current_sha.substr(0, 2);
        string commit_file = commit_dir + "/" + current_sha.substr(2);

        // read the commit file content
        string commit_content = read_file_content(commit_file);
        if (commit_content.empty()) {
            cerr << "Error: Could not read commit object for SHA " << current_sha << endl;
            break;
        }

        // extract the tree SHA
        string tree_sha = extract_field(commit_content, "tree");

        // display commit information with the tree SHA
        display_commit_info(tree_sha, commit_content);

        // retrieve the parent SHA for the next iteration
        current_sha = extract_field(commit_content, "parent");

        // check if parent SHA is 40 zeroes
        if (current_sha == zero_sha) {
            break;
        }
    }
}
