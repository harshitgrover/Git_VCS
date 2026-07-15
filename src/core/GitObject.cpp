#include "GitObject.hpp"
#include <blake3.h>
#include <zlib.h>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>

using namespace std;
namespace fs = std::filesystem;

namespace minigit {

string GitObject::hash() const {
    string data = serialize();
    
    blake3_hasher hasher;
    blake3_hasher_init(&hasher);
    blake3_hasher_update(&hasher, data.data(), data.size());
    
    uint8_t output[BLAKE3_OUT_LEN];
    blake3_hasher_finalize(&hasher, output, BLAKE3_OUT_LEN);
    
    stringstream ss;
    for (int i = 0; i < BLAKE3_OUT_LEN; ++i) {
        ss << hex << setw(2) << setfill('0') << (int)output[i];
    }
    return ss.str();
}

string GitObject::write(const Repository& repo) const {
    string obj_hash = hash();
    string data = serialize();

    uLongf compressed_size = compressBound(data.size());
    string compressed(compressed_size, '\0');
    
    int res = compress(reinterpret_cast<Bytef*>(compressed.data()), &compressed_size,
                       reinterpret_cast<const Bytef*>(data.data()), data.size());
    
    if (res != Z_OK) {
        throw runtime_error("Failed to compress object");
    }
    compressed.resize(compressed_size);

    string dir_name = obj_hash.substr(0, 2);
    string file_name = obj_hash.substr(2);
    
    fs::path dir_path = fs::path(repo.getGitDir()) / "objects" / dir_name;
    fs::path file_path = dir_path / file_name;

    fs::create_directories(dir_path);
    
    if (!fs::exists(file_path)) {
        ofstream out(file_path, ios::binary);
        out.write(compressed.data(), compressed.size());
    }

    return obj_hash;
}

} // namespace minigit
