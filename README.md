# MiniGit - A Custom Version Control System

MiniGit is a fully functional, highly optimized Version Control System written from scratch in modern C++ (C++17/20). It mirrors the core architecture of standard Git but introduces advanced storage optimizations like **Content-Defined Chunking (CDC)** for massive deduplication efficiency.

### 🧠 Core Algorithms & Architecture Used
- **Directed Acyclic Graph (DAG) & Merkle Trees** (Immutable Object Database)
- **Breadth-First Search (BFS)** (Lowest Common Ancestor for Merging)
- **3-Way Merge Algorithm** (Conflict Resolution)
- **Rabin-Karp Rolling Hash** (Content-Defined Chunking)
- **Myers Shortest Edit Script Algorithm** (Dynamic Programming Diff Engine)
- **Mark-and-Sweep Graph Traversal** (Garbage Collection)
- **BLAKE3 Cryptographic Hashing** (High-throughput security)
- **Command Design Pattern** (Modular CLI Architecture)

## 🌟 Architectural Differences from Standard Git

If you are evaluating this project from a system design perspective, here is the core architectural shift from standard Git:

1. **Standard Git stores full-file snapshots:** Every time you commit, Git saves a brand new copy of the *entire* file. It only attempts to compress them later during a slow background process called packfiling.
2. **MiniGit uses Streaming Deduplication (CDC):** By utilizing a **Rabin-Karp rolling hash**, MiniGit dynamically slices files into chunks at the exact moment of staging. If you modify 1 single line in a 10MB file, MiniGit instantly reuses 99.9% of the existing chunks on disk and only saves the new 4KB chunk. 
3. **Cryptographic Speed:** Git relies on the outdated SHA-1 hashing algorithm. MiniGit upgrades the security and throughput bottlenecks by implementing **BLAKE3**, allowing it to hash file trees at ~70+ MB/s.

*Our automated benchmark suite proves that this CDC architecture achieves a **99.96% storage deduplication ratio** on modified files instantly upon commit!*

---

## 🚀 Core System Design Implementations

In addition to the architectural changes above, this project implements several core Computer Science algorithms from scratch:

### 1. Dynamic Programming (Myers Diff)
Instead of relying on external system libraries, MiniGit implements the industry-standard **Myers Shortest Edit Script Algorithm** from scratch. It operates in O(ND) time to calculate the absolute minimum number of insertions and deletions required to transition between two text files.

### 2. Graph Traversal (Garbage Collection)
To manage memory and prevent disk bloat from orphaned chunks, MiniGit features a custom **Mark-and-Sweep Garbage Collector**. It traverses the Directed Acyclic Graph (DAG) starting from the `HEAD` pointer and safely purges any unreachable objects from the database.

### 3. Object Storage (Blobs, Trees, Commits)
MiniGit leverages an immutable object database (a Directed Acyclic Graph) stored in the `.minigit/objects/` directory.
- **BLAKE3 Hashing**: Uses the state-of-the-art cryptographic hash function BLAKE3 (much faster than Git's standard SHA-1) to uniquely identify all objects.
- **ZLIB Compression**: Every object is tightly compressed using zlib before being written to disk to save space.

### 4. Content-Defined Chunking (CDC)
Unlike standard Git (which stores whole files), MiniGit slices files into smaller chunks using a **Rabin-Karp Rolling Hash**.
- As the window slides across the file's bytes, it deterministically cuts chunks (average size 4KB).
- If you insert a single byte at the beginning of a massive 10GB file, MiniGit only creates a new 4KB chunk, heavily deduplicating the remaining 9.9GB.
- File `Blob`s act as manifest files that point to the hashes of these chunks, which are then seamlessly stitched back together upon reading.

### 5. Branching, Merging & Time Travel
MiniGit fully supports non-linear development via branching, merging, and time-traveling.
- **Breadth-First Search (BFS)**: When merging two branches, MiniGit traverses the DAG backwards concurrently from both branch pointers to find the **Lowest Common Ancestor (LCA)**.
- **3-Way Merge**: It extracts the `Tree` objects of the LCA, HEAD, and Target branch, and evaluates every file using the Myers Diff Engine.
- **Conflict Detection**: Automatically detects if two branches modified the exact same file in conflicting ways, inserting standard `<<<<<<<`, `=======`, `>>>>>>>` markers.
- **Time Travel (`restore`)**: Pass any historical commit hash to the `restore` command to instantly traverse the DAG and perfectly reconstruct a specific directory from the past into your present workspace.

### 6. Staging Area & Modern CLI Commands
Built with a modular Command Pattern, MiniGit supports the standard workflows you expect from a VCS: staging files to an `Index`, wrapping them into `Tree` snapshots, tracking history with `Commit` objects, and standard observation commands (`log`, `status`, `cat-file`).
- **Hybrid Router**: The `checkout` command dynamically evaluates targets, acting as a router that either cleanly switches branches (via safety guards that prevent overwriting local work) or recursively restores files/directories.
- **Dedicated Branching**: Features dedicated `branch` creation and a modern, strict `switch` command.

**Supported Commands Quick-Reference:**
*   `minigit init [dir]` - Initialize a repository (creates directory if specified)
*   `minigit add <path>` - Stage a file or directory via CDC Chunking (supports `add .`, `add -A`, `add --all`)
*   `minigit commit -m "<msg>"` - Take a permanent snapshot with a required message
*   `minigit status` - View staging area
*   `minigit log` - View commit history (DAG traversal)
*   `minigit diff <file>` - Run Myers Diff Algorithm
*   `minigit branch [name]` - Create a branch, or list all branches if no name provided
*   `minigit switch <name>` - Safely switch active branch
*   `minigit checkout <target>` - Hybrid branch switch or file restore
*   `minigit merge <name>` - 3-Way merge via LCA BFS search
*   `minigit delete [name]` - Delete a specific branch, or wipe the repository
*   `minigit restore [<hash>] <path>` - Restore files (time-travel)
*   `minigit gc` - Garbage Collection via Mark-and-Sweep
*   `minigit cat-file [-p] <hash>` - Inspect/Decompress internal objects

### 7. DAG Visualizer
Included is a `scripts/visualize.py` script that reads the binary `.minigit/objects/` database, decompresses it, parses the relationships, and generates an interactive, graphical HTML flowchart (using Mermaid.js) of your entire repository architecture in real-time.

---

## 📖 Complete CLI Documentation

For a full breakdown of every command available, how to use them, and exactly how they function under the hood, please refer to the **[Command Reference Guide (COMMANDS.md)](COMMANDS.md)**.

## 🛠️ Build Instructions

### Prerequisites
- A modern C++ compiler (Clang/GCC) supporting C++17.
- CMake (version 3.14 or higher).

### Building from Source
```bash
# Generate the build system
cmake -B build

# Compile the executable and tests
cmake --build build
```
This produces the `minigit` CLI binary inside the `build/` directory, and a test executable named `minigit_tests`.

### Running Tests
The project uses GoogleTest for robust verification. Run the test suite using:
```bash
./build/minigit_tests
```

### Running Benchmarks
To generate statistical data for deduplication and performance (output to `benchmark_results.txt`), run the benchmark suite:
```bash
./build/minigit_benchmark
```
