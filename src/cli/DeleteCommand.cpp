#include "DeleteCommand.hpp"
#include "../core/Repository.hpp"
#include <iostream>
#include <string>
#include <filesystem>
#include <fstream>

using namespace std;

namespace minigit {

void DeleteCommand::execute(const vector<string>& args) {
    if (args.empty()) {
        cout << "\033[31mWARNING: You are about to permanently delete the entire .minigit repository.\033[0m\n";
        cout << "Are you sure you want to proceed? (y/n): ";
        string response;
        if (getline(cin, response)) {
            if (response == "y" || response == "yes" || response == "Y" || response == "YES") {
                Repository repo(".");
                repo.destroy();
            } else {
                cout << "Deletion cancelled.\n";
            }
        }
    } else {
        string branch_name = args[0];
        Repository repo(".");
        
        std::filesystem::path target_branch_path = std::filesystem::path(repo.getGitDir()) / "refs" / "heads" / branch_name;
        if (!std::filesystem::exists(target_branch_path)) {
            throw runtime_error("Branch not found: " + branch_name);
        }
        
        std::filesystem::path head_path = std::filesystem::path(repo.getGitDir()) / "HEAD";
        if (std::filesystem::exists(head_path)) {
            ifstream head_in(head_path);
            string ref_line;
            if (getline(head_in, ref_line)) {
                if (ref_line == "ref: refs/heads/" + branch_name) {
                    throw runtime_error("can't delete current branch");
                }
            }
        }
        
        cout << "Are you sure you want to delete branch '" << branch_name << "'? (y/n): ";
        string response;
        if (getline(cin, response)) {
            if (response == "y" || response == "yes" || response == "Y" || response == "YES") {
                repo.delete_branch(branch_name);
            } else {
                cout << "Deletion cancelled.\n";
            }
        }
    }
}

} // namespace minigit
