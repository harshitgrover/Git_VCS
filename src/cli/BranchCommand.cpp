#include "BranchCommand.hpp"
#include "../core/Repository.hpp"
#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>

using namespace std;
namespace fs = std::filesystem;

namespace minigit {

void BranchCommand::execute(const vector<string>& args) {
    if (args.empty()) {
        Repository repo(".");
        string gitdir = repo.getGitDir();
        string heads_path = gitdir + "/refs/heads";
        
        string head_path = gitdir + "/HEAD";
        string current_branch = "";
        ifstream head_file(head_path);
        if (head_file.is_open()) {
            string line;
            getline(head_file, line);
            if (line.rfind("ref: refs/heads/", 0) == 0) {
                current_branch = line.substr(16);
            }
        }
        
        if (fs::exists(heads_path) && fs::is_directory(heads_path)) {
            for (const auto& entry : fs::directory_iterator(heads_path)) {
                string branch_name = entry.path().filename().string();
                if (branch_name == current_branch) {
                    cout << "* \033[32m" << branch_name << "\033[0m\n";
                } else {
                    cout << "  " << branch_name << "\n";
                }
            }
        }
        return;
    }
    
    Repository repo(".");
    repo.branch(args[0]);
}

} // namespace minigit
