#include "Repository.hpp"
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <iostream>
#include "Blob.hpp"
#include "Tree.hpp"
#include "Commit.hpp"
#include "Index.hpp"
#include "Utils.hpp"
#include <queue>
#include <set>

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
    
    vector<string> parents;
    if (!parent_hash.empty()) {
        parents.push_back(parent_hash);
    }
    
    Commit commit_obj(tree_hash, parents, message);
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
} // End of commit method

void Repository::branch(const string& branch_name) {
    fs::path head_path = fs::path(gitdir_) / "HEAD";
    ifstream head_in(head_path);
    string ref_line;
    string current_hash = "";
    
    if (getline(head_in, ref_line)) {
        if (ref_line.find("ref: ") == 0) {
            string ref_path = ref_line.substr(5);
            fs::path real_ref_path = fs::path(gitdir_) / ref_path;
            if (fs::exists(real_ref_path)) {
                ifstream ref_in(real_ref_path);
                getline(ref_in, current_hash);
            }
        } else {
            current_hash = ref_line;
        }
    }
    
    if (current_hash.empty()) {
        throw runtime_error("Cannot create a branch from an empty repository. Commit something first.");
    }
    
    fs::path new_branch_path = fs::path(gitdir_) / "refs" / "heads" / branch_name;
    if (fs::exists(new_branch_path)) {
        throw runtime_error("Branch already exists: " + branch_name);
    }
    
    ofstream branch_out(new_branch_path);
    branch_out << current_hash << "\n";
    cout << "Created branch " << branch_name << "\n";
}

void Repository::switch_branch(const string& branch_name) {
    fs::path target_branch_path = fs::path(gitdir_) / "refs" / "heads" / branch_name;
    if (!fs::exists(target_branch_path)) {
        throw runtime_error("Branch not found: " + branch_name);
    }
    
    // Safety Check: Check for staged uncommitted changes
    fs::path head_path = fs::path(gitdir_) / "HEAD";
    string head_hash = "";
    if (fs::exists(head_path)) {
        ifstream head_in(head_path);
        string ref_line;
        if (getline(head_in, ref_line)) {
            if (ref_line.find("ref: ") == 0) {
                fs::path real_ref_path = fs::path(gitdir_) / ref_line.substr(5);
                if (fs::exists(real_ref_path)) {
                    ifstream ref_in(real_ref_path);
                    getline(ref_in, head_hash);
                }
            } else {
                head_hash = ref_line;
            }
        }
    }
    
    Index index(gitdir_);
    auto current_index_entries = index.getEntries();
    
    if (!head_hash.empty()) {
        utils::ParsedCommit head_parsed = utils::parse_commit(gitdir_, head_hash);
        map<string, string> head_tree = utils::parse_tree(gitdir_, head_parsed.tree_hash);
        
        bool is_dirty = false;
        if (current_index_entries.size() != head_tree.size()) {
            is_dirty = true;
        } else {
            for (const auto& [path, hash] : current_index_entries) {
                if (head_tree.find(path) == head_tree.end() || head_tree[path] != hash) {
                    is_dirty = true;
                    break;
                }
            }
        }
        
        if (is_dirty) {
            throw runtime_error("Your local changes would be overwritten by checkout. Please commit your changes.");
        }
    }
    
    ifstream t_in(target_branch_path);
    string commit_hash;
    getline(t_in, commit_hash);
    
    utils::ParsedCommit parsed = utils::parse_commit(gitdir_, commit_hash);
    map<string, string> tree = utils::parse_tree(gitdir_, parsed.tree_hash);
    
    for (const auto& [path, hash] : tree) {
        string raw_manifest = utils::read_object_content(gitdir_, hash);
        string file_content = utils::reconstruct_from_manifest(gitdir_, raw_manifest);
        
        fs::path file_dest = fs::path(worktree_) / path;
        fs::create_directories(file_dest.parent_path());
        ofstream out(file_dest, ios::binary);
        out << file_content;
    }
    
    ofstream head_out(fs::path(gitdir_) / "HEAD");
    head_out << "ref: refs/heads/" << branch_name << "\n";
    cout << "Switched to branch '" << branch_name << "'\n";
}

void Repository::checkout(const string& target) {
    fs::path target_branch_path = fs::path(gitdir_) / "refs" / "heads" / target;
    if (fs::exists(target_branch_path)) {
        switch_branch(target);
    } else {
        restore(target);
    }
}

void Repository::restore(const string& path, const string& source_hash) {
    string target_hash = source_hash;
    
    // If no specific hash is provided, fallback to HEAD
    if (target_hash.empty()) {
        fs::path head_path = fs::path(gitdir_) / "HEAD";
        if (fs::exists(head_path)) {
            ifstream head_in(head_path);
            string ref_line;
            if (getline(head_in, ref_line)) {
                if (ref_line.find("ref: ") == 0) {
                    fs::path real_ref_path = fs::path(gitdir_) / ref_line.substr(5);
                    if (fs::exists(real_ref_path)) {
                        ifstream ref_in(real_ref_path);
                        getline(ref_in, target_hash);
                    }
                } else {
                    target_hash = ref_line;
                }
            }
        }
    }
    
    if (target_hash.empty()) {
        throw runtime_error("No commits yet, cannot restore.");
    }
    
    utils::ParsedCommit parsed = utils::parse_commit(gitdir_, target_hash);
    map<string, string> tree = utils::parse_tree(gitdir_, parsed.tree_hash);
    
    bool found = false;
    for (const auto& [tree_path, hash] : tree) {
        // If it's an exact file match, falls under the requested directory, or path is "." (restore everything)
        if (path == "." || tree_path == path || tree_path.find(path + "/") == 0) {
            found = true;
            string raw_manifest = utils::read_object_content(gitdir_, hash);
            string file_content = utils::reconstruct_from_manifest(gitdir_, raw_manifest);
            
            fs::path file_dest = fs::path(worktree_) / tree_path;
            fs::create_directories(file_dest.parent_path());
            ofstream out(file_dest, ios::binary);
            out << file_content;
            
            cout << "Restored '" << tree_path << "'\n";
        }
    }
    
    if (!found) {
        throw runtime_error("Path '" + path + "' did not match any files in HEAD commit");
    }
}

void Repository::merge(const string& branch_name) {
    fs::path target_branch_path = fs::path(gitdir_) / "refs" / "heads" / branch_name;
    if (!fs::exists(target_branch_path)) {
        throw runtime_error("Branch not found: " + branch_name);
    }
    
    ifstream t_in(target_branch_path);
    string target_hash;
    getline(t_in, target_hash);
    
    fs::path head_path = fs::path(gitdir_) / "HEAD";
    ifstream head_in(head_path);
    string ref_line;
    string head_hash = "";
    string current_branch = "";
    if (getline(head_in, ref_line)) {
        if (ref_line.find("ref: ") == 0) {
            current_branch = ref_line.substr(16); // "ref: refs/heads/..."
            string ref_path = ref_line.substr(5);
            fs::path real_ref_path = fs::path(gitdir_) / ref_path;
            if (fs::exists(real_ref_path)) {
                ifstream ref_in(real_ref_path);
                getline(ref_in, head_hash);
            }
        }
    }
    
    if (head_hash.empty()) throw runtime_error("Invalid HEAD");

    // 1. BFS to find LCA
    queue<string> qA, qB;
    set<string> visA, visB;
    qA.push(head_hash);
    qB.push(target_hash);
    
    string lca_hash = "";
    while(!qA.empty() || !qB.empty()) {
        if (!qA.empty()) {
            string curr = qA.front(); qA.pop();
            visA.insert(curr);
            if (visB.count(curr)) { lca_hash = curr; break; }
            auto parsed = utils::parse_commit(gitdir_, curr);
            for (const auto& p : parsed.parents) qA.push(p);
        }
        if (!qB.empty()) {
            string curr = qB.front(); qB.pop();
            visB.insert(curr);
            if (visA.count(curr)) { lca_hash = curr; break; }
            auto parsed = utils::parse_commit(gitdir_, curr);
            for (const auto& p : parsed.parents) qB.push(p);
        }
    }
    
    if (lca_hash.empty()) throw runtime_error("No common ancestor found (Repositories have completely distinct histories)");
    
    // 2. Parse Trees
    auto head_commit = utils::parse_commit(gitdir_, head_hash);
    auto target_commit = utils::parse_commit(gitdir_, target_hash);
    auto lca_commit = utils::parse_commit(gitdir_, lca_hash);
    
    auto tree_head = utils::parse_tree(gitdir_, head_commit.tree_hash);
    auto tree_target = utils::parse_tree(gitdir_, target_commit.tree_hash);
    auto tree_lca = utils::parse_tree(gitdir_, lca_commit.tree_hash);
    
    // 3. 3-Way Merge Evaluation
    set<string> all_files;
    for (const auto& [n, h] : tree_head) all_files.insert(n);
    for (const auto& [n, h] : tree_target) all_files.insert(n);
    for (const auto& [n, h] : tree_lca) all_files.insert(n);
    
    bool conflict = false;
    
    for (const string& file : all_files) {
        string hO = tree_lca.count(file) ? tree_lca[file] : "";
        string hA = tree_head.count(file) ? tree_head[file] : "";
        string hB = tree_target.count(file) ? tree_target[file] : "";
        
        if (hA == hB) continue; 
        
        if (hO == hA && hO != hB) {
            string content = utils::reconstruct_from_manifest(gitdir_, utils::read_object_content(gitdir_, hB));
            ofstream out(fs::path(worktree_) / file, ios::binary);
            out << content;
            add(file); 
            continue;
        }
        if (hO != hA && hO == hB) {
            continue;
        }
        
        conflict = true;
        
        string strA = hA.empty() ? "" : utils::reconstruct_from_manifest(gitdir_, utils::read_object_content(gitdir_, hA));
        string strB = hB.empty() ? "" : utils::reconstruct_from_manifest(gitdir_, utils::read_object_content(gitdir_, hB));
        
        string merged = "<<<<<<< HEAD\n" + strA + "\n=======\n" + strB + "\n>>>>>>> " + branch_name + "\n";
        ofstream out(fs::path(worktree_) / file, ios::binary);
        out << merged;
        cout << "CONFLICT (content): Merge conflict in " << file << "\n";
    }
    
    if (conflict) {
        cout << "Automatic merge failed; fix conflicts and then commit the result.\n";
    } else {
        string message = "Merge branch '" + branch_name + "' into " + current_branch;
        
        Index index(gitdir_);
        vector<TreeEntry> tree_entries;
        for (const auto& [path, hash] : index.getEntries()) {
            tree_entries.push_back({"100644", path, hash});
        }
        Tree tree(tree_entries);
        string new_tree_hash = tree.write(*this);
        
        vector<string> parents = {head_hash, target_hash};
        Commit commit_obj(new_tree_hash, parents, message);
        string commit_hash = commit_obj.write(*this);
        
        fs::path real_ref_path = fs::path(gitdir_) / "refs" / "heads" / current_branch;
        ofstream ref_out(real_ref_path);
        ref_out << commit_hash << "\n";
        
        cout << "Merge successful. Created merge commit [" << commit_hash << "].\n";
    }
}

} // namespace minigit
