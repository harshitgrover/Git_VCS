#pragma once
#include <string>

#include <map>
#include <vector>

using namespace std;

namespace minigit {
namespace utils {

string read_object_content(const string& gitdir, const string& hash);
string reconstruct_from_manifest(const string& gitdir, const string& manifest_data);

map<string, string> parse_tree(const string& gitdir, const string& tree_hash);
struct ParsedCommit {
    string tree_hash;
    vector<string> parents;
};
ParsedCommit parse_commit(const string& gitdir, const string& commit_hash);

} // namespace utils
} // namespace minigit
