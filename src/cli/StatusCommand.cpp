#include "StatusCommand.hpp"
#include "../core/Repository.hpp"
#include <iostream>
#include <fstream>
#include <filesystem>

using namespace std;
namespace fs = std::filesystem;

namespace minigit {

void StatusCommand::execute(const vector<string>& args) {
    Repository repo(".");
    string gitdir = repo.getGitDir();
    
    cout << "On branch main\n\n";
    
    fs::path index_path = fs::path(gitdir) / "index";
    if (fs::exists(index_path)) {
        cout << "Changes to be committed:\n";
        ifstream in(index_path);
        string line;
        while (getline(in, line)) {
            if (line.empty()) continue;
            size_t space_idx = line.find(' ');
            if (space_idx != string::npos) {
                string file = line.substr(space_idx + 1);
                cout << "  modified:   " << file << "\n";
            }
        }
        cout << "\n";
    } else {
        cout << "No commits yet\n\n";
    }
}

} // namespace minigit
