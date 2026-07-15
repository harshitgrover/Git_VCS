#include "CheckoutCommand.hpp"
#include "../core/Repository.hpp"
#include <iostream>

using namespace std;

namespace minigit {

void CheckoutCommand::execute(const vector<string>& args) {
    if (args.empty()) {
        cerr << "Usage: minigit checkout <branch_name>\n";
        return;
    }
    
    Repository repo(".");
    repo.checkout(args[0]);
}

} // namespace minigit
