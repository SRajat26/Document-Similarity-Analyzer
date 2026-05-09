/*
 * kmp.cpp
 * KMP (Knuth-Morris-Pratt) Algorithm — Exact Phrase Matching
 *
 * Implements:
 *   - Failure function computation
 *   - Single-pattern matching
 *   - Exact substring detection
 *   - Copied phrase identification
 *
 * Usage: kmp.exe <file_a> <file_b>
 * Output: JSON with matched positions, substrings, occurrences
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <chrono>
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

// ===================== KMP FAILURE FUNCTION =====================
// Compute the longest proper prefix which is also suffix (LPS) array
vector<int> computeFailureFunction(const string& pattern) {
    int m = pattern.size();
    vector<int> lps(m, 0);
    int len = 0;  // length of previous longest prefix suffix
    int i = 1;

    while (i < m) {
        if (pattern[i] == pattern[len]) {
            len++;
            lps[i] = len;
            i++;
        } else {
            if (len != 0) {
                len = lps[len - 1];
                // Do NOT increment i
            } else {
                lps[i] = 0;
                i++;
            }
        }
    }
    return lps;
}

// ===================== KMP SEARCH =====================
// Returns all starting positions where pattern is found in text
vector<int> kmpSearch(const string& text, const string& pattern) {
    vector<int> positions;
    int n = text.size();
    int m = pattern.size();

    if (m == 0 || n == 0 || m > n) return positions;

    vector<int> lps = computeFailureFunction(pattern);

    int i = 0;  // index for text
    int j = 0;  // index for pattern

    while (i < n) {
        if (pattern[j] == text[i]) {
            i++;
            j++;
        }

        if (j == m) {
            // Match found at position (i - j)
            positions.push_back(i - j);
            j = lps[j - 1];
        } else if (i < n && pattern[j] != text[i]) {
            if (j != 0) {
                j = lps[j - 1];
            } else {
                i++;
            }
        }
    }

    return positions;
}

// ===================== EXTRACT PHRASES FROM DOCUMENT =====================
// Extract meaningful phrases (sequences of words) from a document
vector<string> extractPhrases(const string& text, int minWords, int maxWords) {
    // Tokenize
    vector<string> words;
    istringstream stream(text);
    string word;
    while (stream >> word) {
        // Remove non-alphanumeric
        string clean;
        for (char c : word) {
            if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == ' ') {
                clean += c;
            }
        }
        if (!clean.empty()) words.push_back(clean);
    }

    vector<string> phrases;
    set<string> seen;

    for (int len = maxWords; len >= minWords; len--) {
        for (int i = 0; i <= (int)words.size() - len; i++) {
            string phrase;
            for (int j = 0; j < len; j++) {
                if (j > 0) phrase += " ";
                phrase += words[i + j];
            }
            if (seen.find(phrase) == seen.end()) {
                phrases.push_back(phrase);
                seen.insert(phrase);
            }
        }
    }

    return phrases;
}

// ===================== MAIN =====================
int main(int argc, char* argv[]) {
    if (argc < 3) {
        cerr << "Usage: kmp.exe <file_a> <file_b>" << endl;
        return 1;
    }

    string fileA = argv[1];
    string fileB = argv[2];

    auto startTime = chrono::high_resolution_clock::now();

    string textA = toLowercase(readFile(fileA));
    string textB = toLowercase(readFile(fileB));

    if (textA.empty() || textB.empty()) {
        cout << "{\"error\": \"Could not read one or both files\"}" << endl;
        return 1;
    }

    // Extract phrases from document A (patterns to search for in B)
    // Use phrases of 3-8 words for meaningful matching
    vector<string> phrases = extractPhrases(textA, 3, 8);

    struct Match {
        string phrase;
        vector<int> positionsInA;
        vector<int> positionsInB;
        int length;
    };

    vector<Match> matches;
    int totalComparisons = 0;
    int totalMatches = 0;

    // Search for each phrase from A in B using KMP
    for (const string& phrase : phrases) {
        totalComparisons++;

        vector<int> posA = kmpSearch(textA, phrase);
        vector<int> posB = kmpSearch(textB, phrase);

        if (!posB.empty() && phrase.size() >= 10) {  // Only report meaningful matches
            Match m;
            m.phrase = phrase;
            m.positionsInA = posA;
            m.positionsInB = posB;
            m.length = phrase.size();
            matches.push_back(m);
            totalMatches += posB.size();
        }

        // Limit to prevent excessive computation
        if (matches.size() >= 200) break;
    }

    // Sort by match length (longest first)
    sort(matches.begin(), matches.end(), [](const Match& a, const Match& b) {
        return a.length > b.length;
    });

    // Remove overlapping matches (keep longest)
    vector<Match> filteredMatches;
    set<int> coveredPositions;

    for (const Match& m : matches) {
        bool overlaps = false;
        for (int pos : m.positionsInB) {
            for (int k = pos; k < pos + m.length; k++) {
                if (coveredPositions.count(k)) {
                    overlaps = true;
                    break;
                }
            }
            if (overlaps) break;
        }
        if (!overlaps) {
            filteredMatches.push_back(m);
            for (int pos : m.positionsInB) {
                for (int k = pos; k < pos + m.length; k++) {
                    coveredPositions.insert(k);
                }
            }
        }
        if (filteredMatches.size() >= 50) break;
    }

    auto endTime = chrono::high_resolution_clock::now();
    double duration = chrono::duration<double, milli>(endTime - startTime).count();

    // Calculate similarity based on matched characters
    int totalMatchedChars = 0;
    for (const Match& m : filteredMatches) {
        totalMatchedChars += m.length;
    }
    double similarity = min(100.0, (double)totalMatchedChars / max(1, (int)min(textA.size(), textB.size())) * 100.0);

    // Output JSON
    cout << "{" << endl;
    cout << "  \"algorithm\": \"KMP (Knuth-Morris-Pratt)\"," << endl;
    cout << "  \"time_complexity\": \"O(n + m) per pattern\"," << endl;
    cout << "  \"space_complexity\": \"O(m) for failure function\"," << endl;
    cout << "  \"runtime_ms\": " << duration << "," << endl;
    cout << "  \"text_a_length\": " << textA.size() << "," << endl;
    cout << "  \"text_b_length\": " << textB.size() << "," << endl;
    cout << "  \"phrases_checked\": " << totalComparisons << "," << endl;
    cout << "  \"total_matches\": " << totalMatches << "," << endl;
    cout << "  \"unique_matches\": " << filteredMatches.size() << "," << endl;
    cout << "  \"matched_characters\": " << totalMatchedChars << "," << endl;
    cout << "  \"similarity_percentage\": " << similarity << "," << endl;

    // Matched phrases
    cout << "  \"matches\": [" << endl;
    for (int i = 0; i < (int)filteredMatches.size(); i++) {
        const Match& m = filteredMatches[i];
        cout << "    {" << endl;
        cout << "      \"phrase\": \"" << jsonEscape(m.phrase) << "\"," << endl;
        cout << "      \"length\": " << m.length << "," << endl;

        cout << "      \"positions_in_a\": [";
        for (int j = 0; j < (int)m.positionsInA.size(); j++) {
            if (j > 0) cout << ", ";
            cout << m.positionsInA[j];
        }
        cout << "]," << endl;

        cout << "      \"positions_in_b\": [";
        for (int j = 0; j < (int)m.positionsInB.size(); j++) {
            if (j > 0) cout << ", ";
            cout << m.positionsInB[j];
        }
        cout << "]," << endl;

        cout << "      \"occurrences_in_b\": " << m.positionsInB.size() << endl;
        cout << "    }";
        if (i < (int)filteredMatches.size() - 1) cout << ",";
        cout << endl;
    }
    cout << "  ]" << endl;
    cout << "}" << endl;

    return 0;
}
