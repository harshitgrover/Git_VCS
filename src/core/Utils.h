#pragma once
#include <string>

using namespace std;

namespace minigit {
namespace utils {

string read_object_content(const string& gitdir, const string& hash);
string reconstruct_from_manifest(const string& gitdir, const string& manifest_data);

} // namespace utils
} // namespace minigit
