#include "Utils.h"
#include <fstream>
#include <filesystem>
#include <sstream>
#include <zlib.h>

using namespace std;
namespace fs = std::filesystem;

namespace minigit {
namespace utils {

string read_object_content(const string& gitdir, const string& hash) {
    if (hash.length() < 4) return "";
    
    string dir_name = hash.substr(0, 2);
    string file_name = hash.substr(2);
    fs::path blob_path = fs::path(gitdir) / "objects" / dir_name / file_name;
    
    if (!fs::exists(blob_path)) return "";
    
    ifstream in(blob_path, ios::binary);
    string compressed((istreambuf_iterator<char>(in)), istreambuf_iterator<char>());
    
    string decompressed(compressed.size() * 4, '\0');
    uLongf dest_len = decompressed.size();
    
    while (true) {
        int res = uncompress(reinterpret_cast<Bytef*>(decompressed.data()), &dest_len,
                             reinterpret_cast<const Bytef*>(compressed.data()), compressed.size());
        if (res == Z_OK) {
            decompressed.resize(dest_len);
            break;
        } else if (res == Z_BUF_ERROR) {
            decompressed.resize(decompressed.size() * 2);
            dest_len = decompressed.size();
        } else {
            return "";
        }
    }
    
    return decompressed;
}

string reconstruct_from_manifest(const string& gitdir, const string& manifest_data) {
    size_t null_idx = manifest_data.find('\0');
    if (null_idx != string::npos) {
        string content = manifest_data.substr(null_idx + 1);
        if (content.substr(0, 6) == "chunk ") {
            string full_content = "";
            istringstream iss(content);
            string line;
            while (getline(iss, line)) {
                if (line.substr(0, 6) == "chunk ") {
                    string chunk_hash = line.substr(6);
                    full_content += read_object_content(gitdir, chunk_hash);
                }
            }
            return full_content;
        }
        return content;
    }
    return manifest_data;
}

} // namespace utils
} // namespace minigit
