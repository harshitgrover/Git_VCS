#pragma once
#include "Command.hpp"
#include <string>
#include <set>

using namespace std;

namespace minigit {

class GcCommand : public Command {
public:
    void execute(const vector<string>& args) override;

private:
    void mark_commit(const string& gitdir, const string& hash, set<string>& reachable);
    void mark_tree(const string& gitdir, const string& hash, set<string>& reachable);
    void mark_blob(const string& gitdir, const string& hash, set<string>& reachable);
};

} // namespace minigit
