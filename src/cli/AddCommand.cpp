#include "AddCommand.hpp"
#include "../core/Repository.hpp"
#include <iostream>

using namespace std;

namespace minigit {

void AddCommand::execute(const vector<string>& args) {
    if (args.empty()) {
        cerr << "Usage: minigit add <file1> <file2> ...\n";
        return;
    }
    
    Repository repo(".");
    for (const auto& file : args) {
        if (file == "-A" || file == "--all") {
            repo.add(repo.getWorktree());
        } else {
            repo.add(file);
        }
    }
}

} // namespace minigit
