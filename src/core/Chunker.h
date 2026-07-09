#pragma once
#include <string>
#include <vector>

using namespace std;

namespace minigit {

class Chunker {
public:
    static vector<string> chunk(const string& data);
};

} // namespace minigit
