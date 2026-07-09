#pragma once
#include "Command.h"

using namespace std;

namespace minigit {

class DiffCommand : public Command {
public:
    void execute(const vector<string>& args) override;
};

} // namespace minigit
