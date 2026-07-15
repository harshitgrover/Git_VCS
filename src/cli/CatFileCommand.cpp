#include "CatFileCommand.hpp"
#include "../core/Repository.hpp"
#include "../core/Utils.hpp"
#include <iostream>

using namespace std;

namespace minigit {

void CatFileCommand::execute(const vector<string>& args) {
    if (args.empty()) {
        cerr << "Usage: minigit cat-file [-p] <hash>\n";
        return;
    }
    
    string hash;
    bool reconstruct = false;
    
    if (args.size() == 2 && args[0] == "-p") {
        reconstruct = true;
        hash = args[1];
    } else {
        hash = args[0];
    }
    
    Repository repo(".");
    string gitdir = repo.getGitDir();
    
    string content = utils::read_object_content(gitdir, hash);
    if (content.empty()) {
        cerr << "fatal: Not a valid object name " << hash << "\n";
        return;
    }
    
    if (reconstruct) {
        content = utils::reconstruct_from_manifest(gitdir, content);
    }
    
    cout << content;
}

} // namespace minigit
