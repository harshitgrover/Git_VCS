#include "CommitCommand.h"
#include "../core/Repository.h"
#include <iostream>

using namespace std;

namespace minigit {

void CommitCommand::execute(const vector<string>& args) {
    if (args.size() < 2 || args[0] != "-m") {
        cerr << "Usage: minigit commit -m \"<message>\"\n";
        return;
    }
    
    Repository repo(".");
    repo.commit(args[1]);
}

} // namespace minigit
