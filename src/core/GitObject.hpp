#pragma once
#include <string>
#include "Repository.hpp"

using namespace std;

namespace minigit {

class GitObject {
public:
    virtual ~GitObject() = default;

    virtual string serialize() const = 0;
    virtual string type() const = 0;

    string hash() const;
    virtual string write(const Repository& repo) const;
};

} // namespace minigit
