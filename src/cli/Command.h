#pragma once
#include <vector>
#include <string>

using namespace std;

namespace minigit {

class Command {
public:
    virtual ~Command() = default;
    virtual void execute(const vector<string>& args) = 0;
};

} // namespace minigit
