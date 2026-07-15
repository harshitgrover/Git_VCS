#include "MergeCommand.hpp"
#include "../core/Repository.hpp"
#include <iostream>

using namespace std;

namespace minigit {

void MergeCommand::execute(const vector<string>& args) {
    if (args.empty()) {
        cerr << "Usage: minigit merge <branch_name>\n";
        return;
    }
    
    Repository repo(".");
    repo.merge(args[0]);
}

} // namespace minigit
