#include <iostream>
#include <vector>
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <cstring>
#include <zstd.h>
#include "header.h"
using namespace std;

string getObjectFilePath(const string &hash) {
    string objectsDir = ".mygit/objects";
    string subDir = objectsDir + "/" + hash.substr(0, 2);
    return subDir + "/" + hash.substr(2);
}

vector<unsigned char> readCompressedBlobData(const string &filePath) {
    int fd = open(filePath.c_str(), O_RDONLY);
    if (fd < 0) {
        cerr << "Error: Unable to open file " << filePath << ": " << strerror(errno) << endl;
        return {};
    }

    struct stat st;
    if (fstat(fd, &st) < 0) {
        cerr << "Error: Unable to get file size: " << strerror(errno) << endl;
        close(fd);
        return {};
    }

    vector<unsigned char> buffer(st.st_size);
    if (read(fd, buffer.data(), buffer.size()) < 0) {
        cerr << "Error: Unable to read file: " << strerror(errno) << endl;
        close(fd);
        return {};
    }
    close(fd);

    return buffer;
}

string decompressBlob(const vector<unsigned char> &compressedData, size_t headerEnd) {
    size_t decompressedSize = 10 * 1024 * 1024; // Estimate decompressed size
    vector<char> decompressedData(decompressedSize);

    size_t actualDecompressedSize = ZSTD_decompress(decompressedData.data(), decompressedSize,
                                                    compressedData.data() + headerEnd + 1,
                                                    compressedData.size() - headerEnd - 1);

    if (ZSTD_isError(actualDecompressedSize)) {
        cerr << "Error: Decompression failed: " << ZSTD_getErrorName(actualDecompressedSize) << endl;
        return "";
    }

    return string(decompressedData.data(), actualDecompressedSize);
}

void processBlob(const string &flag, const string &decompressedBlob) {
    if (flag == "-p") {
        cout << decompressedBlob << endl; 
    } 
    else if (flag == "-s") {
        cout << "Size: " << decompressedBlob.size() << " bytes" << endl; 
    } 
    else if (flag == "-t") {
        cout << "Type: blob" << endl; 
    } 
    else {
        cerr << "Unknown flag: " << flag << endl;
    }
}

void processTree(const string &flag, const string &treeContent) {
    if (flag == "-p") {
        cout << treeContent << endl; 
    } 
    else if (flag == "-s") {
        cout << "Size: " << treeContent.size() << " bytes" << endl; 
    } 
    else if (flag == "-t") {
        cout << "Type: tree" << endl; 
    } 
    else {
        cerr << "Unknown flag: " << flag << endl;
    }
}

void catFile(const string &flag, const string &fileHash) {
    string objectFilePath = getObjectFilePath(fileHash);
    vector<unsigned char> fileData = readCompressedBlobData(objectFilePath);

    if (fileData.empty()) {
        cerr << "Error: File data is empty." << endl;
        return; 
    }

    // Locate the header end in compressed data for blob type
    size_t headerEnd = find(fileData.begin(), fileData.end(), '\0') - fileData.begin();
    
    // Check if this is a blob by looking at the header prefix
    string header(fileData.begin(), fileData.begin() + headerEnd);
    if (header.substr(0, 4) == "blob") {
        // Decompress and process as blob
        string decompressedBlob = decompressBlob(fileData, headerEnd);
        
        if (decompressedBlob.empty()) {
            return; 
        }
        processBlob(flag, decompressedBlob);
    } else {
        // Process as tree, starting from the beginning
        string treeContent(fileData.begin(), fileData.end());
        processTree(flag, treeContent);
    }
}
