#ifndef MY_HEADER_H 
#define MY_HEADER_H
#include <iostream>
#include <sys/stat.h>  
#include <fcntl.h>     
#include <unistd.h>    
#include <cstring>    
#include <cerrno> 
#include<bits/stdc++.h>
using namespace std;
// init
bool directoryexists(const string &dirName);
bool createdirectory(const string &dirName);
void initializeHEAD(const string &filePath);
void initializeconfig(const string &filePath);
void repository();

// hash-object
string sha1(const string &data);
string readfilecontent(const string &filePath);
void writeblobobject(const string &fileContent, const string &hash);
void hashobject(const string &filepath, bool writeBlob);
bool createDirectory(const string &path) ;

// catfile
void catFile(const string &flag, const string &fileHash);
void processTree(const string &flag, const string &treeContent);
void processBlob(const string &flag, const string &decompressedBlob);
string decompressBlob(const vector<unsigned char> &compressedData, size_t headerEnd);
vector<unsigned char> readCompressedBlobData(const string &filePath);
string getObjectFilePath(const string &hash);

//add
bool ishidden(const string& name);
string relativepath(const string& base_path, const string& full_path);
bool is_directory(const string& path);
string get_file_mode(const string& file_path) ;
void filetoindex(const string& base_path, int index_fd, const string& file_path);
void directorytoindex(const string& base_path, int index_fd, const string& dir_path);

// writetree
void write_tree();
string create_tree(const string& dir_path) ;
void store_object(const string& sha, const string& content);
string create_compressed_blob(const string& file_path);

// ls tree
void ls_tree(const string& tree_sha, bool name_only);
void display_tree_content(const string& tree_content, bool name_only, const string& prefix="") ;
string get_object_content(const string& sha);

// commit
struct IndexEntry {
    string mode;
    string type;
    string sha;
    string path;
};
void commit(const string& message);
vector<IndexEntry> parseIndexFile();
string create_tree_object(const vector<IndexEntry>& entries);
void write_file1(const string& path, const string& content);

// log
string read_file_content(const string& file_path);
string extract_field(const string& content, const string& field_name);
void display_commit_info(const string& commit_sha, const string& commit_content);
void log();

// checkout
void checkout(const string& tree_sha) ;
void create_directory1(const string& dir_path);
void write_file(const string& file_path, const string& content);
void restore_tree(const string& tree_sha, const string& base_path);
#endif