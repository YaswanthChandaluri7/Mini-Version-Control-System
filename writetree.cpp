#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <openssl/sha.h>
#include <zstd.h>
#include <string>
#include <vector>
#include <iostream>
#include <dirent.h>
#include <map>
#include "header.h"
using namespace std;



//  to store blob or tree in objects
void store_object(const string& sha, const string& content) {
    string dir_path = ".mygit/objects/" + sha.substr(0, 2);
    string file_path = dir_path + "/" + sha.substr(2);

    struct stat st;
    if (stat(dir_path.c_str(), &st) == -1) {
        mkdir(dir_path.c_str(), 0755);
    }

    int fd = open(file_path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        perror("Error creating object file");
        return;
    }
    write(fd, content.c_str(), content.size());
    close(fd);
}

    
    string create_compressed_blob(const string& file_path) {
    int file_fd = open(file_path.c_str(), O_RDONLY);
    if (file_fd < 0) {
        perror(("Error opening file: " + file_path).c_str());
        return "";
    }

    
    vector<char> file_content;
    char buffer[4096];
    ssize_t bytes_read;
    while ((bytes_read = read(file_fd, buffer, sizeof(buffer))) > 0) {
        file_content.insert(file_content.end(), buffer, buffer + bytes_read);
    }
    close(file_fd);

    // allocate space for compressed data using ZSTD_compressBound
    size_t compressed_size_bound = ZSTD_compressBound(file_content.size());
    vector<char> compressed_data(compressed_size_bound);

    // compress using ZSTD_compress
    size_t compressed_size = ZSTD_compress(compressed_data.data(), compressed_size_bound,
                                           file_content.data(), file_content.size(),
                                           ZSTD_CLEVEL_DEFAULT);

    if (ZSTD_isError(compressed_size)) {
        cerr << "Error compressing file content: " << ZSTD_getErrorName(compressed_size) << endl;
        return "";
    }
    // resize to actual compressed size
    compressed_data.resize(compressed_size);  

    // calculate SHA-1 on compressed data
    string compressed_content(compressed_data.begin(), compressed_data.end());
    string sha = sha1("blob " + to_string(compressed_content.size()) + "\0" + compressed_content);

    
    store_object(sha, compressed_content);
    return sha;
}

    // recursive function to create tree objects for directories
    string create_tree(const string& dir_path) {
        vector<string> entries;
        DIR* dir = opendir(dir_path.c_str());
        if (!dir) {
            perror(("Error opening directory: " + dir_path).c_str());
            return "";
        }

        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            string name = entry->d_name;

            // skip hidden files and directories
            if (name == "." || name == ".." || name[0] == '.') continue;

            string full_path = dir_path + "/" + name;
            string entry_sha;

            if (entry->d_type == DT_DIR) {
                // for subdirectories
                entry_sha = create_tree(full_path);
                entries.push_back("040000 tree " + entry_sha + " " + name);
            } 
            else if (entry->d_type == DT_REG) {
                // regular files
                entry_sha = create_compressed_blob(full_path);
                entries.push_back("100644 blob " + entry_sha + " " + name);
            }
        }

        closedir(dir);

        // Build tree object content
        string tree_content;
        for (const auto& entry : entries) {
            tree_content += entry + "\n";
        }

        string tree_sha = sha1("tree " + to_string(tree_content.size()) + "\0" + tree_content);
        store_object(tree_sha, tree_content);
        return tree_sha;
    }

void write_tree() {
    string root_tree_sha = create_tree(".");
    cout << "tree SHA-1: " << root_tree_sha << endl;
}

