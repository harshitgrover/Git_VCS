#pragma once
#include "Command.hpp"

namespace minigit {

class BranchCommand : public Command {
public:
    void execute(const vector<string>& args) override;
};

} // namespace minigit
