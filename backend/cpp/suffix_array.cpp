/*
 * suffix_array.cpp
 * Suffix Array + LCP Array — Longest Common Substring Detection
 *
 * Implements:
 *   - Suffix array construction (O(n log^2 n))
 *   - LCP array computation (Kasai's algorithm)
 *   - Longest common substring between two texts
 *   - Longest shared contiguous passages detection
 *
 * Usage: suffix_array.exe <file_a> <file_b>
 * Output: JSON with longest common substrings, positions, structure
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <chrono>
#include <numeric>
#include <set>

using namespace std;

// ===================== UTILITY FUNCTIONS =====================
string readFile(const string& filepath) {
    ifstream file(filepath);
    if (!file.is_open()) {
        cerr << "Error: Cannot open file: " << filepath << endl;
        return "";
    }
    stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

string toLowercase(const string& text) {
    string result = text;
    for (char& c : result) {
        if (c >= 'A' && c <= 'Z') c = c + 32;
    }
    return result;
}

string cleanText(const string& text) {
    string result;
    result.reserve(text.size());
    for (char c : text) {
        if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == ' ') {
            result += c;
        } else if (c == '\n' || c == '\t') {
            result += ' ';
        }
    }
    return result;
}

string jsonEscape(const string& s) {
    string result;
    result.reserve(s.size() + 10);
    for (char c : s) {
        switch (c) {
            case '"':  result += "\\\""; break;
            case '\\': result += "\\\\"; break;
            case '\n': result += "\\n"; break;
            case '\r': result += "\\r"; break;
            case '\t': result += "\\t"; break;
            default:   result += c;
        }
    }
    return result;
}

// ===================== SUFFIX ARRAY CONSTRUCTION =====================
// Build suffix array using prefix-doubling (O(n log^2 n))
vector<int> buildSuffixArray(const string& s) {
    int n = s.size();
    vector<int> sa(n), rank_(n), tmp(n);

    // Initialize: sort suffixes by first character
    iota(sa.begin(), sa.end(), 0);
    for (int i = 0; i < n; i++) rank_[i] = s[i];

    for (int gap = 1; gap < n; gap *= 2) {
        // Comparator using pair of ranks
        auto cmp = [&](int a, int b) {
            if (rank_[a] != rank_[b]) return rank_[a] < rank_[b];
            int ra = (a + gap < n) ? rank_[a + gap] : -1;
            int rb = (b + gap < n) ? rank_[b + gap] : -1;
            return ra < rb;
        };

        sort(sa.begin(), sa.end(), cmp);

        // Compute new ranks
        tmp[sa[0]] = 0;
        for (int i = 1; i < n; i++) {
            tmp[sa[i]] = tmp[sa[i - 1]] + (cmp(sa[i - 1], sa[i]) ? 1 : 0);
        }
        rank_ = tmp;

        // If all ranks are unique, we're done
        if (rank_[sa[n - 1]] == n - 1) break;
    }

    return sa;
}

// ===================== LCP ARRAY (KASAI'S ALGORITHM) =====================
vector<int> buildLCPArray(const string& s, const vector<int>& sa) {
    int n = s.size();
    vector<int> rank_(n), lcp(n, 0);

    // Compute inverse suffix array (rank)
    for (int i = 0; i < n; i++) rank_[sa[i]] = i;

    int h = 0;
    for (int i = 0; i < n; i++) {
        if (rank_[i] > 0) {
            int j = sa[rank_[i] - 1];
            while (i + h < n && j + h < n && s[i + h] == s[j + h]) {
                h++;
            }
            lcp[rank_[i]] = h;
            if (h > 0) h--;
        } else {
            h = 0;
        }
    }

    return lcp;
}

// ===================== FIND LONGEST COMMON SUBSTRINGS =====================
struct LCSResult {
    string substring;
    int posInA;
    int posInB;
    int length;
};

vector<LCSResult> findLongestCommonSubstrings(const string& textA, const string& textB, int topK = 10) {
    int lenA = textA.size();
    int lenB = textB.size();

    // Concatenate: textA + '#' + textB + '$'
    // Use '#' as separator (not in either text after cleaning)
    string combined = textA + '#' + textB + '$';
    int n = combined.size();

    // Build suffix array
    vector<int> sa = buildSuffixArray(combined);

    // Build LCP array
    vector<int> lcp = buildLCPArray(combined, sa);

    // Find common substrings between A and B
    // A suffix belongs to A if sa[i] < lenA
    // A suffix belongs to B if sa[i] > lenA (after separator)
    vector<LCSResult> results;

    for (int i = 1; i < n; i++) {
        bool prevInA = sa[i - 1] < lenA;
        bool currInA = sa[i] < lenA;
        bool prevInB = sa[i - 1] > lenA;
        bool currInB = sa[i] > lenA;

        // Adjacent suffixes from different documents
        if ((prevInA && currInB) || (prevInB && currInA)) {
            if (lcp[i] >= 5) { // Minimum meaningful length
                LCSResult r;
                r.length = lcp[i];
                r.substring = combined.substr(sa[i], min(lcp[i], 200));

                if (prevInA) {
                    r.posInA = sa[i - 1];
                    r.posInB = sa[i] - lenA - 1;
                } else {
                    r.posInA = sa[i];
                    r.posInB = sa[i - 1] - lenA - 1;
                }

                results.push_back(r);
            }
        }
    }

    // Sort by length descending
    sort(results.begin(), results.end(), [](const LCSResult& a, const LCSResult& b) {
        return a.length > b.length;
    });

    // Remove overlapping results, keep top K
    vector<LCSResult> filtered;
    set<int> coveredA, coveredB;

    for (const LCSResult& r : results) {
        bool overlap = false;
        for (int k = r.posInA; k < r.posInA + min(r.length, 20); k++) {
            if (coveredA.count(k)) { overlap = true; break; }
        }
        if (!overlap) {
            for (int k = r.posInB; k < r.posInB + min(r.length, 20); k++) {
                if (coveredB.count(k)) { overlap = true; break; }
            }
        }

        if (!overlap && r.posInB >= 0) {
            filtered.push_back(r);
            for (int k = r.posInA; k < r.posInA + r.length; k++) coveredA.insert(k);
            for (int k = r.posInB; k < r.posInB + r.length; k++) coveredB.insert(k);
        }

        if ((int)filtered.size() >= topK) break;
    }

    return filtered;
}

// ===================== MAIN =====================
int main(int argc, char* argv[]) {
    if (argc < 3) {
        cerr << "Usage: suffix_array.exe <file_a> <file_b>" << endl;
        return 1;
    }

    string fileA = argv[1];
    string fileB = argv[2];

    auto startTime = chrono::high_resolution_clock::now();

    string textA = cleanText(toLowercase(readFile(fileA)));
    string textB = cleanText(toLowercase(readFile(fileB)));

    if (textA.empty() || textB.empty()) {
        cout << "{\"error\": \"Could not read one or both files\"}" << endl;
        return 1;
    }

    // Truncate very long texts for suffix array (memory constraint)
    int maxLen = 50000;
    if ((int)textA.size() > maxLen) textA = textA.substr(0, maxLen);
    if ((int)textB.size() > maxLen) textB = textB.substr(0, maxLen);

    // Build suffix array for combined text
    string combined = textA + '#' + textB + '$';
    vector<int> sa = buildSuffixArray(combined);
    vector<int> lcp = buildLCPArray(combined, sa);

    // Find longest common substrings
    vector<LCSResult> results = findLongestCommonSubstrings(textA, textB, 15);

    auto endTime = chrono::high_resolution_clock::now();
    double duration = chrono::duration<double, milli>(endTime - startTime).count();

    // Calculate total shared characters
    int totalShared = 0;
    for (const LCSResult& r : results) totalShared += r.length;
    double similarity = min(100.0, (double)totalShared / max(1, (int)min(textA.size(), textB.size())) * 100.0);

    // Output JSON
    cout << "{" << endl;
    cout << "  \"algorithm\": \"Suffix Array + LCP Array\"," << endl;
    cout << "  \"time_complexity\": \"O(n log^2 n) construction, O(n) LCP\"," << endl;
    cout << "  \"space_complexity\": \"O(n)\"," << endl;
    cout << "  \"runtime_ms\": " << duration << "," << endl;
    cout << "  \"text_a_length\": " << textA.size() << "," << endl;
    cout << "  \"text_b_length\": " << textB.size() << "," << endl;
    cout << "  \"combined_length\": " << combined.size() << "," << endl;
    cout << "  \"suffix_array_size\": " << sa.size() << "," << endl;
    cout << "  \"total_shared_characters\": " << totalShared << "," << endl;
    cout << "  \"similarity_percentage\": " << similarity << "," << endl;

    // Longest common substring (the top result)
    if (!results.empty()) {
        cout << "  \"longest_common_substring\": \"" << jsonEscape(results[0].substring.substr(0, 200)) << "\"," << endl;
        cout << "  \"longest_length\": " << results[0].length << "," << endl;
    } else {
        cout << "  \"longest_common_substring\": \"\"," << endl;
        cout << "  \"longest_length\": 0," << endl;
    }

    // Suffix array structure sample (first 20 entries)
    cout << "  \"suffix_array_sample\": [";
    for (int i = 0; i < min((int)sa.size(), 20); i++) {
        if (i > 0) cout << ", ";
        cout << sa[i];
    }
    cout << "]," << endl;

    // LCP array sample
    cout << "  \"lcp_array_sample\": [";
    for (int i = 0; i < min((int)lcp.size(), 20); i++) {
        if (i > 0) cout << ", ";
        cout << lcp[i];
    }
    cout << "]," << endl;

    // All common substrings
    cout << "  \"common_substrings\": [" << endl;
    for (int i = 0; i < (int)results.size(); i++) {
        const LCSResult& r = results[i];
        cout << "    {" << endl;
        cout << "      \"substring\": \"" << jsonEscape(r.substring.substr(0, 200)) << "\"," << endl;
        cout << "      \"length\": " << r.length << "," << endl;
        cout << "      \"position_in_a\": " << r.posInA << "," << endl;
        cout << "      \"position_in_b\": " << r.posInB << endl;
        cout << "    }";
        if (i < (int)results.size() - 1) cout << ",";
        cout << endl;
    }
    cout << "  ]" << endl;
    cout << "}" << endl;

    return 0;
}
