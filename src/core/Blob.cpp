#include "Blob.hpp"
#include "Chunker.hpp"
#include "Repository.hpp"
#include <blake3.h>
#include <zlib.h>
#include <filesystem>
#include <fstream>

using namespace std;
namespace fs = std::filesystem;

namespace minigit {

Blob::Blob(const string& content) : content_(content) {}

string Blob::type() const {
    return "blob";
}

string Blob::serialize() const {
    return content_;
}


string Blob::write(const Repository& repo) const {
    auto chunks = Chunker::chunk(content_);
    string manifest_content;
    
    for (const auto& chunk : chunks) {
        blake3_hasher hasher;
        blake3_hasher_init(&hasher);
        blake3_hasher_update(&hasher, chunk.c_str(), chunk.size());
        uint8_t output[BLAKE3_OUT_LEN];
        blake3_hasher_finalize(&hasher, output, BLAKE3_OUT_LEN);
        
        char hex_hash[BLAKE3_OUT_LEN * 2 + 1];
        for (size_t i = 0; i < BLAKE3_OUT_LEN; ++i) {
            snprintf(hex_hash + 2 * i, 3, "%02x", output[i]);
        }
        string chunk_hash(hex_hash);
        
        string dir_name = chunk_hash.substr(0, 2);
        string file_name = chunk_hash.substr(2);
        fs::path dir_path = fs::path(repo.getGitDir()) / "objects" / dir_name;
        fs::create_directories(dir_path);
        
        fs::path obj_path = dir_path / file_name;
        if (!fs::exists(obj_path)) {
            uLongf compressed_size = compressBound(chunk.size());
            string compressed(compressed_size, '\0');
            compress(reinterpret_cast<Bytef*>(compressed.data()), &compressed_size,
                     reinterpret_cast<const Bytef*>(chunk.data()), chunk.size());
            compressed.resize(compressed_size);
            
            ofstream out(obj_path, ios::binary);
            out.write(compressed.data(), compressed.size());
        }
        
        manifest_content += "chunk " + chunk_hash + "\n";
    }
    
    string full_data = type() + " " + to_string(manifest_content.size()) + '\0' + manifest_content;
    
    blake3_hasher hasher;
    blake3_hasher_init(&hasher);
    blake3_hasher_update(&hasher, full_data.c_str(), full_data.size());
    uint8_t output[BLAKE3_OUT_LEN];
    blake3_hasher_finalize(&hasher, output, BLAKE3_OUT_LEN);
    
    char hex_hash[BLAKE3_OUT_LEN * 2 + 1];
    for (size_t i = 0; i < BLAKE3_OUT_LEN; ++i) {
        snprintf(hex_hash + 2 * i, 3, "%02x", output[i]);
    }
    string manifest_hash(hex_hash);
    
    string dir_name = manifest_hash.substr(0, 2);
    string file_name = manifest_hash.substr(2);
    fs::path dir_path = fs::path(repo.getGitDir()) / "objects" / dir_name;
    fs::create_directories(dir_path);
    
    fs::path obj_path = dir_path / file_name;
    if (!fs::exists(obj_path)) {
        uLongf compressed_size = compressBound(full_data.size());
        string compressed(compressed_size, '\0');
        compress(reinterpret_cast<Bytef*>(compressed.data()), &compressed_size,
                 reinterpret_cast<const Bytef*>(full_data.data()), full_data.size());
        compressed.resize(compressed_size);
        
        ofstream out(obj_path, ios::binary);
        out.write(compressed.data(), compressed.size());
    }
    
    return manifest_hash;
}

} // namespace minigit
