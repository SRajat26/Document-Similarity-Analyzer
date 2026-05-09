/*
 * corpus_search.cpp
 * Efficient Corpus Search — Divide & Conquer + Hashing + Binary Search
 *
 * Implements:
 *   - Document fingerprint generation (hash-based)
 *   - Sorted fingerprint indexes
 *   - Binary search over indexes
 *   - Divide and conquer pruning
 *   - Trie for phrase indexing
 *   - Hash table for fingerprints
 *   - Priority queue for ranking
 *
 * Usage: corpus_search.exe <query_file> <corpus_dir> [top_k]
 * Output: JSON with ranked results, search stats, pruning info
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
#include <queue>
#include <filesystem>

namespace fs = std::filesystem;
using namespace std;

// ===================== CONSTANTS =====================
const long long HASH_MOD = 1000000007LL;
const long long HASH_BASE = 31LL;

// ===================== UTILITY FUNCTIONS =====================
string readFile(const string& filepath) {
    ifstream file(filepath);
    if (!file.is_open()) return "";
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

vector<string> tokenize(const string& text) {
    vector<string> tokens;
    istringstream stream(text);
    string word;
    set<string> stopWords = {
        "a", "an", "the", "and", "or", "but", "is", "are", "was", "were",
        "be", "been", "being", "have", "has", "had", "do", "does", "did",
        "to", "of", "in", "for", "on", "with", "at", "by", "from", "as",
        "it", "its", "this", "that", "he", "she", "they", "we", "you", "i"
    };
    while (stream >> word) {
        if (!word.empty() && stopWords.find(word) == stopWords.end()) {
            tokens.push_back(word);
        }
    }
    return tokens;
}

// ===================== TRIE DATA STRUCTURE =====================
struct TrieNode {
    unordered_map<char, int> children;
    bool isEnd;
    vector<int> docIds; // documents containing this phrase

    TrieNode() : isEnd(false) {}
};

class Trie {
private:
    vector<TrieNode> nodes;

public:
    Trie() {
        nodes.push_back(TrieNode()); // root
    }

    void insert(const string& word, int docId) {
        int current = 0;
        for (char c : word) {
            if (nodes[current].children.find(c) == nodes[current].children.end()) {
                nodes[current].children[c] = nodes.size();
                nodes.push_back(TrieNode());
            }
            current = nodes[current].children[c];
        }
        nodes[current].isEnd = true;
        nodes[current].docIds.push_back(docId);
    }

    vector<int> search(const string& word) {
        int current = 0;
        for (char c : word) {
            if (nodes[current].children.find(c) == nodes[current].children.end()) {
                return {};
            }
            current = nodes[current].children[c];
        }
        if (nodes[current].isEnd) {
            return nodes[current].docIds;
        }
        return {};
    }

    int size() { return nodes.size(); }
};

// ===================== HASH TABLE FOR FINGERPRINTS =====================
class FingerprintHashTable {
private:
    static const int TABLE_SIZE = 10007; // prime
    vector<vector<pair<long long, int>>> table;  // hash -> list of (fingerprint, docId)

public:
    FingerprintHashTable() : table(TABLE_SIZE) {}

    void insert(long long fingerprint, int docId) {
        int bucket = fingerprint % TABLE_SIZE;
        table[bucket].push_back({fingerprint, docId});
    }

    vector<int> lookup(long long fingerprint) {
        int bucket = fingerprint % TABLE_SIZE;
        vector<int> result;
        for (auto& [fp, id] : table[bucket]) {
            if (fp == fingerprint) {
                result.push_back(id);
            }
        }
        return result;
    }
};

struct DocumentFingerprint {
    int docId;
    string filename;
    vector<long long> fingerprints;  // set of hash fingerprints
    vector<string> tokens;
    long long primaryHash;  // single representative hash
};

long long computeHash(const string& s) {
    long long hash = 0;
    long long power = 1;
    for (char c : s) {
        long long val = (unsigned char)c;
        hash = (hash + val * power) % HASH_MOD;
        power = (power * HASH_BASE) % HASH_MOD;
    }
    if (hash < 0) hash += HASH_MOD;
    return hash;
}

vector<long long> generateFingerprints(const vector<string>& tokens, int shingleSize) {
    vector<long long> fingerprints;
    for (int i = 0; i <= (int)tokens.size() - shingleSize; i++) {
        string shingle;
        for (int j = 0; j < shingleSize; j++) {
            if (j > 0) shingle += " ";
            shingle += tokens[i + j];
        }
        fingerprints.push_back(computeHash(shingle));
    }
    return fingerprints;
}

// ===================== BINARY SEARCH ON SORTED FINGERPRINTS =====================
bool binarySearchFingerprint(const vector<long long>& sorted, long long target) {
    int lo = 0, hi = (int)sorted.size() - 1;
    while (lo <= hi) {
        int mid = lo + (hi - lo) / 2;
        if (sorted[mid] == target) return true;
        else if (sorted[mid] < target) lo = mid + 1;
        else hi = mid - 1;
    }
    return false;
}

// ===================== JACCARD SIMILARITY VIA FINGERPRINTS =====================
double fingerprintJaccard(const vector<long long>& sortedA, const vector<long long>& sortedB) {
    int i = 0, j = 0;
    int intersection = 0, union_ = 0;

    while (i < (int)sortedA.size() && j < (int)sortedB.size()) {
        if (sortedA[i] == sortedB[j]) {
            intersection++;
            union_++;
            i++; j++;
        } else if (sortedA[i] < sortedB[j]) {
            union_++;
            i++;
        } else {
            union_++;
            j++;
        }
    }
    union_ += ((int)sortedA.size() - i) + ((int)sortedB.size() - j);

    if (union_ == 0) return 0.0;
    return (double)intersection / union_;
}

// ===================== DIVIDE AND CONQUER PRUNING =====================
// Partition corpus docs and prune irrelevant halves based on hash overlap
struct SearchStats {
    int totalDocs;
    int docsPruned;
    int docsSearched;
    int binarySearches;
    int hashLookups;
};

struct CorpusMatch {
    int docId;
    string filename;
    double similarity;
    int sharedFingerprints;
    vector<string> sharedPhrases;
};

// Divide and conquer: split docs, compute aggregate similarity, prune low halves
void divideAndConquerSearch(
    const vector<DocumentFingerprint>& docs,
    const vector<long long>& queryFPs,
    int lo, int hi,
    double threshold,
    vector<CorpusMatch>& results,
    SearchStats& stats
) {
    if (lo > hi) return;

    if (lo == hi) {
        // Base case: compare single document
        stats.docsSearched++;

        vector<long long> sortedDocFP = docs[lo].fingerprints;
        sort(sortedDocFP.begin(), sortedDocFP.end());

        vector<long long> sortedQueryFP = queryFPs;
        sort(sortedQueryFP.begin(), sortedQueryFP.end());

        double sim = fingerprintJaccard(sortedDocFP, sortedQueryFP);

        int shared = 0;
        for (long long fp : queryFPs) {
            stats.binarySearches++;
            if (binarySearchFingerprint(sortedDocFP, fp)) {
                shared++;
            }
        }

        if (sim > 0.01) {
            CorpusMatch cm;
            cm.docId = docs[lo].docId;
            cm.filename = docs[lo].filename;
            cm.similarity = sim * 100.0;
            cm.sharedFingerprints = shared;
            results.push_back(cm);
        }
        return;
    }

    int mid = lo + (hi - lo) / 2;

    // Quick aggregate check for left half
    unordered_set<long long> leftHashes;
    for (int i = lo; i <= mid; i++) {
        for (long long fp : docs[i].fingerprints) {
            leftHashes.insert(fp);
        }
    }
    int leftOverlap = 0;
    for (long long fp : queryFPs) {
        stats.hashLookups++;
        if (leftHashes.count(fp)) leftOverlap++;
    }

    // Quick aggregate check for right half
    unordered_set<long long> rightHashes;
    for (int i = mid + 1; i <= hi; i++) {
        for (long long fp : docs[i].fingerprints) {
            rightHashes.insert(fp);
        }
    }
    int rightOverlap = 0;
    for (long long fp : queryFPs) {
        stats.hashLookups++;
        if (rightHashes.count(fp)) rightOverlap++;
    }

    double leftRatio = (double)leftOverlap / max(1, (int)queryFPs.size());
    double rightRatio = (double)rightOverlap / max(1, (int)queryFPs.size());

    // Recurse into halves, prune if very low overlap
    if (leftRatio >= threshold || (mid - lo + 1) <= 3) {
        divideAndConquerSearch(docs, queryFPs, lo, mid, threshold, results, stats);
    } else {
        stats.docsPruned += (mid - lo + 1);
    }

    if (rightRatio >= threshold || (hi - mid) <= 3) {
        divideAndConquerSearch(docs, queryFPs, mid + 1, hi, threshold, results, stats);
    } else {
        stats.docsPruned += (hi - mid);
    }
}

// ===================== MAIN =====================
int main(int argc, char* argv[]) {
    if (argc < 3) {
        cerr << "Usage: corpus_search.exe <query_file> <corpus_dir> [top_k]" << endl;
        return 1;
    }

    string queryFile = argv[1];
    string corpusDir = argv[2];
    int topK = 10;
    if (argc >= 4) topK = atoi(argv[3]);

    auto startTime = chrono::high_resolution_clock::now();

    // Read query document
    string queryText = cleanText(toLowercase(readFile(queryFile)));
    if (queryText.empty()) {
        cout << "{\"error\": \"Could not read query file\"}" << endl;
        return 1;
    }

    vector<string> queryTokens = tokenize(queryText);
    vector<long long> queryFingerprints = generateFingerprints(queryTokens, 3);

    // Load corpus documents
    vector<DocumentFingerprint> corpus;
    Trie trie;
    FingerprintHashTable hashTable;

    int docId = 0;
    try {
        for (const auto& entry : fs::directory_iterator(corpusDir)) {
            if (entry.is_regular_file()) {
                string ext = entry.path().extension().string();
                if (ext == ".txt" || ext == ".md" || ext == ".text") {
                    string content = cleanText(toLowercase(readFile(entry.path().string())));
                    if (!content.empty()) {
                        DocumentFingerprint df;
                        df.docId = docId;
                        df.filename = entry.path().filename().string();
                        df.tokens = tokenize(content);
                        df.fingerprints = generateFingerprints(df.tokens, 3);

                        // Insert into trie
                        for (const string& token : df.tokens) {
                            trie.insert(token, docId);
                        }

                        // Insert fingerprints into hash table
                        for (long long fp : df.fingerprints) {
                            hashTable.insert(fp, docId);
                        }

                        // Sort fingerprints for binary search
                        sort(df.fingerprints.begin(), df.fingerprints.end());

                        if (!df.fingerprints.empty()) {
                            df.primaryHash = df.fingerprints[df.fingerprints.size() / 2];
                        } else {
                            df.primaryHash = 0;
                        }

                        corpus.push_back(df);
                        docId++;
                    }
                }
            }
        }
    } catch (const exception& e) {
        cout << "{\"error\": \"Could not read corpus directory: " << jsonEscape(e.what()) << "\"}" << endl;
        return 1;
    }

    if (corpus.empty()) {
        cout << "{\"error\": \"No documents found in corpus\"}" << endl;
        return 1;
    }

    // Sort corpus by primary hash for divide and conquer
    sort(corpus.begin(), corpus.end(), [](const DocumentFingerprint& a, const DocumentFingerprint& b) {
        return a.primaryHash < b.primaryHash;
    });

    // Divide and conquer search
    SearchStats stats;
    stats.totalDocs = corpus.size();
    stats.docsPruned = 0;
    stats.docsSearched = 0;
    stats.binarySearches = 0;
    stats.hashLookups = 0;

    vector<CorpusMatch> results;
    divideAndConquerSearch(corpus, queryFingerprints, 0, corpus.size() - 1, 0.005, results, stats);

    // Find shared phrases for top results using trie
    for (CorpusMatch& cm : results) {
        for (const string& token : queryTokens) {
            vector<int> trieResult = trie.search(token);
            for (int id : trieResult) {
                if (id == cm.docId) {
                    if (cm.sharedPhrases.size() < 20) {
                        cm.sharedPhrases.push_back(token);
                    }
                    break;
                }
            }
        }
    }

    // Sort by similarity (priority queue style)
    // Using a min-heap to get top K
    auto cmp = [](const CorpusMatch& a, const CorpusMatch& b) {
        return a.similarity > b.similarity;
    };
    priority_queue<CorpusMatch, vector<CorpusMatch>, decltype(cmp)> pq(cmp);

    for (const CorpusMatch& cm : results) {
        pq.push(cm);
        if ((int)pq.size() > topK) pq.pop();
    }

    // Extract from priority queue
    vector<CorpusMatch> topResults;
    while (!pq.empty()) {
        topResults.push_back(pq.top());
        pq.pop();
    }
    reverse(topResults.begin(), topResults.end()); // highest first

    auto endTime = chrono::high_resolution_clock::now();
    double duration = chrono::duration<double, milli>(endTime - startTime).count();

    // Output JSON
    cout << "{" << endl;
    cout << "  \"algorithm\": \"Corpus Search (D&C + Hashing + Binary Search + Trie)\"," << endl;
    cout << "  \"time_complexity\": \"O(n log n) average with pruning\"," << endl;
    cout << "  \"space_complexity\": \"O(n * k) for fingerprints\"," << endl;
    cout << "  \"runtime_ms\": " << duration << "," << endl;
    cout << "  \"query_tokens\": " << queryTokens.size() << "," << endl;
    cout << "  \"query_fingerprints\": " << queryFingerprints.size() << "," << endl;
    cout << "  \"corpus_size\": " << stats.totalDocs << "," << endl;
    cout << "  \"documents_pruned\": " << stats.docsPruned << "," << endl;
    cout << "  \"documents_searched\": " << stats.docsSearched << "," << endl;
    cout << "  \"binary_searches_performed\": " << stats.binarySearches << "," << endl;
    cout << "  \"hash_lookups\": " << stats.hashLookups << "," << endl;
    cout << "  \"trie_nodes\": " << trie.size() << "," << endl;

    // Results
    cout << "  \"results\": [" << endl;
    for (int i = 0; i < (int)topResults.size(); i++) {
        const CorpusMatch& cm = topResults[i];
        cout << "    {" << endl;
        cout << "      \"rank\": " << (i + 1) << "," << endl;
        cout << "      \"filename\": \"" << jsonEscape(cm.filename) << "\"," << endl;
        cout << "      \"similarity_percentage\": " << cm.similarity << "," << endl;
        cout << "      \"shared_fingerprints\": " << cm.sharedFingerprints << "," << endl;

        cout << "      \"shared_phrases\": [";
        for (int j = 0; j < (int)cm.sharedPhrases.size(); j++) {
            if (j > 0) cout << ", ";
            cout << "\"" << jsonEscape(cm.sharedPhrases[j]) << "\"";
        }
        cout << "]" << endl;

        cout << "    }";
        if (i < (int)topResults.size() - 1) cout << ",";
        cout << endl;
    }
    cout << "  ]" << endl;
    cout << "}" << endl;

    return 0;
}
