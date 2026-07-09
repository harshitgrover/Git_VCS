#include "Chunker.h"

using namespace std;

namespace minigit {

vector<string> Chunker::chunk(const string& data) {
    vector<string> chunks;
    if (data.empty()) return chunks;

    const size_t WINDOW_SIZE = 48;
    const uint32_t BASE = 31;
    const uint32_t MASK = 0x0FFF; // Average chunk size of 4096 bytes
    
    uint32_t power = 1;
    for (size_t i = 0; i < WINDOW_SIZE; ++i) {
        power *= BASE;
    }
    
    uint32_t hash = 0;
    size_t last_split = 0;
    
    for (size_t i = 0; i < data.size(); ++i) {
        hash = hash * BASE + (unsigned char)data[i];
        
        if (i >= WINDOW_SIZE) {
            hash -= (unsigned char)data[i - WINDOW_SIZE] * power;
        }
        
        if (i - last_split >= WINDOW_SIZE) {
            if ((hash & MASK) == 0 || i == data.size() - 1) {
                chunks.push_back(data.substr(last_split, i - last_split + 1));
                last_split = i + 1;
                hash = 0; 
            }
        } else if (i == data.size() - 1) {
            chunks.push_back(data.substr(last_split));
        }
    }
    
    return chunks;
}

} // namespace minigit
