#include "LogCommand.hpp"
#include "../core/Repository.hpp"
#include "../core/Utils.hpp"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <sstream>

using namespace std;
namespace fs = std::filesystem;

namespace minigit {

void LogCommand::execute(const vector<string>& args) {
    Repository repo(".");
    string gitdir = repo.getGitDir();
    
    fs::path head_path = fs::path(gitdir) / "HEAD";
    if (!fs::exists(head_path)) {
        cerr << "fatal: not a minigit repository\n";
        return;
    }
    
    string current_hash;
    ifstream head_in(head_path);
    string line;
    if (getline(head_in, line)) {
        if (line.substr(0, 5) == "ref: ") {
            fs::path ref_path = fs::path(gitdir) / line.substr(5);
            if (fs::exists(ref_path)) {
                ifstream ref_in(ref_path);
                getline(ref_in, current_hash);
            } else {
                cerr << "fatal: your current branch does not have any commits yet\n";
                return;
            }
        } else {
            current_hash = line;
        }
    }
    
    while (!current_hash.empty()) {
        string content = utils::read_object_content(gitdir, current_hash);
        if (content.empty()) break;
        
        size_t null_idx = content.find('\0');
        if (null_idx == string::npos) break;
        
        string body = content.substr(null_idx + 1);
        istringstream iss(body);
        string commit_line;
        
        string parent_hash = "";
        
        cout << "commit " << current_hash << "\n";
        
        while (getline(iss, commit_line)) {
            if (commit_line.empty()) break;
            if (commit_line.substr(0, 7) == "parent ") {
                parent_hash = commit_line.substr(7);
            } else if (commit_line.substr(0, 7) == "author ") {
                size_t ts_start = commit_line.find("> ");
                if (ts_start != string::npos) {
                    string timestamp = commit_line.substr(ts_start + 2);
                    size_t space = timestamp.find(' ');
                    if (space != string::npos) timestamp = timestamp.substr(0, space);
                    
                    try {
                        time_t ts = stoll(timestamp);
                        char date_buf[100];
                        struct tm* tm_info = localtime(&ts);
                        strftime(date_buf, sizeof(date_buf), "Date:   %a %b %d %H:%M:%S %Y %z", tm_info);
                        cout << "Author: " << commit_line.substr(7, ts_start - 6) << "\n";
                        cout << date_buf << "\n";
                    } catch(...) {}
                }
            }
        }
        
        string message = "";
        string msg_line;
        while (getline(iss, msg_line)) {
            message += "    " + msg_line + "\n";
        }
        
        cout << "\n" << message << "\n";
        current_hash = parent_hash;
    }
}

} // namespace minigit
