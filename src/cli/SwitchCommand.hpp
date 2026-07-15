#pragma once
#include "Command.hpp"

namespace minigit {

class SwitchCommand : public Command {
public:
    void execute(const vector<string>& args) override;
};

} // namespace minigit
