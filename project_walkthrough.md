# Algorithmic Document Similarity Analyzer - Complete Walkthrough

This document outlines the exact step-by-step execution pipeline of the system, detailing how files move from the frontend to the C++ core algorithms, and how results are derived.

---

## Mode 1: 1-on-1 Document Comparison

When a user selects two documents (Document A and Document B) and clicks "Run Algorithmic Comparison", the following deterministic sequence executes:

### Phase 1: API Hand-off & Initialization
1. **Frontend**: React packages the two files (`text/plain` or `.md`) into a `FormData` object and sends a `POST /api/compare` request to the FastAPI backend.
2. **Backend**: FastAPI receives the files, saves them to a temporary `uploads/` directory to allow the C++ executables to read them directly from the disk.

### Phase 2: Parallel Algorithmic Execution
The FastAPI backend (`main.py`) acts as an orchestrator, spawning sub-processes to run independent C++ binaries.

#### 1. Preprocessing (`preprocessor.cpp`)
*   **What it does:** Standardizes the text. Converts everything to lowercase, strips out punctuation, and removes common stop words (e.g., "the", "and", "is").
*   **Purpose:** Ensures algorithms aren't fooled by simple case changes or formatting tricks.

#### 2. KMP (Knuth-Morris-Pratt) Algorithm (`kmp.cpp`)
*   **What it does:** Extracts contiguous phrases (3-8 words long) from Document A. For each phrase, it builds a Longest Proper Prefix which is also Suffix (LPS) array, which acts as a failure function. It then scans Document B linearly. 
*   **Detailed Working:** Traditional search algorithms backtrack when a character mismatch occurs. KMP uses the LPS array to know exactly how far to shift the search window forward without re-evaluating characters it has already seen. This ensures a strict $O(N+M)$ time complexity.
*   **Result:** Finds exact, un-paraphrased copies of phrases and returns their specific character positions, bypassing superficial spacing tricks.

#### 3. Rabin-Karp Algorithm (`rabin_karp.cpp`)
*   **What it does:** Uses a rolling hash function to scan sliding windows of text. It computes a numeric hash (using a prime modulus to prevent overflow) for a block of text in Document A and slides through Document B comparing hashes.
*   **Detailed Working:** Instead of comparing characters one-by-one, it treats a sequence of characters as a base-$P$ number. When the window slides, the algorithm mathematically drops the oldest character's value and adds the newest character's value in $O(1)$ time. If the hashes match, a secondary exact-character check is performed to resolve potential hash collisions.
*   **Result:** Extremely fast for multi-pattern matching, catching direct copy-pasting rapidly.

#### 4. Suffix Arrays & Kasai's LCP (`suffix_array.cpp`)
*   **What it does:** Concatenates Document A and B with a special delimiter (e.g., `#`). It generates a Suffix Array (a lexicographically sorted array of all suffixes of the string).
*   **Detailed Working:** Sorting the suffixes directly would take $O(N^2 \log N)$, but the implemented prefix-doubling algorithm achieves $O(N \log^2 N)$. Once sorted, identical substrings will be strictly adjacent in the array. It then applies **Kasai's Algorithm** to compute the Longest Common Prefix (LCP) array in $O(N)$ time by examining the overlap between adjacent suffixes.
*   **Result:** Identifies the absolute longest exact contiguous blocks of text shared between the two files.

#### 5. Longest Common Subsequence DP (`lcs.cpp`)
*   **What it does:** Constructs a 2D Dynamic Programming matrix of size $N \times M$ (where N and M are the number of tokens in the documents).
*   **Detailed Working:** Unlike substrings, subsequences don't need to be contiguous. The DP table is built bottom-up. If `wordA[i] == wordB[j]`, the cell takes the diagonal value + 1. If not, it inherits the maximum of the top or left cell. After the table is filled, the algorithm backtracks to reconstruct the actual longest shared sequence.
*   **Result:** Catches **paraphrased plagiarism**. If a user copies a paragraph but injects filler words in between every sentence, KMP fails, but LCS catches the preserved structural order of the original words. Returns a heatmap sample of the DP table.

### Phase 3: Aggregation & Display
1. **Scoring**: If `main.py` detects the files are identical byte-for-byte, it overrides the score to 100%. Otherwise, it aggregates the outputs using a weighted formula:
   *   `KMP`: 25%
   *   `Rabin-Karp`: 25%
   *   `Suffix Array`: 20%
   *   `LCS`: 30%
2. **Cleanup**: The temporary files in `uploads/` are securely deleted.
3. **Frontend**: React receives the massive JSON response and renders the Gauge, Complexity Tables, and highlights the specific array positions returned by the C++ algorithms in the Document Viewer.

---

## Mode 2: Corpus Search

When a user uploads a single query document to search against a database of files, the system employs advanced retrieval techniques.

### Phase 1: Corpus Pre-computation (Startup)
When the application starts, it processes every document in the `corpus/` directory:
1.  **Tokenization**: It breaks texts into tokens.
2.  **Trie Construction**: It inserts every word into a highly optimized C++ Trie (Prefix Tree).
3.  **Fingerprinting**: It generates rolling hash signatures (shingles) for every document and sorts these integers for $O(\log N)$ binary searching.

### Phase 2: Query Execution (`corpus_search.cpp`)
1. **Query Hashing**: The uploaded document is tokenized and its rolling hash fingerprints are generated.
2. **Divide and Conquer Pruning**:
   *   Instead of checking the query against every document sequentially ($O(N)$), the corpus documents are initially sorted by their median primary hash.
   *   The algorithm splits the corpus array recursively in half (Divide).
   *   For a given half, it checks the aggregate hash overlap with the query. By creating a unified `unordered_set` of hashes for that half, it checks how many query hashes fall within it.
   *   If a half shares almost zero hashes with the query (below a strict threshold like 0.005), **the entire half is pruned in $O(1)$ time** without checking its constituent documents.
   *   This algorithm bypasses thousands of irrelevant documents, exponentially speeding up corpus indexing by only "conquering" clusters of documents that show mathematical overlap.
3. **Binary Search Evaluation**: For the un-pruned documents, the algorithm uses Binary Search over their sorted hash arrays to quickly calculate the intersection of hashes (Jaccard Similarity).
4. **Trie Traversal**: For the highest scoring documents, it traverses the Trie to extract the exact english words/phrases they share with the query.

### Phase 3: Deep Refinement & Ranking
1. **Sub-execution**: For the **Top 5** surviving documents, the backend realizes that hash-similarity isn't enough for a final verdict.
2. It spins up the `suffix_array.cpp` and `lcs.cpp` algorithms specifically for the query against those top 5 documents to extract the exact Longest Substrings and DP matrix scores.
3. **Priority Queue**: A C++ Min-Heap (Priority Queue) sorts the documents by their final refined similarity percentage.
4. **Frontend Rendering**: React displays the pruning statistics (e.g., "500 docs pruned in 2ms"), the hash lookup counts, and the ranked list of matches.
