#pragma once
#include <string>

using namespace std;

namespace minigit {

class Repository {
public:
    Repository(const string& path);

    static Repository init(const string& path);

    void add(const string& file_path);
    void commit(const string& message);
    void branch(const string& branch_name);
    void checkout(const string& target);
    void switch_branch(const string& branch_name);
    void merge(const string& branch_name);
    void restore(const string& path, const string& source_hash = "");
    void delete_branch(const string& branch_name);
    void destroy();

    string getWorktree() const { return worktree_; }
    string getGitDir() const { return gitdir_; }

private:
    string worktree_;
    string gitdir_;
};

} // namespace minigit
