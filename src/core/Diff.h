#pragma once
#include <string>
#include <vector>

using namespace std;

namespace minigit {

enum class EditType {
    EQUAL,
    INSERT,
    DELETE
};

struct Edit {
    EditType type;
    string text;
};

class DiffStrategy {
public:
    virtual ~DiffStrategy() = default;
    virtual vector<Edit> compute(const vector<string>& old_lines, const vector<string>& new_lines) = 0;
};

class MyersDiffStrategy : public DiffStrategy {
public:
    vector<Edit> compute(const vector<string>& old_lines, const vector<string>& new_lines) override;
};

} // namespace minigit
