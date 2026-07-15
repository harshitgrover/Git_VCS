#include "SwitchCommand.hpp"
#include "../core/Repository.hpp"
#include <iostream>

using namespace std;

namespace minigit {

void SwitchCommand::execute(const vector<string>& args) {
    if (args.empty()) {
        cerr << "Usage: minigit switch <branch_name>\n";
        return;
    }
    
    Repository repo(".");
    repo.switch_branch(args[0]);
}

} // namespace minigit
