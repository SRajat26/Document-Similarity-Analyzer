/*
 * lcs.cpp
 * Longest Common Subsequence (LCS) — Paraphrase Detection
 *
 * Implements:
 *   - Dynamic programming LCS computation
 *   - Token-level comparison
 *   - LCS table construction
 *   - Backtracking for matched path
 *   - Edit distance (Levenshtein)
 *   - Jaccard similarity
 *   - Synonym dictionary matching (heuristic)
 *
 * Usage: lcs.exe <file_a> <file_b>
 * Output: JSON with LCS info, similarity, matched paths, DP table sample
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <chrono>
#include <set>
#include <unordered_map>
#include <unordered_set>

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

// ===================== TOKENIZATION =====================
vector<string> tokenize(const string& text) {
    vector<string> tokens;
    string cleaned;
    for (char c : text) {
        if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == ' ') {
            cleaned += c;
        } else if (c == '\n' || c == '\t') {
            cleaned += ' ';
        }
    }
    istringstream stream(cleaned);
    string word;
    set<string> stopWords = {
        "a", "an", "the", "and", "or", "but", "is", "are", "was", "were",
        "be", "been", "being", "have", "has", "had", "do", "does", "did",
        "will", "would", "could", "should", "may", "might", "shall", "can",
        "to", "of", "in", "for", "on", "with", "at", "by", "from", "as",
        "into", "through", "during", "before", "after", "it", "its",
        "this", "that", "these", "those", "he", "she", "they", "we", "you", "i"
    };
    while (stream >> word) {
        if (!word.empty() && stopWords.find(word) == stopWords.end()) {
            tokens.push_back(word);
        }
    }
    return tokens;
}

// ===================== SYNONYM DICTIONARY =====================
unordered_map<string, int> buildSynonymGroups() {
    // Simple synonym dictionary — words in same group have same ID
    unordered_map<string, int> synonymMap;
    vector<vector<string>> groups = {
        {"fast", "quick", "rapid", "speedy", "swift"},
        {"big", "large", "huge", "enormous", "massive", "vast"},
        {"small", "tiny", "little", "minute", "compact"},
        {"good", "great", "excellent", "fine", "superior"},
        {"bad", "poor", "terrible", "awful", "inferior"},
        {"start", "begin", "commence", "initiate", "launch"},
        {"end", "finish", "conclude", "terminate", "complete"},
        {"make", "create", "build", "construct", "produce", "generate"},
        {"use", "utilize", "employ", "apply", "leverage"},
        {"show", "display", "demonstrate", "present", "exhibit"},
        {"help", "assist", "aid", "support", "facilitate"},
        {"change", "modify", "alter", "adjust", "transform"},
        {"improve", "enhance", "optimize", "refine", "upgrade"},
        {"important", "significant", "crucial", "critical", "essential", "vital"},
        {"problem", "issue", "challenge", "difficulty", "obstacle"},
        {"method", "approach", "technique", "strategy", "procedure"},
        {"data", "information", "records", "facts"},
        {"system", "framework", "platform", "infrastructure"},
        {"efficient", "effective", "productive", "optimal"},
        {"analyze", "examine", "study", "investigate", "evaluate", "assess"},
        {"implement", "develop", "execute", "deploy"},
        {"algorithm", "procedure", "method", "process", "routine"},
        {"compute", "calculate", "determine", "evaluate"},
        {"complex", "complicated", "intricate", "sophisticated"},
        {"similar", "alike", "comparable", "analogous", "related"},
        {"different", "distinct", "diverse", "varied", "dissimilar"},
        {"increase", "grow", "expand", "rise", "escalate"},
        {"decrease", "reduce", "diminish", "decline", "shrink"},
        {"result", "outcome", "consequence", "effect", "output"},
        {"feature", "characteristic", "attribute", "property", "trait"}
    };

    int groupId = 0;
    for (const auto& group : groups) {
        for (const string& word : group) {
            synonymMap[word] = groupId;
        }
        groupId++;
    }
    return synonymMap;
}

bool areSynonyms(const string& a, const string& b, const unordered_map<string, int>& synMap) {
    if (a == b) return true;
    auto itA = synMap.find(a);
    auto itB = synMap.find(b);
    if (itA != synMap.end() && itB != synMap.end()) {
        return itA->second == itB->second;
    }
    return false;
}

// ===================== LCS DYNAMIC PROGRAMMING =====================
struct LCSResult {
    int lcsLength;
    vector<string> lcsSequence;
    vector<pair<int, int>> matchedPositions; // (posInA, posInB)
    vector<vector<int>> dpTableSample; // small sample of DP table
    int dpRows;
    int dpCols;
};

LCSResult computeLCS(const vector<string>& tokensA, const vector<string>& tokensB,
                     const unordered_map<string, int>& synMap) {
    int m = tokensA.size();
    int n = tokensB.size();

    // Limit for memory: use at most 2000 tokens from each
    int maxTokens = 2000;
    int effM = min(m, maxTokens);
    int effN = min(n, maxTokens);

    // DP table
    vector<vector<int>> dp(effM + 1, vector<int>(effN + 1, 0));

    for (int i = 1; i <= effM; i++) {
        for (int j = 1; j <= effN; j++) {
            if (areSynonyms(tokensA[i - 1], tokensB[j - 1], synMap)) {
                dp[i][j] = dp[i - 1][j - 1] + 1;
            } else {
                dp[i][j] = max(dp[i - 1][j], dp[i][j - 1]);
            }
        }
    }

    // Backtrack to find the LCS sequence
    LCSResult result;
    result.lcsLength = dp[effM][effN];
    result.dpRows = effM;
    result.dpCols = effN;

    int i = effM, j = effN;
    while (i > 0 && j > 0) {
        if (areSynonyms(tokensA[i - 1], tokensB[j - 1], synMap)) {
            result.lcsSequence.push_back(tokensA[i - 1]);
            result.matchedPositions.push_back({i - 1, j - 1});
            i--;
            j--;
        } else if (dp[i - 1][j] > dp[i][j - 1]) {
            i--;
        } else {
            j--;
        }
    }

    // Reverse (backtracking gives reverse order)
    reverse(result.lcsSequence.begin(), result.lcsSequence.end());
    reverse(result.matchedPositions.begin(), result.matchedPositions.end());

    // Extract a sample of the DP table (top-left corner, max 15x15)
    int sampleSize = min(15, min(effM + 1, effN + 1));
    for (int ii = 0; ii < sampleSize; ii++) {
        vector<int> row;
        for (int jj = 0; jj < sampleSize; jj++) {
            row.push_back(dp[ii][jj]);
        }
        result.dpTableSample.push_back(row);
    }

    return result;
}

// ===================== EDIT DISTANCE (LEVENSHTEIN) =====================
int editDistance(const string& a, const string& b) {
    int m = a.size(), n = b.size();
    if (m > 1000) return -1;
    if (n > 1000) return -1;

    vector<vector<int>> dp(m + 1, vector<int>(n + 1, 0));
    for (int i = 0; i <= m; i++) dp[i][0] = i;
    for (int j = 0; j <= n; j++) dp[0][j] = j;

    for (int i = 1; i <= m; i++) {
        for (int j = 1; j <= n; j++) {
            if (a[i - 1] == b[j - 1]) {
                dp[i][j] = dp[i - 1][j - 1];
            } else {
                dp[i][j] = 1 + min({dp[i - 1][j], dp[i][j - 1], dp[i - 1][j - 1]});
            }
        }
    }
    return dp[m][n];
}

// ===================== JACCARD SIMILARITY =====================
double jaccardSimilarity(const vector<string>& tokensA, const vector<string>& tokensB) {
    unordered_set<string> setA(tokensA.begin(), tokensA.end());
    unordered_set<string> setB(tokensB.begin(), tokensB.end());

    int intersection = 0;
    for (const string& s : setA) {
        if (setB.count(s)) intersection++;
    }

    int union_ = setA.size() + setB.size() - intersection;
    if (union_ == 0) return 0.0;
    return (double)intersection / union_;
}

// ===================== MAIN =====================
int main(int argc, char* argv[]) {
    if (argc < 3) {
        cerr << "Usage: lcs.exe <file_a> <file_b>" << endl;
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

    // Tokenize
    vector<string> tokensA = tokenize(textA);
    vector<string> tokensB = tokenize(textB);

    // Build synonym map
    unordered_map<string, int> synMap = buildSynonymGroups();

    // Compute LCS
    LCSResult lcsResult = computeLCS(tokensA, tokensB, synMap);

    // Compute Jaccard similarity
    double jaccard = jaccardSimilarity(tokensA, tokensB);

    // Compute edit distance on first 500 chars
    string shortA = textA.substr(0, min((int)textA.size(), 500));
    string shortB = textB.substr(0, min((int)textB.size(), 500));
    int editDist = editDistance(shortA, shortB);

    auto endTime = chrono::high_resolution_clock::now();
    double duration = chrono::duration<double, milli>(endTime - startTime).count();

    // LCS-based similarity
    double lcsSimilarity = (double)lcsResult.lcsLength / max(1, max((int)tokensA.size(), (int)tokensB.size())) * 100.0;

    // Combined similarity score
    double combinedSimilarity = lcsSimilarity * 0.6 + jaccard * 100.0 * 0.3 +
        (editDist >= 0 ? max(0.0, (1.0 - (double)editDist / max(1, (int)max(shortA.size(), shortB.size())))) * 100.0 * 0.1 : lcsSimilarity * 0.1);

    // Output JSON
    cout << "{" << endl;
    cout << "  \"algorithm\": \"LCS (Longest Common Subsequence) + Heuristics\"," << endl;
    cout << "  \"time_complexity\": \"O(n * m) for LCS\"," << endl;
    cout << "  \"space_complexity\": \"O(n * m) for DP table\"," << endl;
    cout << "  \"runtime_ms\": " << duration << "," << endl;
    cout << "  \"tokens_a\": " << tokensA.size() << "," << endl;
    cout << "  \"tokens_b\": " << tokensB.size() << "," << endl;
    cout << "  \"lcs_length\": " << lcsResult.lcsLength << "," << endl;
    cout << "  \"dp_table_rows\": " << lcsResult.dpRows << "," << endl;
    cout << "  \"dp_table_cols\": " << lcsResult.dpCols << "," << endl;
    cout << "  \"lcs_similarity_percentage\": " << lcsSimilarity << "," << endl;
    cout << "  \"jaccard_similarity\": " << jaccard << "," << endl;
    cout << "  \"jaccard_percentage\": " << jaccard * 100.0 << "," << endl;
    cout << "  \"edit_distance\": " << editDist << "," << endl;
    cout << "  \"combined_similarity\": " << combinedSimilarity << "," << endl;
    cout << "  \"synonym_groups_used\": " << 30 << "," << endl;

    // LCS sequence (first 100 tokens)
    cout << "  \"lcs_sequence\": [";
    for (int i = 0; i < min((int)lcsResult.lcsSequence.size(), 100); i++) {
        if (i > 0) cout << ", ";
        cout << "\"" << jsonEscape(lcsResult.lcsSequence[i]) << "\"";
    }
    cout << "]," << endl;

    // Matched token positions
    cout << "  \"matched_positions\": [" << endl;
    for (int i = 0; i < min((int)lcsResult.matchedPositions.size(), 100); i++) {
        cout << "    {\"pos_a\": " << lcsResult.matchedPositions[i].first
             << ", \"pos_b\": " << lcsResult.matchedPositions[i].second << "}";
        if (i < min((int)lcsResult.matchedPositions.size(), 100) - 1) cout << ",";
        cout << endl;
    }
    cout << "  ]," << endl;

    // DP table sample
    cout << "  \"dp_table_sample\": [" << endl;
    for (int i = 0; i < (int)lcsResult.dpTableSample.size(); i++) {
        cout << "    [";
        for (int j = 0; j < (int)lcsResult.dpTableSample[i].size(); j++) {
            if (j > 0) cout << ", ";
            cout << lcsResult.dpTableSample[i][j];
        }
        cout << "]";
        if (i < (int)lcsResult.dpTableSample.size() - 1) cout << ",";
        cout << endl;
    }
    cout << "  ]," << endl;

    // Token samples
    cout << "  \"tokens_a_sample\": [";
    for (int i = 0; i < min((int)tokensA.size(), 15); i++) {
        if (i > 0) cout << ", ";
        cout << "\"" << jsonEscape(tokensA[i]) << "\"";
    }
    cout << "]," << endl;

    cout << "  \"tokens_b_sample\": [";
    for (int i = 0; i < min((int)tokensB.size(), 15); i++) {
        if (i > 0) cout << ", ";
        cout << "\"" << jsonEscape(tokensB[i]) << "\"";
    }
    cout << "]" << endl;

    cout << "}" << endl;

    return 0;
}
