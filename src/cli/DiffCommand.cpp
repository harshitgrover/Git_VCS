#include "DiffCommand.hpp"
#include "../core/Repository.hpp"
#include "../core/Index.hpp"
#include "../core/Utils.hpp"
#include "../core/Diff.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <zlib.h>

using namespace std;
namespace fs = std::filesystem;

namespace minigit {

vector<string> read_lines(const string& content) {
    vector<string> lines;
    string line;
    for (char c : content) {
        if (c == '\n') {
            lines.push_back(line);
            line.clear();
        } else {
            line += c;
        }
    }
    if (!line.empty()) lines.push_back(line);
    return lines;
}

string read_blob_content(const string& gitdir, const string& hash) {
    string raw_data = utils::read_object_content(gitdir, hash);
    return utils::reconstruct_from_manifest(gitdir, raw_data);
}

void DiffCommand::execute(const vector<string>& args) {
    if (args.empty()) {
        cerr << "Usage: minigit diff <file>\n";
        return;
    }
    
    string file_path = args[0];
    Repository repo(".");
    Index index(repo.getWorktree());
    
    string old_content = "";
    auto entries = index.getEntries();
    if (entries.find(file_path) != entries.end()) {
        string hash = entries[file_path];
        old_content = read_blob_content(repo.getGitDir(), hash);
    }
    
    string new_content = "";
    fs::path full_path = fs::path(repo.getWorktree()) / file_path;
    if (fs::exists(full_path)) {
        ifstream in(full_path);
        new_content = string((istreambuf_iterator<char>(in)), istreambuf_iterator<char>());
    }
    
    auto old_lines = read_lines(old_content);
    auto new_lines = read_lines(new_content);
    
    MyersDiffStrategy diff_strategy;
    auto edits = diff_strategy.compute(old_lines, new_lines);
    
    int old_idx = 1;
    int new_idx = 1;
    
    vector<bool> visible(edits.size(), false);
    for (size_t i = 0; i < edits.size(); ++i) {
        if (edits[i].type != EditType::EQUAL) {
            for (int j = max(0, (int)i - 3); j <= min((int)edits.size() - 1, (int)i + 3); ++j) {
                visible[j] = true;
            }
        }
    }
    
    size_t i = 0;
    while (i < edits.size()) {
        if (!visible[i]) {
            if (edits[i].type == EditType::EQUAL || edits[i].type == EditType::DELETE) old_idx++;
            if (edits[i].type == EditType::EQUAL || edits[i].type == EditType::INSERT) new_idx++;
            i++;
            continue;
        }
        
        size_t start_i = i;
        int old_start = old_idx;
        int new_start = new_idx;
        int old_len = 0;
        int new_len = 0;
        
        while (i < edits.size() && visible[i]) {
            if (edits[i].type == EditType::EQUAL || edits[i].type == EditType::DELETE) old_len++;
            if (edits[i].type == EditType::EQUAL || edits[i].type == EditType::INSERT) new_len++;
            i++;
        }
        
        cout << "\033[36m@@ -" << old_start << "," << old_len << " +" << new_start << "," << new_len << " @@\033[0m\n";
        
        for (size_t j = start_i; j < i; ++j) {
            if (edits[j].type == EditType::EQUAL) {
                cout << "  " << edits[j].text << "\n";
            } else if (edits[j].type == EditType::INSERT) {
                cout << "\033[32m+ " << edits[j].text << "\033[0m\n";
            } else if (edits[j].type == EditType::DELETE) {
                cout << "\033[31m- " << edits[j].text << "\033[0m\n";
            }
        }
        
        old_idx += old_len;
        new_idx += new_len;
    }
}

} // namespace minigit
