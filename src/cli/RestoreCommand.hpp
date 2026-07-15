#pragma once
#include "Command.hpp"

namespace minigit {

class RestoreCommand : public Command {
public:
    void execute(const vector<string>& args) override;
};

} // namespace minigit
