/*
 * preprocessor.cpp
 * Text Preprocessing Module — Algorithmic Document Similarity Analyzer
 *
 * Implements:
 *   - Lowercase conversion
 *   - Punctuation removal
 *   - Stop word removal
 *   - Tokenization
 *   - Word normalization
 *   - N-gram / shingle generation (unigram, bigram, trigram)
 *
 * Usage: preprocessor.exe <input_file> [ngram_size]
 * Output: JSON to stdout
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <set>
#include <algorithm>
#include <chrono>

using namespace std;

// ===================== STOP WORDS LIST =====================
set<string> getStopWords() {
    return {
        "a", "an", "the", "and", "or", "but", "is", "are", "was", "were",
        "be", "been", "being", "have", "has", "had", "do", "does", "did",
        "will", "would", "could", "should", "may", "might", "shall", "can",
        "to", "of", "in", "for", "on", "with", "at", "by", "from", "as",
        "into", "through", "during", "before", "after", "above", "below",
        "between", "out", "off", "over", "under", "again", "further",
        "then", "once", "here", "there", "when", "where", "why", "how",
        "all", "each", "every", "both", "few", "more", "most", "other",
        "some", "such", "no", "nor", "not", "only", "own", "same", "so",
        "than", "too", "very", "just", "because", "about", "up", "its",
        "it", "he", "she", "they", "we", "you", "i", "me", "my", "your",
        "his", "her", "our", "their", "this", "that", "these", "those",
        "what", "which", "who", "whom", "if", "while", "also", "am"
    };
}

// ===================== LOWERCASE CONVERSION =====================
string toLowercase(const string& text) {
    string result = text;
    for (char& c : result) {
        if (c >= 'A' && c <= 'Z') {
            c = c + ('a' - 'A');
        }
    }
    return result;
}

// ===================== PUNCTUATION REMOVAL =====================
string removePunctuation(const string& text) {
    string result;
    result.reserve(text.size());
    for (char c : text) {
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9') || c == ' ' || c == '\n' || c == '\t') {
            result += c;
        } else {
            result += ' '; // replace punctuation with space
        }
    }
    return result;
}

// ===================== TOKENIZATION =====================
vector<string> tokenize(const string& text) {
    vector<string> tokens;
    istringstream stream(text);
    string word;
    while (stream >> word) {
        if (!word.empty()) {
            tokens.push_back(word);
        }
    }
    return tokens;
}

// ===================== STOP WORD REMOVAL =====================
vector<string> removeStopWords(const vector<string>& tokens, const set<string>& stopWords) {
    vector<string> filtered;
    for (const string& token : tokens) {
        if (stopWords.find(token) == stopWords.end()) {
            filtered.push_back(token);
        }
    }
    return filtered;
}

// ===================== WORD NORMALIZATION =====================
// Simple stemming: remove common suffixes
string normalizeWord(const string& word) {
    string w = word;
    int len = w.length();

    // Remove "ing" suffix
    if (len > 5 && w.substr(len - 3) == "ing") {
        w = w.substr(0, len - 3);
        len = w.length();
    }
    // Remove "tion" suffix
    else if (len > 5 && w.substr(len - 4) == "tion") {
        w = w.substr(0, len - 4);
        len = w.length();
    }
    // Remove "ly" suffix
    else if (len > 4 && w.substr(len - 2) == "ly") {
        w = w.substr(0, len - 2);
        len = w.length();
    }
    // Remove "ed" suffix
    else if (len > 4 && w.substr(len - 2) == "ed") {
        w = w.substr(0, len - 2);
        len = w.length();
    }
    // Remove "es" suffix
    else if (len > 4 && w.substr(len - 2) == "es") {
        w = w.substr(0, len - 2);
        len = w.length();
    }
    // Remove "s" suffix (but not "ss")
    else if (len > 3 && w[len - 1] == 's' && w[len - 2] != 's') {
        w = w.substr(0, len - 1);
    }

    return w;
}

vector<string> normalizeTokens(const vector<string>& tokens) {
    vector<string> normalized;
    for (const string& token : tokens) {
        normalized.push_back(normalizeWord(token));
    }
    return normalized;
}

// ===================== N-GRAM / SHINGLE GENERATION =====================
vector<string> generateNgrams(const vector<string>& tokens, int n) {
    vector<string> ngrams;
    if ((int)tokens.size() < n) return ngrams;

    for (int i = 0; i <= (int)tokens.size() - n; i++) {
        string gram;
        for (int j = 0; j < n; j++) {
            if (j > 0) gram += " ";
            gram += tokens[i + j];
        }
        ngrams.push_back(gram);
    }
    return ngrams;
}

// ===================== JSON ESCAPE =====================
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

// ===================== READ FILE =====================
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

// ===================== MAIN =====================
int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Usage: preprocessor.exe <input_file> [ngram_size]" << endl;
        return 1;
    }

    string filepath = argv[1];
    int ngramSize = 3; // default trigram
    if (argc >= 3) {
        ngramSize = atoi(argv[2]);
        if (ngramSize < 1) ngramSize = 1;
        if (ngramSize > 5) ngramSize = 5;
    }

    auto startTime = chrono::high_resolution_clock::now();

    // Read input file
    string rawText = readFile(filepath);
    if (rawText.empty()) {
        cout << "{\"error\": \"Could not read file or file is empty\"}" << endl;
        return 1;
    }

    int originalLength = rawText.size();

    // Step 1: Lowercase
    string lowered = toLowercase(rawText);

    // Step 2: Remove punctuation
    string cleaned = removePunctuation(lowered);

    // Step 3: Tokenize
    vector<string> tokens = tokenize(cleaned);
    int tokenCountBeforeStopWords = tokens.size();

    // Step 4: Remove stop words
    set<string> stopWords = getStopWords();
    vector<string> filteredTokens = removeStopWords(tokens, stopWords);
    int tokenCountAfterStopWords = filteredTokens.size();
    int stopWordsRemoved = tokenCountBeforeStopWords - tokenCountAfterStopWords;

    // Step 5: Normalize
    vector<string> normalizedTokens = normalizeTokens(filteredTokens);

    // Step 6: Generate n-grams (unigram, bigram, trigram)
    vector<string> unigrams = generateNgrams(normalizedTokens, 1);
    vector<string> bigrams = generateNgrams(normalizedTokens, 2);
    vector<string> trigrams = generateNgrams(normalizedTokens, 3);
    vector<string> requestedNgrams = generateNgrams(normalizedTokens, ngramSize);

    auto endTime = chrono::high_resolution_clock::now();
    double duration = chrono::duration<double, milli>(endTime - startTime).count();

    // Build processed text preview (first 500 chars)
    string processedPreview;
    for (int i = 0; i < min((int)normalizedTokens.size(), 80); i++) {
        if (i > 0) processedPreview += " ";
        processedPreview += normalizedTokens[i];
    }

    // Output JSON
    cout << "{" << endl;
    cout << "  \"original_length\": " << originalLength << "," << endl;
    cout << "  \"token_count_raw\": " << tokenCountBeforeStopWords << "," << endl;
    cout << "  \"token_count_filtered\": " << tokenCountAfterStopWords << "," << endl;
    cout << "  \"stop_words_removed\": " << stopWordsRemoved << "," << endl;
    cout << "  \"processing_time_ms\": " << duration << "," << endl;

    // Processed text preview
    cout << "  \"processed_preview\": \"" << jsonEscape(processedPreview) << "\"," << endl;

    // Tokens array
    cout << "  \"tokens\": [";
    for (int i = 0; i < (int)normalizedTokens.size(); i++) {
        if (i > 0) cout << ", ";
        cout << "\"" << jsonEscape(normalizedTokens[i]) << "\"";
    }
    cout << "]," << endl;

    // Unigrams
    cout << "  \"unigrams\": [";
    for (int i = 0; i < min((int)unigrams.size(), 50); i++) {
        if (i > 0) cout << ", ";
        cout << "\"" << jsonEscape(unigrams[i]) << "\"";
    }
    cout << "]," << endl;
    cout << "  \"unigram_count\": " << unigrams.size() << "," << endl;

    // Bigrams
    cout << "  \"bigrams\": [";
    for (int i = 0; i < min((int)bigrams.size(), 50); i++) {
        if (i > 0) cout << ", ";
        cout << "\"" << jsonEscape(bigrams[i]) << "\"";
    }
    cout << "]," << endl;
    cout << "  \"bigram_count\": " << bigrams.size() << "," << endl;

    // Trigrams
    cout << "  \"trigrams\": [";
    for (int i = 0; i < min((int)trigrams.size(), 50); i++) {
        if (i > 0) cout << ", ";
        cout << "\"" << jsonEscape(trigrams[i]) << "\"";
    }
    cout << "]," << endl;
    cout << "  \"trigram_count\": " << trigrams.size() << "," << endl;

    // Requested n-grams
    cout << "  \"ngram_size\": " << ngramSize << "," << endl;
    cout << "  \"shingles\": [";
    for (int i = 0; i < min((int)requestedNgrams.size(), 100); i++) {
        if (i > 0) cout << ", ";
        cout << "\"" << jsonEscape(requestedNgrams[i]) << "\"";
    }
    cout << "]," << endl;
    cout << "  \"shingle_count\": " << requestedNgrams.size() << endl;

    cout << "}" << endl;

    return 0;
}
