# Algorithmic Document Similarity Analyzer

A full-stack application designed to analyze and compare text documents for similarity and potential plagiarism using classical string matching and dynamic programming algorithms. The project features a fast React-based frontend and a high-performance C++ backend orchestrated by FastAPI.

## 🚀 Features

- **1-on-1 Document Comparison**: Directly compare two documents to find exact matches, longest shared substrings, and paraphrased similarities.
- **Corpus Search**: Search a single query document against a large database (corpus) of files using advanced retrieval techniques, trie construction, and divide & conquer pruning.
- **Explainable Results**: The UI provides detailed insights into similarity metrics, algorithmic time/space complexity, and token-level visual highlights.
- **High Performance**: Heavy lifting is done entirely in C++ ensuring optimal speed and memory efficiency.

## 🧠 Algorithms Implemented

1. **Knuth-Morris-Pratt (KMP)**:
   - *Purpose*: Extracts contiguous phrases and builds an LPS (Longest Proper Prefix which is also Suffix) array to find exact, un-paraphrased copies of phrases.
   - *Complexity*: $O(N + M)$ time.
2. **Rabin-Karp**:
   - *Purpose*: Uses rolling hashes to scan sliding windows of text. Extremely fast for multi-pattern matching and catching direct copy-pasting.
   - *Complexity*: $O(N + M)$ average time.
3. **Suffix Arrays & Kasai's LCP**:
   - *Purpose*: Generates a Suffix Array and applies Kasai's algorithm to compute the Longest Common Prefix (LCP). Identifies the absolute longest exact contiguous blocks of text shared between two files.
   - *Complexity*: $O(N \log^2 N)$ time for prefix-doubling.
4. **Longest Common Subsequence (LCS)**:
   - *Purpose*: Uses Dynamic Programming to find the longest non-contiguous shared sequence. Extremely useful for catching paraphrased plagiarism where filler words are injected.
   - *Complexity*: $O(N \times M)$ time.
5. **Trie & Divide-and-Conquer Pruning**:
   - *Purpose*: For corpus searching, documents are fingerprinted and searched. Divide-and-conquer quickly prunes irrelevant documents $O(1)$ time, and a Trie helps rapidly identify exact shared phrases.

## 🛠️ Technology Stack

- **Frontend**: React 19, Vite, TailwindCSS, Recharts.
- **Backend Orchestrator**: Python, FastAPI, Uvicorn.
- **Core Engine**: C++17 (compiled as standalone executables).

## 📁 Project Structure

```text
DAA/
├── backend/
│   ├── bin/               # Compiled C++ executables
│   ├── corpus/            # Database of documents for Corpus Search
│   ├── cpp/               # C++ source code for algorithms
│   ├── samples/           # Sample text files for testing
│   ├── uploads/           # Temporary storage for uploaded files
│   ├── main.py            # FastAPI server
│   └── requirements.txt   # Python dependencies
├── frontend/
│   ├── public/            # Static assets
│   ├── src/               # React components, pages, and logic
│   ├── package.json       # Node.js dependencies
│   └── vite.config.js     # Vite configuration
└── README.md              # Project documentation
```

## ⚙️ Installation & Setup

### Prerequisites
- Node.js (v18+)
- Python (3.9+)
- C++ Compiler (GCC/G++ or Clang)

### 1. Core Backend (C++)

First, compile the C++ files into executables inside the `backend/bin` directory.

```bash
cd backend/cpp
g++ preprocessor.cpp -o ../bin/preprocessor
g++ kmp.cpp -o ../bin/kmp
g++ rabin_karp.cpp -o ../bin/rabin_karp
g++ suffix_array.cpp -o ../bin/suffix_array
g++ lcs.cpp -o ../bin/lcs
g++ corpus_search.cpp -o ../bin/corpus_search
```
*(Note: On Windows, the executables should be named with `.exe` extension, e.g., `preprocessor.exe`.)*

### 2. Backend Orchestrator (Python)

Navigate to the backend directory and install the required Python packages.

```bash
cd backend
python -m venv venv
source venv/bin/activate  # On Windows: venv\Scripts\activate
pip install -r requirements.txt
```

Run the FastAPI server:

```bash
uvicorn main:app --host 0.0.0.0 --port 8000 --reload
```

### 3. Frontend (React)

Open a new terminal window, navigate to the frontend directory, and install dependencies.

```bash
cd frontend
npm install
```

Start the Vite development server:

```bash
npm run dev
```

The application will be accessible at `http://localhost:5173`.

## 🔬 Execution Pipeline Overview

### Mode 1: 1-on-1 Document Comparison
1. React packages two files and sends them to FastAPI.
2. FastAPI saves them to a temp directory and spawns parallel sub-processes to run `preprocessor.exe`, `kmp.exe`, `rabin_karp.exe`, `suffix_array.exe`, and `lcs.exe`.
3. Results are aggregated via a weighted formula (KMP 25%, Rabin-Karp 25%, Suffix Array 20%, LCS 30%).
4. React receives the JSON response and renders gauges, complexity tables, and highlighted text viewers.

### Mode 2: Corpus Search
1. On start, the system processes the `corpus/` directory, generating token tries and fingerprint hashes.
2. An uploaded query document is fingerprinted. A divide-and-conquer strategy prunes the corpus based on hash overlaps to massively reduce search space.
3. Binary Search over remaining sorted hashes calculates Jaccard Similarity.
4. The top 5 matching documents are dynamically re-evaluated using Suffix Arrays and LCS for high-precision ranking.
5. The final ranked results, along with prune statistics, are rendered on the frontend.
