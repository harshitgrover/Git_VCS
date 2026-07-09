# MiniGit - A Custom Version Control System

MiniGit is a fully functional, highly optimized Version Control System written from scratch in modern C++ (C++17/20). It mirrors the core architecture of standard Git but introduces advanced storage optimizations like **Content-Defined Chunking (CDC)** for massive deduplication efficiency.

## 🚀 Key Features and Architecture

### 1. Object Storage (Blobs, Trees, Commits)
MiniGit leverages an immutable object database (a Directed Acyclic Graph) stored in the `.minigit/objects/` directory.
- **BLAKE3 Hashing**: Uses the state-of-the-art cryptographic hash function BLAKE3 (much faster than Git's standard SHA-1) to uniquely identify all objects.
- **ZLIB Compression**: Every object is tightly compressed using zlib before being written to disk to save space.

### 2. Content-Defined Chunking (CDC)
Unlike standard Git (which stores whole files), MiniGit slices files into smaller chunks using a **Rabin-Karp Rolling Hash**.
- As the window slides across the file's bytes, it deterministically cuts chunks (average size 4KB).
- If you insert a single byte at the beginning of a massive 10GB file, MiniGit only creates a new 4KB chunk, heavily deduplicating the remaining 9.9GB.
- File `Blob`s act as manifest files that point to the hashes of these chunks, which are then seamlessly stitched back together upon reading.

### 3. Myers Diff Engine
MiniGit implements the industry-standard **Myers Shortest Edit Script Algorithm**.
- Automatically computes the optimal sequences of inserts and deletes to transition between two text files.
- Operates in O(ND) time complexity, ensuring maximum performance when analyzing code changes.

### 4. Mark-and-Sweep Garbage Collection
Because CDC generates many granular chunk objects, staging files rapidly can leave orphaned chunks.
- The `gc` command performs a Mark-and-Sweep traversal across the entire commit graph, starting from branch pointers (like `HEAD`).
- It safely identifies and deletes orphaned/unreachable chunks, keeping the system clean.

### 5. Staging Area & CLI Commands
Built with a modular Command Pattern, MiniGit supports the standard workflows you expect from a VCS: staging files to an `Index`, wrapping them into `Tree` snapshots, tracking history with `Commit` objects, and standard observation commands (`log`, `status`, `cat-file`).

### 6. DAG Visualizer
Included is a `visualize.py` script that reads the binary `.minigit/objects/` database, decompresses it, parses the relationships, and generates an interactive, graphical HTML flowchart (using Mermaid.js) of your entire repository architecture in real-time.

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
