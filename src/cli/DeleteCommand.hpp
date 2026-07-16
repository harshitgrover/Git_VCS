#ifndef DELETE_COMMAND_HPP
#define DELETE_COMMAND_HPP

#include "Command.hpp"
#include <vector>
#include <string>

namespace minigit {

class DeleteCommand : public Command {
public:
    void execute(const std::vector<std::string>& args) override;
};

} // namespace minigit

#endif // DELETE_COMMAND_HPP
