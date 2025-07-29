# AOS-Asssignment-4

# MyGit: A Mini Version Control System

MyGit is a simplified version control system that replicates some core functionalities of Git. It allows you to initialize a repository, track file changes, commit snapshots, and restore project versions.This is implemented in c++ language.

## Table of Contents
1. [Initialize Repository](#1-initialize-repository)
2. [Hash-Object](#2-hash-object)
3. [Cat-File](#3-cat-file)
4. [Write Tree](#4-write-tree)
5. [List Tree (ls-tree)](#5-list-tree-ls-tree)
6. [Add Files](#6-add-files)
7. [Commit Changes](#7-commit-changes)
8. [Log Command](#8-log-command)
9. [Checkout Command](#9-checkout-command)


## 1.Repository Initialization

The `repository()` function is responsible for initializing a new MyGit repository. It performs the following tasks:

1. **Directory Check**: 
   - It first checks if a directory named `.mygit` already exists using the `directoryexists` function. If the directory exists, it displays an error message indicating that the repository is already initialized.

2. **Create Repository Directory**: 
   - If the `.mygit` directory does not exist, it attempts to create it using the `createdirectory` function. This function creates the directory with permissions set to `0755`.

3. **Create Necessary Subdirectories**: 
   - The function then creates essential subdirectories within the `.mygit` directory:
     - `objects`: This will store the blob objects of files.
     - `refs`: This will hold references to commits.
     - `refs/heads`: This subdirectory will store the heads of branches.
   
4. **Initialize Configuration Files**: 
   - After creating the necessary directories, the function initializes several key files:
     - **HEAD**: This file is created to point to the current branch (defaulting to `refs/heads/master`).
     - **config**: This file contains configuration settings for the repository, including the version and file mode.
     - **index**: This is an empty file created to keep track of the staging area.

5. **Confirmation Message**: 
   - Once all operations are successfully completed, a confirmation message is displayed indicating that the MyGit repository has been initialized.

>Execution:
- we need to execute the make file which contain all the cpp files.In the terminal we need to give as below:
>make

- to execute the above command
> ./mygit init

## 2. Hash-Object

The `hashobject` function calculates the SHA-1 hash of a file and optionally stores it as a compressed blob in the `.mygit` repository.

### Key Functions

1. **`sha1`**: 
   - Generates the SHA-1 hash of the file content and returns it as a string.

2. **`readfilecontent`**:
   - Reads the entire content of the specified file, handling any errors during file access.

3. **`createDirectory`**:
   - Creates a new directory if it does not already exist, setting up the storage structure for blobs.

4. **`writeblobobject`**:
   - Compresses the file content using Zstandard and stores it as a blob in the `.mygit/objects` directory.
   - A unique directory and file are created based on the SHA-1 hash, ensuring each blob is uniquely identified.

5. **`hashobject`**:
   - Orchestrates the hash and blob storage process. 
   - If the `-w` option is provided, it stores the blob; otherwise, it only outputs the SHA-1 hash.

### Usage
To calculate the SHA-1 hash of a file and store it as a blob, run:

>./mygit hash-object [-w] <file>


## 3.Cat-File: 

The `catFile` function allows inspecting objects in the `.mygit` repository. It can display content, show type, and report the size of objects such as blobs and trees.

### Key Functions

1. **`getObjectFilePath`**:
   - Constructs the file path for an object based on its SHA-1 hash.

2. **`readCompressedBlobData`**:
   - Reads compressed blob data from the given file path, handling errors during file access.

3. **`decompressBlob`**:
   - Decompresses blob data using Zstandard, returning the decompressed content as a string.

4. **`processBlob`**:
   - Processes blob objects based on the specified flag:
     - `-p`: Prints the content of the blob.
     - `-s`: Displays the blob’s size.
     - `-t`: Shows the object type (blob).

5. **`processTree`**:
   - Processes tree objects similarly to `processBlob`, handling flags `-p`, `-s`, and `-t`.

6. **`catFile`**:
   - Reads and inspects an object based on its hash. Determines if the object is a blob or a tree, then processes accordingly using `processBlob` or `processTree`.

### Usage
To inspect an object, use:

>./mygit cat-file <flag> <object_hash>

## 4.Write-tree

This implementation stores compressed blob and tree objects in the `.mygit/objects` directory. It compresses files, calculates SHA-1 hashes, and organizes objects into directories based on their hash.

### Key Functions

1. **`store_object`**:
   - Stores a blob or tree object in the `.mygit/objects` directory. It organizes files by creating a subdirectory using the first two characters of the SHA-1 hash and stores the compressed content.

2. **`create_compressed_blob`**:
   - Reads a file’s content, compresses it using Zstandard, calculates a SHA-1 hash on the compressed content, and stores the blob object in `.mygit/objects`. It returns the blob’s SHA-1 hash.

3. **`create_tree`**:
   - Recursively traverses a directory and creates tree objects. For each file, it creates a compressed blob object; for each subdirectory, it creates a nested tree. Each entry includes permissions, type, hash, and name. The function assembles these into a tree object, calculates its SHA-1 hash, and stores it in `.mygit/objects`.

4. **`write_tree`**:
   - Calls `create_tree` for the current directory (`"."`), creating a root tree object and outputting its SHA-1 hash.

### Usage

To write the current directory as a tree object, use:

>./mygit write-tree

## 5.`ls_tree`: 

This code retrieves and displays the contents of a tree object in a Git-style repository. It shows the mode, type, SHA-1, and name of each entry, with optional recursive display for subtrees.

### Key Functions

1. **`get_object_content`**:
   - Retrieves the raw content of a stored object file based on its SHA-1 hash by reading it from the `.mygit/objects` directory.

2. **`display_tree_content`**:
   - Parses and displays the tree object's entries. It distinguishes between `blob` (file) and `tree` (subdirectory) entries.
   - If the entry is a `tree`, it recursively retrieves and displays its contents, appending each name to build a complete path.
   - The function supports an optional `name_only` mode, where only the names of files and directories are displayed.

3. **`ls_tree`**:
   - Initiates the display process by retrieving the main tree content using `get_object_content` and calling `display_tree_content`.
   - Outputs an error if the tree object is not found.

### Usage

To display the contents of a tree object, use:

>./mygit ls-tree <tree_sha> [-name_only]

### 6. add files:
**`Structure:`**

Files and directories added are stored in the .mygit/objects folder. The index file maintains references to these objects.

```Function Descriptions:```

`addtostagingarea`:
Adds specified paths to the staging area, identifying files and directories and handling each accordingly.

`ishidden:`Identifies hidden files, used to skip system and hidden files during staging.

`relativepath:`Calculates the relative path of a file based on a base path, allowing consistent object naming in the staging area.

`is_directory:`Checks if a path points to a directory, facilitating recursive directory traversal.

`filetoindex:`Adds a file to the index, calculating its SHA-1 hash, storing its content, and updating the .mygit/index with an entry.

`directorytoindex:`Recursively adds all files in a directory to the index by invoking filetoindex on each regular file.

>To execute: ./mygit add filename

## 7.Commit:

`Key Functions`

`get_object_content:`
Retrieves the raw content of a Git object file (like blob, tree, or commit) by locating it in the .mygit/objects directory using its SHA-1 hash. This function reads and returns the stored content, allowing access to any object data by its hash.

`display_tree_content:`
Parses and displays entries within a tree object, identifying whether each entry is a blob (file) or another tree (subdirectory). If the entry is a tree, the function recursively fetches and displays its contents, constructing complete paths for nested items. Additionally, it supports a name_only mode, which shows only the names without full details.

`ls_tree:`
Starts the tree display process by retrieving the main tree object content with get_object_content and passing it to display_tree_content. It shows the structure and contents of a specified tree object, and if the object isn’t found, it displays an error. This command is useful for listing contents in a structured format.

`Usage:`

To display a tree object's contents, use a command structured like:

>./mygit ls-tree <tree_sha> [-name_only]

## 8.log file:

This will output the tree structure, showing files and directories stored in the specified tree object.

`Function Descriptions:`

`read_file_content:`
Reads and returns the entire content of a specified file.


`extract_field:`
Extracts and returns the value of a specified field from commit content.

`display_commit_info:`
Displays the extracted information from a commit, including tree SHA, parent SHA, commit message, timestamp, and committer.

`log:`
Main function to read the latest commit SHA from the master file, navigate through the commit history, and display the commit details until the root commit (indicated by a parent SHA of 40 zeroes) is reached.

* To excucute:
> ./mygit log

## 9.checkout:
`Function Descriptions:`

`write_file:`
Writes the provided content to a file at the specified path. Creates the file if it doesn't exist or truncates it if it does.

`create_directory1:`
Creates a directory at the specified path, ignoring errors if the directory already exists.

`restore_tree:`
Reads the tree object from the repository and restores the files and directories according to the tree SHA. It decompresses blob content and writes it to the appropriate file paths.

`checkout:`
Main function that initializes the checkout process for a specified tree SHA. It creates a base directory (src by default) and calls restore_tree to restore the file structure.


* To execute:
> ./mygit checkout <tree_sha>
