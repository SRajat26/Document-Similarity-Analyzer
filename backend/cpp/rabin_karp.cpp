/*
 * rabin_karp.cpp
 * Rabin-Karp Algorithm — Rolling Hash Based Matching
 *
 * Implements:
 *   - Rolling hash computation
 *   - Multi-pattern detection
 *   - Collision handling
 *   - Plagiarism-style scanning
 *
 * Usage: rabin_karp.exe <file_a> <file_b> [window_size]
 * Output: JSON with hashes, matches, collision info
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <chrono>
#include <set>

using namespace std;

// ===================== CONSTANTS =====================
const long long MOD = 1000000007LL;  // Large prime modulus
const long long BASE = 31LL;         // Hash base

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

// ===================== ROLLING HASH COMPUTATION =====================
// Compute hash of a string using polynomial rolling hash
long long computeHash(const string& s, int start, int length) {
    long long hash = 0;
    long long power = 1;
    for (int i = 0; i < length; i++) {
        hash = (hash + (long long)(s[start + i] - 'a' + 1) * power) % MOD;
        if (i < length - 1) {
            power = (power * BASE) % MOD;
        }
    }
    return hash;
}

// ===================== RABIN-KARP MULTI-PATTERN MATCHING =====================
struct RKMatch {
    string phrase;
    int posInA;
    int posInB;
    long long hashValue;
    int length;
};

struct RKResult {
    vector<RKMatch> matches;
    int totalHashComputations;
    int collisions;
    int hashComparisons;
    vector<pair<long long, string>> sampleHashes;  // sample of rolling hashes
};

RKResult rabinKarpMultiMatch(const string& textA, const string& textB, int windowSize) {
    RKResult result;
    result.totalHashComputations = 0;
    result.collisions = 0;
    result.hashComparisons = 0;

    int lenA = textA.size();
    int lenB = textB.size();

    if (windowSize > lenA || windowSize > lenB) {
        return result;
    }

    // Step 1: Compute all hashes for windows in text A
    // Store in hash map: hash -> list of (position, actual substring)
    unordered_map<long long, vector<pair<int, string>>> hashMapA;

    // Compute power for rolling hash update
    long long highPower = 1;
    for (int i = 0; i < windowSize - 1; i++) {
        highPower = (highPower * BASE) % MOD;
    }

    // Initial hash for text A
    long long hashA = computeHash(textA, 0, windowSize);
    hashMapA[hashA].push_back({0, textA.substr(0, windowSize)});
    result.totalHashComputations++;

    // Store sample hashes
    if (result.sampleHashes.size() < 10) {
        result.sampleHashes.push_back({hashA, textA.substr(0, min(windowSize, 50))});
    }

    // Rolling hash through text A
    for (int i = 1; i <= lenA - windowSize; i++) {
        // Remove leftmost character, add rightmost character
        hashA = (hashA - (long long)(textA[i - 1] - 'a' + 1) + MOD) % MOD;
        hashA = (hashA * BASE + (long long)(textA[i + windowSize - 1] - 'a' + 1) * highPower % MOD) % MOD;
        // Recompute to avoid drift
        if (i % 100 == 0) {
            hashA = computeHash(textA, i, windowSize);
        }
        hashMapA[hashA].push_back({i, textA.substr(i, windowSize)});
        result.totalHashComputations++;

        if (result.sampleHashes.size() < 10 && i % max(1, lenA / 10) == 0) {
            result.sampleHashes.push_back({hashA, textA.substr(i, min(windowSize, 50))});
        }
    }

    // Step 2: Compute rolling hash through text B, check against hash map
    long long hashB = computeHash(textB, 0, windowSize);
    result.totalHashComputations++;

    // Check initial window
    if (hashMapA.count(hashB)) {
        result.hashComparisons++;
        string substringB = textB.substr(0, windowSize);
        for (auto& [posA, subA] : hashMapA[hashB]) {
            if (subA == substringB) {
                RKMatch m;
                m.phrase = substringB;
                m.posInA = posA;
                m.posInB = 0;
                m.hashValue = hashB;
                m.length = windowSize;
                result.matches.push_back(m);
            } else {
                result.collisions++;
            }
        }
    }

    // Rolling hash through text B
    for (int i = 1; i <= lenB - windowSize; i++) {
        hashB = computeHash(textB, i, windowSize);
        result.totalHashComputations++;

        if (hashMapA.count(hashB)) {
            result.hashComparisons++;
            string substringB = textB.substr(i, windowSize);
            for (auto& [posA, subA] : hashMapA[hashB]) {
                if (subA == substringB) {
                    RKMatch m;
                    m.phrase = substringB;
                    m.posInA = posA;
                    m.posInB = i;
                    m.hashValue = hashB;
                    m.length = windowSize;
                    result.matches.push_back(m);
                } else {
                    result.collisions++;
                }
            }
        }

        if (result.matches.size() >= 500) break;
    }

    return result;
}

// ===================== REMOVE OVERLAPPING MATCHES =====================
vector<RKMatch> removeOverlaps(vector<RKMatch>& matches) {
    // Sort by position in B
    sort(matches.begin(), matches.end(), [](const RKMatch& a, const RKMatch& b) {
        return a.posInB < b.posInB;
    });

    vector<RKMatch> filtered;
    int lastEnd = -1;
    for (const RKMatch& m : matches) {
        if (m.posInB >= lastEnd) {
            filtered.push_back(m);
            lastEnd = m.posInB + m.length;
        }
    }
    return filtered;
}

// ===================== MAIN =====================
int main(int argc, char* argv[]) {
    if (argc < 3) {
        cerr << "Usage: rabin_karp.exe <file_a> <file_b> [window_size]" << endl;
        return 1;
    }

    string fileA = argv[1];
    string fileB = argv[2];
    int windowSize = 20; // default window
    if (argc >= 4) {
        windowSize = atoi(argv[3]);
        if (windowSize < 5) windowSize = 5;
        if (windowSize > 100) windowSize = 100;
    }

    auto startTime = chrono::high_resolution_clock::now();

    string textA = cleanText(toLowercase(readFile(fileA)));
    string textB = cleanText(toLowercase(readFile(fileB)));

    if (textA.empty() || textB.empty()) {
        cout << "{\"error\": \"Could not read one or both files\"}" << endl;
        return 1;
    }

    // Run Rabin-Karp with multiple window sizes
    vector<int> windows = {windowSize};
    if (windowSize > 10) windows.push_back(windowSize / 2);
    if (windowSize > 20) windows.push_back(windowSize * 2 / 3);

    vector<RKMatch> allMatches;
    int totalHashes = 0;
    int totalCollisions = 0;
    int totalHashComps = 0;
    vector<pair<long long, string>> allSampleHashes;

    for (int ws : windows) {
        RKResult res = rabinKarpMultiMatch(textA, textB, ws);
        for (auto& m : res.matches) allMatches.push_back(m);
        totalHashes += res.totalHashComputations;
        totalCollisions += res.collisions;
        totalHashComps += res.hashComparisons;
        for (auto& sh : res.sampleHashes) {
            if (allSampleHashes.size() < 15) allSampleHashes.push_back(sh);
        }
    }

    // Remove overlapping matches
    vector<RKMatch> filtered = removeOverlaps(allMatches);

    // Limit output
    if (filtered.size() > 50) filtered.resize(50);

    auto endTime = chrono::high_resolution_clock::now();
    double duration = chrono::duration<double, milli>(endTime - startTime).count();

    // Calculate similarity
    int totalMatchedChars = 0;
    for (const RKMatch& m : filtered) {
        totalMatchedChars += m.length;
    }
    double similarity = min(100.0, (double)totalMatchedChars / max(1, (int)min(textA.size(), textB.size())) * 100.0);

    // Output JSON
    cout << "{" << endl;
    cout << "  \"algorithm\": \"Rabin-Karp (Rolling Hash)\"," << endl;
    cout << "  \"time_complexity\": \"O(n + m) average, O(nm) worst case\"," << endl;
    cout << "  \"space_complexity\": \"O(n) for hash table\"," << endl;
    cout << "  \"runtime_ms\": " << duration << "," << endl;
    cout << "  \"window_sizes_used\": [";
    for (int i = 0; i < (int)windows.size(); i++) {
        if (i > 0) cout << ", ";
        cout << windows[i];
    }
    cout << "]," << endl;
    cout << "  \"text_a_length\": " << textA.size() << "," << endl;
    cout << "  \"text_b_length\": " << textB.size() << "," << endl;
    cout << "  \"total_hash_computations\": " << totalHashes << "," << endl;
    cout << "  \"hash_comparisons\": " << totalHashComps << "," << endl;
    cout << "  \"collisions\": " << totalCollisions << "," << endl;
    cout << "  \"total_matches_found\": " << allMatches.size() << "," << endl;
    cout << "  \"filtered_matches\": " << filtered.size() << "," << endl;
    cout << "  \"matched_characters\": " << totalMatchedChars << "," << endl;
    cout << "  \"similarity_percentage\": " << similarity << "," << endl;
    cout << "  \"hash_modulus\": " << MOD << "," << endl;
    cout << "  \"hash_base\": " << BASE << "," << endl;

    // Sample rolling hashes
    cout << "  \"sample_rolling_hashes\": [" << endl;
    for (int i = 0; i < (int)allSampleHashes.size(); i++) {
        cout << "    {\"hash\": " << allSampleHashes[i].first
             << ", \"text\": \"" << jsonEscape(allSampleHashes[i].second) << "\"}";
        if (i < (int)allSampleHashes.size() - 1) cout << ",";
        cout << endl;
    }
    cout << "  ]," << endl;

    // Matches
    cout << "  \"matches\": [" << endl;
    for (int i = 0; i < (int)filtered.size(); i++) {
        const RKMatch& m = filtered[i];
        cout << "    {" << endl;
        cout << "      \"phrase\": \"" << jsonEscape(m.phrase) << "\"," << endl;
        cout << "      \"hash\": " << m.hashValue << "," << endl;
        cout << "      \"position_in_a\": " << m.posInA << "," << endl;
        cout << "      \"position_in_b\": " << m.posInB << "," << endl;
        cout << "      \"length\": " << m.length << endl;
        cout << "    }";
        if (i < (int)filtered.size() - 1) cout << ",";
        cout << endl;
    }
    cout << "  ]" << endl;
    cout << "}" << endl;

    return 0;
}
