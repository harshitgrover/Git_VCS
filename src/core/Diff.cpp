#include "Diff.h"
#include <algorithm>

using namespace std;

namespace minigit {

vector<Edit> MyersDiffStrategy::compute(const vector<string>& old_lines, const vector<string>& new_lines) {
    int n = old_lines.size();
    int m = new_lines.size();
    int max_d = n + m;
    
    if (n == 0 && m == 0) return {};
    
    vector<int> v(2 * max_d + 1, 0);
    vector<vector<int>> trace;
    
    for (int d = 0; d <= max_d; ++d) {
        trace.push_back(v);
        for (int k = -d; k <= d; k += 2) {
            int x, y;
            if (k == -d || (k != d && v[max_d + k - 1] < v[max_d + k + 1])) {
                x = v[max_d + k + 1];
            } else {
                x = v[max_d + k - 1] + 1;
            }
            y = x - k;
            
            while (x < n && y < m && old_lines[x] == new_lines[y]) {
                x++;
                y++;
            }
            
            v[max_d + k] = x;
            
            if (x >= n && y >= m) {
                vector<Edit> edits;
                int curr_x = n;
                int curr_y = m;
                
                for (int td = d; td >= 0; --td) {
                    const auto& tv = trace[td];
                    int tk = curr_x - curr_y;
                    
                    int prev_x, prev_y, prev_k;
                    if (td == 0) {
                        prev_x = 0;
                        prev_y = 0;
                    } else if (tk == -td || (tk != td && tv[max_d + tk - 1] < tv[max_d + tk + 1])) {
                        prev_k = tk + 1;
                        prev_x = tv[max_d + prev_k];
                        prev_y = prev_x - prev_k;
                    } else {
                        prev_k = tk - 1;
                        prev_x = tv[max_d + prev_k];
                        prev_y = prev_x - prev_k;
                    }
                    
                    while (curr_x > prev_x && curr_y > prev_y) {
                        curr_x--;
                        curr_y--;
                        edits.push_back({EditType::EQUAL, old_lines[curr_x]});
                    }
                    
                    if (td > 0) {
                        if (curr_x == prev_x) {
                            curr_y--;
                            edits.push_back({EditType::INSERT, new_lines[curr_y]});
                        } else {
                            curr_x--;
                            edits.push_back({EditType::DELETE, old_lines[curr_x]});
                        }
                    }
                }
                reverse(edits.begin(), edits.end());
                return edits;
            }
        }
    }
    return {};
}

} // namespace minigit
