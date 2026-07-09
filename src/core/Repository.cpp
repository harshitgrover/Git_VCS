#include "Repository.h"
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <iostream>
#include "Blob.h"
#include "Tree.h"
#include "Commit.h"
#include "Index.h"

using namespace std;
namespace fs = std::filesystem;

namespace minigit {

Repository::Repository(const string& path) {
    worktree_ = fs::absolute(path).string();
    gitdir_ = (fs::absolute(path) / ".minigit").string();
    
    if (!fs::exists(gitdir_) || !fs::is_directory(gitdir_)) {
        throw runtime_error("Not a minigit repository: " + path);
    }
}

Repository Repository::init(const string& path) {
    auto worktree = fs::absolute(path);
    auto gitdir = worktree / ".minigit";

    if (fs::exists(gitdir)) {
        throw runtime_error("Git directory already exists");
    }

    fs::create_directories(gitdir / "objects");
    fs::create_directories(gitdir / "refs" / "heads");

    ofstream headFile(gitdir / "HEAD");
    if (headFile.is_open()) {
        headFile << "ref: refs/heads/main\n";
    }

    cout << "Initialized empty minigit repository in " << gitdir.string() << "\n";
    
    return Repository(path);
}

void Repository::add(const string& file_path) {
    fs::path full_path = fs::absolute(file_path);
    if (!fs::exists(full_path)) {
        throw runtime_error("File not found: " + file_path);
    }
    
    ifstream in(full_path, ios::binary);
    string data((istreambuf_iterator<char>(in)), istreambuf_iterator<char>());
    
    Blob blob(data);
    string hash = blob.write(*this);
    
    Index index(gitdir_);
    
    string rel_path = fs::relative(full_path, worktree_).string();
    index.add(rel_path, hash);
    index.write();
}

void Repository::commit(const string& message) {
    Index index(gitdir_);
    if (index.getEntries().empty()) {
        throw runtime_error("Nothing to commit");
    }
    
    vector<TreeEntry> tree_entries;
    for (const auto& [path, hash] : index.getEntries()) {
        tree_entries.push_back({"100644", path, hash});
    }
    
    Tree tree(tree_entries);
    string tree_hash = tree.write(*this);
    
    string parent_hash = "";
    fs::path head_path = fs::path(gitdir_) / "HEAD";
    
    ifstream head_in(head_path);
    string ref_line;
    if (getline(head_in, ref_line)) {
        if (ref_line.find("ref: ") == 0) {
            string ref_path = ref_line.substr(5);
            fs::path real_ref_path = fs::path(gitdir_) / ref_path;
            if (fs::exists(real_ref_path)) {
                ifstream ref_in(real_ref_path);
                getline(ref_in, parent_hash);
            }
        } else {
            parent_hash = ref_line;
        }
    }
    
    Commit commit_obj(tree_hash, parent_hash, message);
    string commit_hash = commit_obj.write(*this);
    
    if (ref_line.find("ref: ") == 0) {
        string ref_path = ref_line.substr(5);
        ofstream ref_out(fs::path(gitdir_) / ref_path);
        ref_out << commit_hash << "\n";
    } else {
        ofstream head_out(head_path);
        head_out << commit_hash << "\n";
    }
    
    cout << "[" << commit_hash << "] " << message << "\n";
}

} // namespace minigit
