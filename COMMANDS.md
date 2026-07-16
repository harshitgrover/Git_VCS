# MiniGit Command Reference

This document outlines every command implemented in the MiniGit Version Control System and explains its underlying function.

To use these commands, first build the project and set up your alias:
```bash
# Add this to your ~/.zshrc or equivalent shell config
alias minigit='$PWD/build/minigit'
```

Now you can run the commands from anywhere:
```bash
minigit <command> [args...]
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

### 4. Run the Performance Benchmarks
```bash
./build/minigit_benchmark
```
- **Function:** Runs stress tests on the BLAKE3 hasher, Myers Diff, and CDC engines. It automatically generates and saves statistical data (like deduplication percentages and MB/s throughput) into a `benchmark_results.txt` file in your root folder.

---

## Core Version Control Commands

### 1. `init`
**Usage:** `minigit init [directory]`
- **Function:** Initializes a new, empty MiniGit repository. If a `directory` name is provided, it creates that folder automatically. If omitted, it initializes in the current working directory.
- **Under the hood:** Creates the `.minigit` hidden folder structure recursively, including `.minigit/objects/` (for the database), `.minigit/refs/heads/` (for branches), and the default `.minigit/HEAD` file pointing to `refs/heads/main`.

### 2. `add`
**Usage:** `minigit add <filepath_or_directory>` (or `-A` / `--all`)
- **Function:** Stages a file or an entire directory, preparing it to be included in the next commit. If you pass `.`, it recursively adds everything in the current directory. If you pass `-A` or `--all`, it recursively adds all changes from the root of the entire repository.
- **Under the hood:** Recursively scans directories. For each file, it hashes its contents using BLAKE3, creates chunks via the Rolling Hash CDC algorithm, compresses the chunks with zlib, stores them in the object database, and updates the `.minigit/index` file to map the file path to the new blob manifest hash.

### 3. `commit`
**Usage:** `minigit commit -m "<message>"`
- **Function:** Takes a permanent snapshot of the currently staged files in the index. The `-m` flag is required to attach a short, human-readable description (message) to the snapshot so you can identify it later in the log.
- **Under the hood:** 
  1. Reads the `.minigit/index` and constructs a `Tree` object.
  2. Constructs a `Commit` object pointing to this `Tree`, and includes the current `HEAD` as its parent.
  3. Writes the Commit object to the database.
  4. Updates the branch reference (e.g., `refs/heads/main`) to point to this new commit hash.

### 4. `diff`
**Usage:** `minigit diff <filepath>`
- **Function:** Shows the line-by-line differences between your working directory file and the version currently staged in the index.
- **Under the hood:** Fetches the blob manifest from the index, reconstructs the file by pulling and decompressing all CDC chunks, reads your current working file, and runs the **Myers Diff Algorithm** to calculate the shortest sequence of additions (`+`) and deletions (`-`).

### 5. `branch`
**Usage:** `minigit branch [branch_name]`
- **Function:** If a `branch_name` is provided, it creates a new branch pointer at the current commit. If called with no arguments, it lists all available branches and highlights the currently active branch.
- **Under the hood:** To create a branch, it creates a new text file at `.minigit/refs/heads/<branch_name>` and writes the current commit hash (from `HEAD`) into it. To list branches, it simply iterates over all the text files inside the `.minigit/refs/heads/` directory.

### 6. `switch`
**Usage:** `minigit switch <branch_name>`
- **Function:** A dedicated command to switch your active branch and update your working directory.
- **Safety Feature:** MiniGit automatically checks your staging `Index`. If you have uncommitted changes that differ from the `HEAD` commit, it will safely abort the switch to prevent you from losing your work!
- **Under the hood:** Updates the `.minigit/HEAD` file to point to the new branch ref. Then, it parses the target branch's tree to reconstruct and overwrite the files in your working directory.

### 7. `restore`
**Usage 1 (Instant Undo):** `minigit restore <filepath_or_directory>`<br>
**Usage 2 (Time Travel):** `minigit restore <commit_hash> <filepath_or_directory>`<br>
**Usage 3 (Restore All):** `minigit restore .`<br>
- **Function:** Discards local changes or pulls old versions of files from history.
- **Under the hood:** By default, it reads your most recent `HEAD` commit. If you pass a `commit_hash`, it parses that historical commit instead. It searches the Tree for an exact file match (e.g., `src/main.cpp`) or a directory prefix (e.g., `src/`). It then decompresses the requested chunks and flawlessly reconstructs the past files directly into your present-day working directory.

### 8. `checkout` (Hybrid Router)
**Usage:** `minigit checkout <target>`
- **Function:** A modern, hybrid routing command that mimics standard Git.
- **Under the hood:** It dynamically evaluates the `<target>`. If `.minigit/refs/heads/<target>` exists, it realizes you are asking to change branches and automatically routes the request to the `switch` logic. If it doesn't exist, it assumes `<target>` is a file or directory and automatically routes the request to the `restore` logic.

### 9. `merge`
**Usage:** `minigit merge <branch_name>`
- **Function:** Merges the specified branch into the current branch using a 3-way merge algorithm.
- **Under the hood:** 
  1. Uses **Breadth-First Search (BFS)** to traverse the commit DAG and find the Lowest Common Ancestor (LCA).
  2. Extracts the trees for HEAD, the target branch, and the LCA.
  3. Evaluates every file to see if it was modified differently across branches.
  4. Automatically resolves changes, or detects conflicts and injects `<<<<<<<` markers into the file.
  5. Automatically creates a **Merge Commit** with two parent hashes.

---

## System Optimization & Maintenance

### 10. `gc` (Garbage Collection)
**Usage:** `minigit gc`
- **Function:** Cleans up the object database by deleting orphaned chunks and blobs to save disk space.
- **Under the hood:** Performs a **Mark-and-Sweep** algorithm. It traverses the Directed Acyclic Graph (DAG) starting from all branch pointers and the index, marking every reachable Commit, Tree, and Chunk. It then sweeps the `.minigit/objects/` directory and deletes any file that wasn't marked as reachable.

---

## Debug & Observability Tools

### 11. `status`
**Usage:** `minigit status`
- **Function:** Displays the current state of the staging area.
- **Under the hood:** Reads `.minigit/index` and outputs the list of files currently marked as "Changes to be committed".

### 12. `log`
**Usage:** `minigit log`
- **Function:** Prints the chronological commit history of the current branch.
- **Under the hood:** Reads the commit hash from `.minigit/HEAD`, decompresses the commit object, parses the embedded commit message, and iteratively follows the `parent <hash>` pointers backward in time to the root commit, printing each hash and message to the console.

### 13. `cat-file`
**Usage:** `minigit cat-file [-p] <hash>`
- **Function:** Inspects the raw, internal data of any object in the database.
- **Under the hood:** Decompresses the file located at `.minigit/objects/<hash[0:2]>/<hash[2:]>`. If the `-p` (pretty-print) flag is provided and the object is a chunk manifest, it will automatically fetch and assemble all the underlying chunks, printing the file exactly as it was originally added.

### 14. `delete`
**Usage:** `minigit delete [branch_name]`
- **Function:** Safely deletes a specific branch, or deletes the entire repository.
- **Under the hood:** If a branch name is provided, it deletes the branch reference file at `.minigit/refs/heads/<branch>`. If no branch is provided, it prompts for confirmation before recursively deleting the `.minigit` folder, reverting the directory to an un-tracked state.

### 15. `scripts/visualize.py`
**Usage:** `python3 scripts/visualize.py`
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
minigit init

# 3. Create a file and add some content
echo "Hello World!" > myfile.txt

# 4. Check the status (Shows myfile.txt as untracked/modified)
minigit status

# 5. Stage the file
minigit add myfile.txt

# 6. Commit the file to the database
minigit commit -m "First commit: added myfile.txt"

# 7. Modify the file
echo "MiniGit is awesome!" >> myfile.txt

# 8. View the diff (Shows what you just added)
minigit diff myfile.txt

# 9. Stage and commit the modification
minigit add myfile.txt
minigit commit -m "Second commit: updated myfile.txt"

# 10. View the commit history
minigit log

# 11. Create and switch to a new branch
minigit branch feature-branch
minigit checkout feature-branch

# 12. Modify a file on the new branch
echo "Adding feature work..." > feature.txt
minigit add feature.txt
minigit commit -m "Third commit: added feature.txt"

# 13. Switch back to the main branch
minigit checkout main

# 14. Merge the feature branch into main
minigit merge feature-branch

# 15. Run Garbage Collection to clean up old orphaned chunks
minigit gc

# 16. Generate a visual chart of the repository
python3 ../scripts/visualize.py
# (Then open the generated visualize.html in your web browser!)
```
