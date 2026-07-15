#include <iostream>
#include <chrono>
#include <vector>
#include <string>
#include <iomanip>
#include "core/Chunker.hpp"
#include "core/Diff.hpp"
#include <blake3.h>

using namespace std;
using namespace std::chrono;

void benchmark_blake3() {
    cout << "========================================\n";
    cout << "1. BLAKE3 Hashing Throughput Benchmark\n";
    cout << "========================================\n";
    
    // Generate 50 MB of data
    size_t data_size = 50 * 1024 * 1024;
    string data(data_size, 'a');
    
    auto start = high_resolution_clock::now();
    
    blake3_hasher hasher;
    blake3_hasher_init(&hasher);
    blake3_hasher_update(&hasher, data.c_str(), data.size());
    uint8_t output[BLAKE3_OUT_LEN];
    blake3_hasher_finalize(&hasher, output, BLAKE3_OUT_LEN);
    
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(end - start);
    
    double seconds = duration.count() / 1000000.0;
    double mb_per_sec = (50.0) / seconds;
    
    cout << "Data Size: 50 MB\n";
    cout << "Time taken: " << duration.count() << " microseconds (" << seconds << " seconds)\n";
    cout << "Throughput: " << fixed << setprecision(2) << mb_per_sec << " MB/s\n\n";
}

void benchmark_cdc_deduplication() {
    cout << "========================================\n";
    cout << "2. CDC Deduplication Benchmark\n";
    cout << "========================================\n";
    
    // Generate 10 MB of data with pseudo-random characters
    size_t data_size = 10 * 1024 * 1024;
    string data;
    data.reserve(data_size);
    srand(42); // fixed seed for reproducibility
    for (size_t i = 0; i < data_size; ++i) {
        data += (char)('A' + (rand() % 50));
    }
    
    auto start1 = high_resolution_clock::now();
    auto chunks_v1 = minigit::Chunker::chunk(data);
    auto end1 = high_resolution_clock::now();
    
    cout << "Chunking V1 (10 MB):\n";
    cout << "  Time taken: " << duration_cast<milliseconds>(end1 - start1).count() << " ms\n";
    cout << "  Total chunks created: " << chunks_v1.size() << "\n\n";
    
    // Modify exactly 1 byte in the middle of the 10 MB file
    string mod_data = data;
    mod_data[data_size / 2] = 'Z';
    
    auto start2 = high_resolution_clock::now();
    auto chunks_v2 = minigit::Chunker::chunk(mod_data);
    auto end2 = high_resolution_clock::now();
    
    cout << "Chunking V2 (10 MB with 1 byte modified):\n";
    cout << "  Time taken: " << duration_cast<milliseconds>(end2 - start2).count() << " ms\n";
    cout << "  Total chunks created: " << chunks_v2.size() << "\n\n";
    
    // Calculate deduplication
    size_t identical_chunks = 0;
    size_t new_chunks = 0;
    
    // We expect the chunks to be almost identical except for the one where 'Z' was inserted
    for (const auto& chunk2 : chunks_v2) {
        bool found = false;
        for (const auto& chunk1 : chunks_v1) {
            if (chunk1 == chunk2) {
                found = true;
                break;
            }
        }
        if (found) identical_chunks++;
        else new_chunks++;
    }
    
    double deduplication_ratio = (double)identical_chunks / chunks_v2.size() * 100.0;
    
    cout << "Deduplication Results:\n";
    cout << "  Reused chunks: " << identical_chunks << "\n";
    cout << "  New chunks (to save to disk): " << new_chunks << "\n";
    cout << "  Space saved: " << fixed << setprecision(2) << deduplication_ratio << "%\n\n";
}

void benchmark_myers_diff() {
    cout << "========================================\n";
    cout << "3. Myers Diff Algorithm Benchmark\n";
    cout << "========================================\n";
    
    // Generate 5,000 lines of code
    vector<string> old_lines;
    for (int i = 0; i < 5000; ++i) {
        old_lines.push_back("Line number " + to_string(i) + " of the original file.");
    }
    
    // Generate V2 with 50 insertions and 50 deletions
    vector<string> new_lines = old_lines;
    for (int i = 0; i < 50; ++i) {
        new_lines.erase(new_lines.begin() + (i * 90)); // Delete a line
        new_lines.insert(new_lines.begin() + (i * 95), "BRAND NEW INSERTED LINE " + to_string(i)); // Insert a line
    }
    
    minigit::MyersDiffStrategy diff;
    
    auto start = high_resolution_clock::now();
    auto edits = diff.compute(old_lines, new_lines);
    auto end = high_resolution_clock::now();
    
    auto duration = duration_cast<milliseconds>(end - start);
    
    cout << "Diffing 5,000 lines against 5,000 lines:\n";
    cout << "  Time taken: " << duration.count() << " ms\n";
    cout << "  Total Edit Operations calculated: " << edits.size() << "\n";
    cout << "========================================\n";
}

#include <fstream>

int main() {
    // Redirect std::cout to a file (overwrites it every time it runs)
    ofstream out("benchmark_results.txt");
    streambuf *coutbuf = cout.rdbuf(); // save old buffer
    cout.rdbuf(out.rdbuf()); // redirect std::cout to out.txt!

    cout << "\nStarting MiniGit Benchmark Suite...\n\n";
    benchmark_blake3();
    benchmark_cdc_deduplication();
    benchmark_myers_diff();
    cout << "Benchmarking Complete.\n\n";
    
    // Restore original cout
    cout.rdbuf(coutbuf);
    
    // Print a tiny message to the terminal so the user knows it finished
    std::cout << "Benchmark successfully completed! Results saved to benchmark_results.txt\n";
    return 0;
}
