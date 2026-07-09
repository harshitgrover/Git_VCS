#pragma once
#include "Command.h"

namespace minigit {

class StatusCommand : public Command {
public:
    void execute(const std::vector<std::string>& args) override;
};

} // namespace minigit
