#include "AddCommand.h"
#include "../core/Repository.h"
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
        repo.add(file);
    }
}

} // namespace minigit
