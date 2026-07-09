#pragma once
#include "Command.h"
#include <string>

using namespace std;

namespace minigit {

class LogCommand : public Command {
public:
    void execute(const vector<string>& args) override;
};

} // namespace minigit
