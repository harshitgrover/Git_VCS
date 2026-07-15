#include "RestoreCommand.hpp"
#include "../core/Repository.hpp"
#include <iostream>

using namespace std;

namespace minigit {

void RestoreCommand::execute(const vector<string>& args) {
    if (args.empty() || args.size() > 2) {
        cerr << "Usage: minigit restore [<commit_hash>] <filepath>\n";
        return;
    }
    
    Repository repo(".");
    
    if (args.size() == 1) {
        // minigit restore <filepath>
        repo.restore(args[0]);
    } else {
        // minigit restore <commit_hash> <filepath>
        repo.restore(args[1], args[0]);
    }
}

} // namespace minigit
