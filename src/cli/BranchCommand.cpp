#include "BranchCommand.hpp"
#include "../core/Repository.hpp"
#include <iostream>

using namespace std;

namespace minigit {

void BranchCommand::execute(const vector<string>& args) {
    if (args.empty()) {
        cerr << "Usage: minigit branch <branch_name>\n";
        return;
    }
    
    Repository repo(".");
    repo.branch(args[0]);
}

} // namespace minigit
