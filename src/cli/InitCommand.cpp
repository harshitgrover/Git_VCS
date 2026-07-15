#include "InitCommand.hpp"
#include "../core/Repository.hpp"

using namespace std;

namespace minigit {

void InitCommand::execute(const vector<string>& args) {
    string path = args.empty() ? "." : args[0];
    Repository::init(path);
}

} // namespace minigit
