#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <zstd.h>  
#include <string>
#include <vector>
#include <iostream>
#include <cstdio>
#include <sys/types.h>
#include <dirent.h>
#include "header.h"

using namespace std;


void write_file(const string& file_path, const string& content) {
    int fd = open(file_path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        perror(("Error opening file for writing: " + file_path).c_str());
        return;
    }

    ssize_t bytes_written = write(fd, content.data(), content.size());
    if (bytes_written < 0) {
        perror(("Error writing to file: " + file_path).c_str());
    }

    close(fd);
}


void create_directory1(const string& dir_path) {
    if (mkdir(dir_path.c_str(), 0755) != 0 && errno != EEXIST) {
        cerr << "Error creating directory: " << dir_path << endl;
    }
}

void restore_tree(const string& tree_sha, const string& base_path) {
    
    string tree_dir = ".mygit/objects/" + tree_sha.substr(0, 2);
    string tree_file = tree_dir + "/" + tree_sha.substr(2);

    // Read tree content
    string tree_content = readfilecontent(tree_file);
    if (tree_content.empty()) {
        cerr << "Error: Could not read tree object for SHA " << tree_sha << endl;
        return;
    }

    size_t start = 0;
    size_t end = 0;

    
    while ((end = tree_content.find('\n', start)) != string::npos) {
        string line = tree_content.substr(start, end - start);
        start = end + 1;

        
        size_t first_space = line.find(' ');
        size_t second_space = line.find(' ', first_space + 1);
        size_t third_space = line.find(' ', second_space + 1);

        if (first_space == string::npos || second_space == string::npos || third_space == string::npos) {
            cerr << "Error: Invalid line format: " << line << endl;
            continue;
        }

        string mode = line.substr(0, first_space);
        string type = line.substr(first_space + 1, second_space - first_space - 1);
        string sha = line.substr(second_space + 1, 40);
        string path = line.substr(third_space + 1);

        
        string output_path = base_path + "/" + path;

        if (type == "tree") {
            create_directory1(output_path);
            restore_tree(sha, output_path);
        } 
        else if (type == "blob") {
            size_t last_slash = output_path.find_last_of('/');
            if (last_slash != string::npos) {
                string dir_path = output_path.substr(0, last_slash);
                create_directory1(dir_path);
            }

            
            string blob_dir = ".mygit/objects/" + sha.substr(0, 2);
            string blob_file = blob_dir + "/" + sha.substr(2);
            string blob_content = readfilecontent(blob_file);
            if (blob_content.empty()) {
                cerr << "Error: Could not read blob object for SHA " << sha << endl;
                continue;
            }

            size_t header_end = blob_content.find('\0');
            if (header_end == string::npos || blob_content.substr(0, 5) != "blob ") {
                cerr << "Error: Invalid blob header for SHA " << sha << endl;
                continue;
            }

            string size_str = blob_content.substr(5, header_end - 5);
            size_t uncompressed_size = stoi(size_str);

            
            const char* compressed_data = blob_content.data() + header_end + 1;
            size_t compressed_data_size = blob_content.size() - header_end - 1;

            
            vector<char> decompressed_data(uncompressed_size);
            size_t result = ZSTD_decompress(decompressed_data.data(), uncompressed_size, compressed_data, compressed_data_size);
            if (ZSTD_isError(result)) {
                cerr << "Error: Decompression failed for blob " << sha << " with result: " << ZSTD_getErrorName(result) << endl;
                continue;
            }

            
            if (result != uncompressed_size) {
                cerr << "Error: Blob data size mismatch for SHA " << sha << endl;
                continue;
            }

            
            write_file(output_path, string(decompressed_data.data(), uncompressed_size));
        } 
        else {
            cerr << "Error: Unknown type in tree: " << type << endl;
        }
    }
}

void checkout(const string& tree_sha) {
    string base_path = "src";
    create_directory1(base_path);
    restore_tree(tree_sha, base_path);
    cout << "Checked out to tree " << tree_sha << endl;
}


