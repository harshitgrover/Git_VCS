#pragma once
#include "GitObject.hpp"
#include <string>

using namespace std;

namespace minigit {

class Blob : public GitObject {
public:
    Blob(const string& content);

    string serialize() const override;
    string type() const override;
    string write(const class Repository& repo) const override;

private:
    string content_;
};

} // namespace minigit
