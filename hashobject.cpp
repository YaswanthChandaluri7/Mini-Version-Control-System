#include <iostream>
#include <sstream>
#include <string>
#include <iomanip>      // for setw and setfill
#include <openssl/sha.h> 
#include <sys/stat.h>   
#include <fcntl.h>
#include <zstd.h>
#include <unistd.h>     
#include <cstring>      
#include <cerrno>   
#include "header.h"    
using namespace std;

string sha1(const string &data) {
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1(reinterpret_cast<const unsigned char *>(data.c_str()), data.size(), hash);

    ostringstream oss;
    for (int i = 0; i < SHA_DIGEST_LENGTH; i++) {
        oss << hex << setw(2) << setfill('0') << (int)hash[i];
    }

    return oss.str();
}

string readfilecontent(const string &filePath) {
    int fd = open(filePath.c_str(), O_RDONLY);
    if (fd < 0) {
        cerr << "Error: Unable to open file " << filePath << ": " << strerror(errno) << endl;
        return "";
    }

    string content;
    char buffer[1024];
    ssize_t bytesRead;
    while ((bytesRead = read(fd, buffer, sizeof(buffer))) > 0) {
        content.append(buffer, bytesRead);
    }

    if (bytesRead < 0) {
        cerr << "Error: Unable to read file " << filePath << ": " << strerror(errno) << endl;
    }

    close(fd);
    return content;
}

bool createDirectory(const string &path) {
    struct stat st;
    if (stat(path.c_str(), &st) != 0) {
        if (mkdir(path.c_str(), 0755) != 0) {
            cerr << "Error: Unable to create directory " << path << ": " << strerror(errno) << endl;
            return false;
        }
    }
    return true;
}

void writeblobobject(const string &fileContent, const string &hash) {
    string objectsDir = ".mygit/objects";
    string subDir = objectsDir + "/" + hash.substr(0, 2);
    if (!createDirectory(subDir)) {
        return;
    }

    string objectFile = subDir + "/" + hash.substr(2);
    int fd = open(objectFile.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        cerr << "Error: Unable to write blob object to " << objectFile << ": " << strerror(errno) << endl;
        return;
    }

    // Compress the file content
    size_t compressedSize = ZSTD_compressBound(fileContent.size());
    vector<char> compressedData(compressedSize);

    size_t actualCompressedSize = ZSTD_compress(compressedData.data(), compressedSize, fileContent.data(), fileContent.size(), 1);
    if (ZSTD_isError(actualCompressedSize)) {
        cerr << "Error: Compression failed: " << ZSTD_getErrorName(actualCompressedSize) << endl;
        close(fd);
        return;
    }

    compressedData.resize(actualCompressedSize);

    // Create blob header and write to the file
    string blobHeader = "blob " + to_string(fileContent.size()) + '\0';
    if (write(fd, blobHeader.c_str(), blobHeader.size()) < 0) {
        cerr << "Error: Unable to write blob header to file " << objectFile << ": " << strerror(errno) << endl;
        close(fd);
        return;
    }

    // Write the compressed data to the blob file
    if (write(fd, compressedData.data(), compressedData.size()) < 0) {
        cerr << "Error: Unable to write compressed data to " << objectFile << ": " << strerror(errno) << endl;
    }

    close(fd);
}

void hashobject(const string &filePath, bool writeBlob) {
    string fileContent = readfilecontent(filePath);
    if (fileContent.empty()) {
        return;
    }

    string hash = sha1(fileContent);

    if (writeBlob) {
        writeblobobject(fileContent, hash);
    }

    cout << hash << endl;
}
