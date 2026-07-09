# MiniGit Command Reference

This document outlines every command implemented in the MiniGit Version Control System and explains its underlying function.

To use these commands, build the project and execute the `minigit` binary:
```bash
./build/minigit <command> [args...]
```

## Setup & Build Commands

Before using MiniGit, you must build the C++ source code into an executable. Run these commands from the root of the project:

### 1. Configure the Build System
```bash
cmake -B build
```
- **Function:** Tells CMake to read the `CMakeLists.txt` and generate the necessary Makefile structure inside a new `build/` directory.

### 2. Compile the Project
```bash
cmake --build build
```
- **Function:** Compiles the source code into the `minigit` executable and the `minigit_tests` executable.

### 3. Run the Automated Tests
```bash
./build/minigit_tests
```
- **Function:** Executes the GoogleTest suite to verify that Hashing, Diffing, CDC, and Garbage Collection are working perfectly.

---

## Core Version Control Commands

### 1. `init`
**Usage:** `minigit init`
- **Function:** Initializes a new, empty MiniGit repository in the current working directory. 
- **Under the hood:** Creates the `.minigit` hidden folder structure, including `.minigit/objects/` (for the database), `.minigit/refs/heads/` (for branches), and the default `.minigit/HEAD` file pointing to `refs/heads/main`.

### 2. `add`
**Usage:** `minigit add <filepath>`
- **Function:** Stages a file, preparing it to be included in the next commit.
- **Under the hood:** Reads the file, hashes its contents using BLAKE3, creates chunks via the Rolling Hash CDC algorithm, compresses the chunks with zlib, stores them in the object database, and updates the `.minigit/index` file to map the file path to the new blob manifest hash.

### 3. `commit`
**Usage:** `minigit commit -m "<message>"`
- **Function:** Takes a permanent snapshot of the currently staged files in the index.
- **Under the hood:** 
  1. Reads the `.minigit/index` and constructs a `Tree` object.
  2. Constructs a `Commit` object pointing to this `Tree`, and includes the current `HEAD` as its parent.
  3. Writes the Commit object to the database.
  4. Updates the branch reference (e.g., `refs/heads/main`) to point to this new commit hash.

### 4. `diff`
**Usage:** `minigit diff <filepath>`
- **Function:** Shows the line-by-line differences between your working directory file and the version currently staged in the index.
- **Under the hood:** Fetches the blob manifest from the index, reconstructs the file by pulling and decompressing all CDC chunks, reads your current working file, and runs the **Myers Diff Algorithm** to calculate the shortest sequence of additions (`+`) and deletions (`-`).

---

## System Optimization & Maintenance

### 5. `gc` (Garbage Collection)
**Usage:** `minigit gc`
- **Function:** Cleans up the object database by deleting orphaned chunks and blobs to save disk space.
- **Under the hood:** Performs a **Mark-and-Sweep** algorithm. It traverses the Directed Acyclic Graph (DAG) starting from all branch pointers and the index, marking every reachable Commit, Tree, and Chunk. It then sweeps the `.minigit/objects/` directory and deletes any file that wasn't marked as reachable.

---

## Debug & Observability Tools

### 6. `status`
**Usage:** `minigit status`
- **Function:** Displays the current state of the staging area.
- **Under the hood:** Reads `.minigit/index` and outputs the list of files currently marked as "Changes to be committed".

### 7. `log`
**Usage:** `minigit log`
- **Function:** Prints the chronological commit history of the current branch.
- **Under the hood:** Reads the commit hash from `.minigit/HEAD`, decompresses the commit object, parses the embedded commit message, and iteratively follows the `parent <hash>` pointers backward in time to the root commit, printing each hash and message to the console.

### 8. `cat-file`
**Usage:** `minigit cat-file [-p] <hash>`
- **Function:** Inspects the raw, internal data of any object in the database.
- **Under the hood:** Decompresses the file located at `.minigit/objects/<hash[0:2]>/<hash[2:]>`. If the `-p` (pretty-print) flag is provided and the object is a chunk manifest, it will automatically fetch and assemble all the underlying chunks, printing the file exactly as it was originally added.

### 9. `visualize.py`
**Usage:** `python3 visualize.py`
- **Function:** An included Python utility script that visually charts the internal database graph.
- **Under the hood:** It scans the `.minigit/objects/` folder, decompresses all blobs, trees, commits, and chunks, resolves their dependencies, and outputs a `visualize.html` file using Mermaid.js. Opening this HTML file in your browser reveals a color-coded diagram of your DAG!

---

## End-to-End Workflow Example

If you want to demonstrate the entire project from the terminal, run these commands in order:

```bash
# 1. Create a demo directory
mkdir demo_repo
cd demo_repo

# 2. Initialize MiniGit
../build/minigit init

# 3. Create a file and add some content
echo "Hello World!" > myfile.txt

# 4. Check the status (Shows myfile.txt as untracked/modified)
../build/minigit status

# 5. Stage the file
../build/minigit add myfile.txt

# 6. Commit the file to the database
../build/minigit commit -m "First commit: added myfile.txt"

# 7. Modify the file
echo "MiniGit is awesome!" >> myfile.txt

# 8. View the diff (Shows what you just added)
../build/minigit diff myfile.txt

# 9. Stage and commit the modification
../build/minigit add myfile.txt
../build/minigit commit -m "Second commit: updated myfile.txt"

# 10. View the commit history
../build/minigit log

# 11. Run Garbage Collection to clean up old orphaned chunks
../build/minigit gc

# 12. Generate a visual chart of the repository
python3 ../visualize.py
# (Then open the generated visualize.html in your web browser!)
```
