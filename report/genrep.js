const {
  Document, Packer, Paragraph, TextRun, Table, TableRow, TableCell,
  Header, Footer, AlignmentType, HeadingLevel, BorderStyle, WidthType,
  ShadingType, VerticalAlign, PageNumber, PageBreak, LevelFormat,
  TableOfContents, TabStopType, TabStopPosition
} = require('docx');
const fs = require('fs');

// ─────────────────────────── helpers ───────────────────────────
const TNR = "Times New Roman";
const BODY_SIZE = 24; // 12pt in half-points
const H1_SIZE = 32;   // 16pt
const H2_SIZE = 28;   // 14pt
const H3_SIZE = 24;   // 12pt bold
const CONTENT_W = 9360; // DXA for 1" margins on Letter

const body = (text, opts = {}) => new Paragraph({
  alignment: AlignmentType.JUSTIFIED,
  spacing: { before: 120, after: 120, line: 360 },
  ...opts,
  children: [new TextRun({ text, font: TNR, size: BODY_SIZE, ...(opts.run || {}) })]
});

const bold = (text, opts = {}) => new Paragraph({
  alignment: AlignmentType.JUSTIFIED,
  spacing: { before: 120, after: 120, line: 360 },
  ...opts,
  children: [new TextRun({ text, font: TNR, size: BODY_SIZE, bold: true, ...(opts.run || {}) })]
});

const mixed = (runs, opts = {}) => new Paragraph({
  alignment: AlignmentType.JUSTIFIED,
  spacing: { before: 120, after: 120, line: 360 },
  ...opts,
  children: runs
});

const h1 = (text) => new Paragraph({
  heading: HeadingLevel.HEADING_1,
  spacing: { before: 320, after: 160 },
  children: [new TextRun({ text, font: TNR, size: H1_SIZE, bold: true })]
});

const h2 = (text) => new Paragraph({
  heading: HeadingLevel.HEADING_2,
  spacing: { before: 240, after: 120 },
  children: [new TextRun({ text, font: TNR, size: H2_SIZE, bold: true })]
});

const h3 = (text) => new Paragraph({
  heading: HeadingLevel.HEADING_3,
  spacing: { before: 200, after: 80 },
  children: [new TextRun({ text, font: TNR, size: H3_SIZE, bold: true })]
});

const bullet = (text, level = 0) => new Paragraph({
  numbering: { reference: "bullets", level },
  alignment: AlignmentType.JUSTIFIED,
  spacing: { before: 60, after: 60, line: 360 },
  children: [new TextRun({ text, font: TNR, size: BODY_SIZE })]
});

const numbered = (text, level = 0) => new Paragraph({
  numbering: { reference: "numbers", level },
  alignment: AlignmentType.JUSTIFIED,
  spacing: { before: 60, after: 60, line: 360 },
  children: [new TextRun({ text, font: TNR, size: BODY_SIZE })]
});

const pageBreak = () => new Paragraph({
  children: [new PageBreak()]
});

const space = (size = 120) => new Paragraph({
  spacing: { before: 0, after: size },
  children: []
});

const center = (text, opts = {}) => new Paragraph({
  alignment: AlignmentType.CENTER,
  spacing: { before: 120, after: 120 },
  children: [new TextRun({ text, font: TNR, size: BODY_SIZE, ...(opts.run || {}) })]
});

// table borders
const bdr = { style: BorderStyle.SINGLE, size: 8, color: "000000" };
const borders = { top: bdr, bottom: bdr, left: bdr, right: bdr };
const headerShade = { fill: "D9D9D9", type: ShadingType.CLEAR };

const cell = (text, opts = {}) => new TableCell({
  borders,
  width: { size: opts.width || Math.floor(CONTENT_W / (opts.cols || 3)), type: WidthType.DXA },
  shading: opts.header ? headerShade : undefined,
  margins: { top: 80, bottom: 80, left: 120, right: 120 },
  verticalAlign: VerticalAlign.CENTER,
  children: [new Paragraph({
    alignment: AlignmentType.CENTER,
    children: [new TextRun({ text, font: TNR, size: BODY_SIZE, bold: !!opts.header })]
  })]
});

const row = (cells) => new TableRow({ children: cells });

// ─────────────────────────── DOCUMENT ───────────────────────────
const doc = new Document({
  numbering: {
    config: [
      {
        reference: "bullets",
        levels: [
          { level: 0, format: LevelFormat.BULLET, text: "\u2022", alignment: AlignmentType.LEFT,
            style: { paragraph: { indent: { left: 720, hanging: 360 } }, run: { font: "Symbol", size: BODY_SIZE } } },
          { level: 1, format: LevelFormat.BULLET, text: "o", alignment: AlignmentType.LEFT,
            style: { paragraph: { indent: { left: 1080, hanging: 360 } }, run: { font: "Courier New", size: BODY_SIZE } } }
        ]
      },
      {
        reference: "numbers",
        levels: [
          { level: 0, format: LevelFormat.DECIMAL, text: "%1.", alignment: AlignmentType.LEFT,
            style: { paragraph: { indent: { left: 720, hanging: 360 } } } },
          { level: 1, format: LevelFormat.LOWER_LETTER, text: "%2.", alignment: AlignmentType.LEFT,
            style: { paragraph: { indent: { left: 1080, hanging: 360 } } } }
        ]
      }
    ]
  },
  styles: {
    default: { document: { run: { font: TNR, size: BODY_SIZE } } },
    paragraphStyles: [
      { id: "Heading1", name: "Heading 1", basedOn: "Normal", next: "Normal", quickFormat: true,
        run: { size: H1_SIZE, bold: true, font: TNR, color: "000000" },
        paragraph: { spacing: { before: 320, after: 160 }, outlineLevel: 0 } },
      { id: "Heading2", name: "Heading 2", basedOn: "Normal", next: "Normal", quickFormat: true,
        run: { size: H2_SIZE, bold: true, font: TNR, color: "000000" },
        paragraph: { spacing: { before: 240, after: 120 }, outlineLevel: 1 } },
      { id: "Heading3", name: "Heading 3", basedOn: "Normal", next: "Normal", quickFormat: true,
        run: { size: H3_SIZE, bold: true, font: TNR, color: "000000" },
        paragraph: { spacing: { before: 200, after: 80 }, outlineLevel: 2 } },
    ]
  },
  sections: [
    // ═══════════════ TITLE PAGE ═══════════════
    {
      properties: {
        page: {
          size: { width: 12240, height: 15840 },
          margin: { top: 1440, right: 1440, bottom: 1440, left: 1440 }
        }
      },
      children: [
        space(720),
        new Paragraph({ alignment: AlignmentType.CENTER, spacing: { before: 0, after: 200 },
          children: [new TextRun({ text: "Jaypee Institute of Information Technology, Noida", font: TNR, size: 32, bold: true })] }),
        new Paragraph({ alignment: AlignmentType.CENTER, spacing: { before: 0, after: 480 },
          children: [new TextRun({ text: "Department of Computer Science and Engineering", font: TNR, size: 26 })] }),
        space(200),
        new Paragraph({ alignment: AlignmentType.CENTER, spacing: { before: 0, after: 160 },
          children: [new TextRun({ text: "PROJECT REPORT", font: TNR, size: 36, bold: true })] }),
        new Paragraph({ alignment: AlignmentType.CENTER, spacing: { before: 0, after: 400 },
          children: [new TextRun({ text: "Design and Analysis of Algorithms Lab (15B11HS1122)", font: TNR, size: 24 })] }),
        space(200),
        new Paragraph({ alignment: AlignmentType.CENTER, spacing: { before: 0, after: 120 },
          children: [new TextRun({ text: "Algorithmic Document Similarity Analyzer", font: TNR, size: 34, bold: true })] }),
        space(400),
        new Paragraph({ alignment: AlignmentType.CENTER, spacing: { before: 0, after: 80 },
          children: [new TextRun({ text: "Submitted By:", font: TNR, size: BODY_SIZE, bold: true })] }),
        new Paragraph({ alignment: AlignmentType.CENTER, spacing: { before: 40, after: 40 },
          children: [new TextRun({ text: "Shreyansh Rajat\t2401030020", font: TNR, size: BODY_SIZE })] }),
        new Paragraph({ alignment: AlignmentType.CENTER, spacing: { before: 40, after: 40 },
          children: [new TextRun({ text: "Aman Saxena\t\t2401030017", font: TNR, size: BODY_SIZE })] }),
        new Paragraph({ alignment: AlignmentType.CENTER, spacing: { before: 40, after: 40 },
          children: [new TextRun({ text: "Aditya Pandey\t2401030027", font: TNR, size: BODY_SIZE })] }),
        new Paragraph({ alignment: AlignmentType.CENTER, spacing: { before: 40, after: 40 },
          children: [new TextRun({ text: "Darsh Tiwari\t\t2401030011", font: TNR, size: BODY_SIZE })] }),
        new Paragraph({ alignment: AlignmentType.CENTER, spacing: { before: 40, after: 40 },
          children: [new TextRun({ text: "Mayank Garg\t\t2401030001", font: TNR, size: BODY_SIZE })] }),
        space(200),
        new Paragraph({ alignment: AlignmentType.CENTER, spacing: { before: 80, after: 80 },
          children: [new TextRun({ text: "Submitted To:", font: TNR, size: BODY_SIZE, bold: true })] }),
        new Paragraph({ alignment: AlignmentType.CENTER, spacing: { before: 40, after: 40 },
          children: [new TextRun({ text: "Dr. Aastha Maheshwari", font: TNR, size: BODY_SIZE })] }),
        new Paragraph({ alignment: AlignmentType.CENTER, spacing: { before: 40, after: 40 },
          children: [new TextRun({ text: "Dr. Kirti Jain", font: TNR, size: BODY_SIZE })] }),
        new Paragraph({ alignment: AlignmentType.CENTER, spacing: { before: 40, after: 40 },
          children: [new TextRun({ text: "Dr. Rajiv Mishra", font: TNR, size: BODY_SIZE })] }),
        new Paragraph({ alignment: AlignmentType.CENTER, spacing: { before: 40, after: 40 },
          children: [new TextRun({ text: "Dr. Taj Alam", font: TNR, size: BODY_SIZE })] }),
        new Paragraph({ alignment: AlignmentType.CENTER, spacing: { before: 40, after: 240 },
          children: [new TextRun({ text: "Dr. Tarun Agrawal", font: TNR, size: BODY_SIZE })] }),
        space(200),
        new Paragraph({ alignment: AlignmentType.CENTER, spacing: { before: 0, after: 60 },
          children: [new TextRun({ text: "BTech CSE | 4th Semester, 2nd Year | SDF Lab-2", font: TNR, size: BODY_SIZE })] }),
        new Paragraph({ alignment: AlignmentType.CENTER, spacing: { before: 0, after: 0 },
          children: [new TextRun({ text: "Academic Year 2024\u20132025", font: TNR, size: BODY_SIZE })] }),
        pageBreak(),
      ]
    },

    // ═══════════════ MAIN CONTENT ═══════════════
    {
      properties: {
        page: {
          size: { width: 12240, height: 15840 },
          margin: { top: 1440, right: 1440, bottom: 1440, left: 1440 }
        }
      },
      footers: {
        default: new Footer({
          children: [new Paragraph({
            alignment: AlignmentType.CENTER,
            children: [
              new TextRun({ text: "Page ", font: TNR, size: BODY_SIZE }),
              new TextRun({ children: [PageNumber.CURRENT], font: TNR, size: BODY_SIZE }),
              new TextRun({ text: " | Algorithmic Document Similarity Analyzer", font: TNR, size: BODY_SIZE })
            ]
          })]
        })
      },
      children: [
        // ── TABLE OF CONTENTS ──
        h1("Table of Contents"),
        body("1.  Summary ............................................................. 3"),
        body("2.  Introduction .......................................................... 3"),
        body("3.  Problem Statement ..................................................... 4"),
        body("4.  Project Objectives .................................................... 4"),
        body("5.  Scope of the Project .................................................. 5"),
        body("6.  System Requirements ................................................... 5"),
        body("7.  Design and Implementation ............................................. 6"),
        body("    7.1  Architecture and UML Overview ..................................... 6"),
        body("    7.2  Preprocessing Module .............................................. 6"),
        body("    7.3  KMP Algorithm ..................................................... 7"),
        body("    7.4  Rabin-Karp Algorithm .............................................. 8"),
        body("    7.5  Suffix Arrays and Kasai\u2019s LCP .................................... 9"),
        body("    7.6  Longest Common Subsequence (LCS) DP .............................. 10"),
        body("    7.7  Trie and Divide-and-Conquer Corpus Search ........................ 11"),
        body("    7.8  Backend Orchestration (FastAPI) ................................... 12"),
        body("    7.9  Frontend (React) .................................................. 12"),
        body("8.  Algorithm Complexities ............................................... 13"),
        body("9.  Output Screenshots and Descriptions .................................. 13"),
        body("10. Conclusion .......................................................... 14"),
        body("11. References .......................................................... 15"),
        pageBreak(),

        // ── 3. SUMMARY ──
        h1("3. Summary"),
        body("The Algorithmic Document Similarity Analyzer is a full-stack application built to detect and quantify similarity between text documents using classical string matching and dynamic programming algorithms. The system exposes two core modes of operation: a one-on-one document comparison mode and a corpus search mode that scans a query document against a large database of files."),
        body("Heavy algorithmic computation is implemented entirely in optimized C++17, ensuring maximum performance and memory efficiency. A Python FastAPI server orchestrates the C++ binaries and exposes RESTful endpoints, while a React 19 frontend provides an interactive user interface with real-time visualizations, complexity tables, and highlighted text viewers."),
        body("The project integrates five complementary algorithms — KMP, Rabin-Karp, Suffix Arrays with Kasai\u2019s LCP, Longest Common Subsequence DP, and Divide-and-Conquer Corpus Search with Tries — each contributing a distinct perspective on text similarity. Together they form a multi-layered plagiarism detection engine that is both educationally rich and practically functional."),
        space(),

        // ── 4. INTRODUCTION ──
        h1("4. Introduction"),
        h2("Background and Context"),
        body("With the exponential growth of digital academic content, ensuring document originality has become increasingly challenging. Students, researchers, and content creators now have access to vast repositories of information, making it easier to reproduce or adapt existing material without proper attribution. Manual comparison of documents is entirely impractical at scale, creating a strong need for automated, algorithmic approaches."),
        body("Plagiarism detection is a well-studied problem in computer science that draws on a rich variety of techniques from string processing, hashing, indexing, and dynamic programming. Tools such as Turnitin and iThenticate operate at a commercial scale, but their internal algorithms are opaque. This project builds a transparent, explainable system from first principles, using only algorithms covered in the Design and Analysis of Algorithms curriculum."),
        body("The system is designed to be educational as well as functional. At every stage of the pipeline, the UI reveals the exact algorithmic decision being made, the time and space complexity of the computation, and the specific matched tokens, giving users a genuine understanding of how each algorithm contributes to the final similarity score."),
        h2("Motivation"),
        body("Academic institutions require reliable, fast tools to evaluate the originality of submitted work. Commercial plagiarism checkers provide results but rarely explain their internal workings. By building a system whose every component is a named, studied algorithm, this project bridges the gap between theoretical computer science education and practical software engineering. It demonstrates that fundamental data structures and algorithms are not abstract exercises but the literal engines that power real-world software."),
        pageBreak(),

        // ── 4. PROBLEM STATEMENT ──
        h1("4. Problem Statement"),
        body("With the rapid growth of digital academic and textual content, manually comparing documents to determine similarity has become impractical and inefficient. Educational institutions, research environments, and content management systems require automated tools that can evaluate textual similarity accurately and efficiently across large collections of documents."),
        body("The specific problem addressed in this project is the design and implementation of an Algorithmic Document Similarity Analyzer that can:"),
        bullet("Accept a query document as input from the user."),
        bullet("Compare it against another document or against a corpus of source documents."),
        bullet("Detect both exact (verbatim) and paraphrased copied content."),
        bullet("Generate precise similarity scores using a weighted algorithmic formula."),
        bullet("Rank the most similar corpus documents by their refined similarity percentage."),
        bullet("Present all results in an explainable, visually rich interface with per-algorithm breakdowns."),
        space(),
        body("The challenge lies not merely in detecting similarity, but in doing so efficiently across large corpora, handling edge cases such as stop-word insertion, case variation, and punctuation tricks, and providing a decomposed explanation of the result rather than a single opaque score."),
        pageBreak(),

        // ── PROJECT OBJECTIVES ──
        h1("4. Project Objectives"),
        body("The following objectives guided the design and development of the system:"),
        numbered("Implement exact phrase detection using Knuth-Morris-Pratt (KMP) and Rabin-Karp algorithms operating at O(N + M) average time complexity."),
        numbered("Implement the longest common substring detection using Suffix Arrays with Kasai\u2019s LCP algorithm at O(N log\u00b2 N) construction time."),
        numbered("Implement paraphrase detection using Longest Common Subsequence Dynamic Programming at O(N \u00d7 M) time."),
        numbered("Build an efficient corpus search engine using Divide-and-Conquer pruning, Trie indexing, and Binary Search on sorted fingerprint hashes."),
        numbered("Develop a FastAPI orchestration layer that spawns parallel C++ subprocesses and aggregates results into a unified weighted similarity score."),
        numbered("Build a React frontend that visualizes algorithmic results, including matched phrase highlights, a similarity gauge, DP heatmap samples, and algorithm complexity tables."),
        numbered("Ensure the system is explainable: every result must be traceable to a specific algorithm and a specific set of matched tokens or positions."),
        pageBreak(),

        // ── SCOPE ──
        h1("5. Scope of the Project"),
        body("The system operates on plain-text (.txt) and Markdown (.md) documents. The scope is defined along two axes:"),
        h3("In Scope"),
        bullet("One-on-one document comparison with four parallel algorithmic analyses."),
        bullet("Corpus search against a pre-loaded set of text documents using hashing, Tries, and Divide-and-Conquer pruning."),
        bullet("Visual output including similarity gauges, matched text highlighting at the character-position level, DP table heatmaps, and algorithm complexity cards."),
        bullet("A REST API backend exposing /api/compare and /api/corpus endpoints."),
        bullet("Support for Windows (compiled .exe binaries) and Unix environments."),
        h3("Out of Scope"),
        bullet("PDF or binary document parsing (no OCR or PDF text extraction)."),
        bullet("Real-time collaborative editing or multi-user session management."),
        bullet("Machine-learning-based semantic similarity (e.g., BERT embeddings)."),
        bullet("Persistent database storage of comparison history."),
        pageBreak(),

        // ── SYSTEM REQUIREMENTS ──
        h1("6. System Requirements"),
        h2("Functional Requirements"),
        bullet("The system shall accept two text files and return a composite similarity score."),
        bullet("The system shall run KMP, Rabin-Karp, Suffix Array, and LCS analyses in parallel."),
        bullet("The system shall aggregate algorithm outputs using the weighted formula: KMP 25%, Rabin-Karp 25%, Suffix Array 20%, LCS 30%."),
        bullet("The system shall detect byte-for-byte identical files and return a score of 100%."),
        bullet("The system shall index a corpus directory and support query-document search with Divide-and-Conquer pruning."),
        bullet("The system shall rank top-5 corpus matches using refined Suffix Array and LCS re-evaluation."),
        h2("Non-Functional Requirements"),
        bullet("Performance: All C++ algorithms must complete within 60 seconds per comparison."),
        bullet("Accuracy: Hash collision resolution must guarantee zero false positives in Rabin-Karp."),
        bullet("Usability: The React UI must display results within 3 seconds of API response."),
        bullet("Portability: The system must run on Windows 10+ with GCC/MinGW and on Ubuntu 22.04+."),
        h2("Technology Stack"),
        new Table({
          width: { size: CONTENT_W, type: WidthType.DXA },
          columnWidths: [2500, 3200, 3660],
          rows: [
            row([cell("Layer", { header: true, cols: 3, width: 2500 }), cell("Technology", { header: true, cols: 3, width: 3200 }), cell("Purpose", { header: true, cols: 3, width: 3660 })]),
            row([cell("Core Engine", { cols: 3, width: 2500 }), cell("C++17 (GCC)", { cols: 3, width: 3200 }), cell("All string algorithms and data structures", { cols: 3, width: 3660 })]),
            row([cell("Orchestrator", { cols: 3, width: 2500 }), cell("Python 3.9+, FastAPI", { cols: 3, width: 3200 }), cell("API server, subprocess management", { cols: 3, width: 3660 })]),
            row([cell("Frontend", { cols: 3, width: 2500 }), cell("React 19, Vite, Tailwind", { cols: 3, width: 3200 }), cell("Interactive UI and visualizations", { cols: 3, width: 3660 })]),
            row([cell("Charting", { cols: 3, width: 2500 }), cell("Recharts", { cols: 3, width: 3200 }), cell("Gauge and heatmap rendering", { cols: 3, width: 3660 })]),
          ]
        }),
        pageBreak(),

        // ── DESIGN AND IMPLEMENTATION ──
        h1("7. Design and Implementation"),
        h2("7.1 Architecture and UML Overview"),
        body("The system follows a three-tier architecture. The React frontend communicates exclusively with the FastAPI backend over HTTP. The FastAPI backend acts as a pure orchestrator: it saves uploaded files to a temporary directory and spawns independent C++ subprocesses for each algorithm. The C++ binaries read files directly from disk, perform their computations, and output structured JSON to stdout, which FastAPI parses and aggregates. No direct communication occurs between the frontend and the C++ layer."),
        body("The class structure of the C++ layer can be conceptually modelled as three cooperating entities: a Document class (holding raw text and tokens), a Preprocessor class (exposing a tokenize() method), and a SimilarityEngine class (exposing KMPmatch(), RabinKarpMatch(), computeLCS(), buildSuffixArray(), and corpusSearch() methods). Each algorithm is compiled as a standalone executable to ensure maximal isolation and testability."),
        h2("7.2 Preprocessing Module (preprocessor.cpp)"),
        body("Before any algorithmic analysis, all text passes through a standardization pipeline implemented in preprocessor.cpp. The module performs the following transformations in sequence:"),
        numbered("Case normalization: Every character is converted to lowercase using direct ASCII arithmetic (c + 32 for uppercase letters), avoiding locale-dependent library calls."),
        numbered("Punctuation stripping: All non-alphanumeric characters except whitespace are removed, ensuring that punctuation differences between documents do not create false negatives."),
        numbered("Stop-word removal: Common English function words (the, and, is, of, etc.) are removed from the token stream. This prevents high-frequency stop words from inflating similarity scores and focuses the algorithms on semantically meaningful content words."),
        numbered("Tokenization: The cleaned text is split on whitespace into a vector of string tokens, which are then passed to the downstream algorithm modules."),
        body("The preprocessor outputs a single JSON object containing the cleaned text string, the token array, and the character count. All downstream C++ binaries reuse equivalent preprocessing logic internally, ensuring consistency across algorithms."),
        h2("7.3 KMP Algorithm (kmp.cpp)"),
        body("The Knuth-Morris-Pratt algorithm is used for exact phrase detection. The implementation extracts contiguous phrases of 3 to 8 words from Document A. For each phrase, it constructs the Longest Proper Prefix which is also Suffix (LPS) array, also known as the failure function. The LPS array at index i stores the length of the longest proper prefix of the pattern that is also a suffix of the pattern\u2019s prefix ending at i."),
        h3("Failure Function Construction"),
        body("The failure function is computed in O(M) time where M is the pattern length. Starting from index 1 with a \u201clen\u201d pointer at 0, the algorithm compares pattern[i] with pattern[len]. A match increments both; a mismatch with len > 0 resets len to lps[len - 1] without advancing i, exploiting the known prefix-suffix overlap to avoid redundant comparisons."),
        h3("Search Phase"),
        body("Two pointers i (over text, length N) and j (over pattern, length M) traverse simultaneously. On a character match, both advance. When j reaches M, a complete match is recorded at position (i - j), and j resets to lps[j - 1] to allow overlapping matches. On a mismatch with j > 0, j resets to lps[j - 1]; with j = 0, only i advances. The total time complexity is strictly O(N + M) per pattern."),
        h3("Phrase Scoring"),
        body("Matches shorter than 10 characters are discarded as noise. The remaining matches are sorted by length descending, and a greedy overlap-elimination step ensures that a given character position in Document B is attributed to at most one matched phrase. The similarity contribution is computed as the ratio of matched characters to the shorter document\u2019s length. KMP contributes 25% to the final weighted score."),
        h2("7.4 Rabin-Karp Algorithm (rabin_karp.cpp)"),
        body("Rabin-Karp uses a rolling polynomial hash to detect multi-pattern matches rapidly. The implementation models each window of text as a number in base B = 31 modulo a large prime MOD = 10\u2079 + 7, treating each character\u2019s value as a digit. This parameterization minimizes the probability of hash collisions while keeping arithmetic within 64-bit integers."),
        h3("Rolling Hash Computation"),
        body("For a window of length W, the initial hash is computed in O(W) time. When the window slides by one character, the outgoing character\u2019s contribution (multiplied by B^(W-1) mod MOD) is subtracted, the remaining hash is multiplied by B, and the incoming character\u2019s value is added\u2014all in O(1) time. This allows scanning an entire document of length N in O(N) time regardless of window size."),
        h3("Collision Resolution"),
        body("If two windows\u2019 hashes match, a secondary character-by-character verification is always performed before reporting a match. This guarantees zero false positives. The probability of an undetected false positive (two distinct windows with the same hash, where the collision is not caught) is bounded by the birthday paradox over the modulus, which is negligible for documents of typical academic length."),
        h3("Multi-Pattern Scanning"),
        body("The implementation generates hash fingerprints for all shingles (sliding windows) of Document A and stores them in an unordered_set for O(1) average lookup. It then slides through Document B and checks each window\u2019s hash against this set. This makes multi-pattern matching extremely fast in practice. Rabin-Karp contributes 25% to the final weighted score."),
        h2("7.5 Suffix Arrays and Kasai\u2019s LCP (suffix_array.cpp)"),
        body("Suffix arrays provide the most powerful exact-match detection in the system, identifying the absolute longest contiguous substring shared between two documents. The implementation concatenates the cleaned texts of Document A and B with a special separator character (#) that does not appear in any token, producing a single combined string S of length N."),
        h3("Suffix Array Construction"),
        body("A naive approach of sorting all N suffixes of S would require O(N\u00b2 log N) time due to O(N) string comparisons. The implementation uses the prefix-doubling (also called the DC3 or Skew-inspired doubling) method, which achieves O(N log\u00b2 N). In each doubling iteration k, suffixes are ranked by their first 2^k characters using the ranks from the previous iteration, requiring O(N log N) time per iteration and O(log N) iterations total."),
        h3("Kasai\u2019s LCP Array"),
        body("Once the suffix array SA is built, Kasai\u2019s algorithm computes the Longest Common Prefix (LCP) array in O(N) time. The key insight is that if the suffix starting at position i has an LCP of h with its neighbour in the sorted array, then the suffix starting at position i+1 must have an LCP of at least h-1 with its own neighbour. This monotonicity property allows LCP[i] to be computed by extending from the previous value rather than recomputing from scratch."),
        h3("Cross-Document Substring Extraction"),
        body("Adjacent entries in the suffix array that originate from different documents (one index < separator position, one index > separator position) with a high LCP value represent long shared substrings. The implementation scans adjacent pairs in the suffix array, filtering for cross-document pairs and retaining those with LCP values above a minimum threshold. The longest such substrings are returned with their positions in both documents. Suffix Array contributes 20% to the final weighted score."),
        h2("7.6 Longest Common Subsequence DP (lcs.cpp)"),
        body("LCS is the system\u2019s paraphrase detection mechanism. Unlike KMP and Rabin-Karp, LCS does not require matched words to be contiguous, making it robust against the \u201cinjected filler word\u201d plagiarism strategy where a student inserts common words between copied sentences to defeat exact-match detectors."),
        h3("DP Table Construction"),
        body("The input to LCS is the token arrays of both documents: A of length N tokens and B of length M tokens. An (N+1) \u00d7 (M+1) DP table is allocated. The recurrence is:"),
        body("  dp[i][j] = dp[i-1][j-1] + 1    if token A[i] == token B[j]"),
        body("  dp[i][j] = max(dp[i-1][j], dp[i][j-1])    otherwise"),
        body("This bottom-up construction runs in O(N \u00d7 M) time and O(N \u00d7 M) space. For large documents, the space can be reduced to O(min(N, M)) if only the final LCS length is needed, but the implementation retains the full table to support backtracking and heatmap visualization."),
        h3("Backtracking and Heatmap"),
        body("After the table is filled, the algorithm backtracks from dp[N][M] following diagonal moves (token matches) to reconstruct the exact sequence of matched tokens. A sample of the DP table (up to 20 \u00d7 20 tokens) is serialized to JSON and returned to the frontend, where it is rendered as a colour-coded heatmap showing regions of high and low similarity. The LCS length divided by the length of the longer document gives the LCS similarity ratio, contributing 30% to the final weighted score\u2014the highest weight of any algorithm, reflecting paraphrase detection\u2019s importance."),
        h2("7.7 Trie and Divide-and-Conquer Corpus Search (corpus_search.cpp)"),
        body("Corpus search introduces two additional data structures\u2014a Trie and a Min-Heap\u2014and a Divide-and-Conquer pruning strategy for efficient large-scale retrieval."),
        h3("Corpus Indexing at Startup"),
        body("When the application initializes, every document in the corpus/ directory is processed. Each document is tokenized and every token is inserted into a global Trie (prefix tree). Each node in the Trie stores its character, a map of child pointers, and a list of document IDs that contain the corresponding prefix. Simultaneously, a rolling hash fingerprint (set of shingles) is generated for each document and stored as a sorted integer array, enabling binary search."),
        h3("Divide-and-Conquer Pruning"),
        body("Given a query document Q with a set of hash fingerprints H_Q, the corpus documents are sorted by their median primary hash. The divide-and-conquer algorithm splits the corpus array in half recursively. For each half, it constructs a unified unordered_set of all hash fingerprints belonging to documents in that half and counts how many of Q\u2019s fingerprints appear in it (the overlap). If the overlap fraction falls below a strict threshold (0.005), the entire half is pruned in O(1) time\u2014its documents are eliminated without individual inspection. This exponentially reduces the number of documents requiring detailed evaluation in practice, making corpus search sub-linear in the number of documents for queries that are genuinely distinct from most of the corpus."),
        h3("Binary Search Jaccard Evaluation"),
        body("For documents that survive pruning, their sorted fingerprint arrays allow O(log N) binary search for each of Q\u2019s fingerprints. The intersection count divided by the union size gives the Jaccard similarity coefficient. This provides a fast, mathematically principled similarity ranking without full string comparison."),
        h3("Priority Queue Ranking"),
        body("A C++ Min-Heap (std::priority_queue) maintains the top-K documents by Jaccard score. After initial hash-based ranking, the top 5 documents are re-evaluated using the full Suffix Array and LCS pipeline to produce high-precision final scores. The final ranked list, along with pruning statistics (documents pruned, time saved), is returned to the frontend."),
        h2("7.8 Backend Orchestration (main.py)"),
        body("The FastAPI server in main.py implements three key endpoints. The GET /api/health endpoint checks that all six compiled C++ binaries exist. The POST /api/compare endpoint saves both uploaded files to the uploads/ temporary directory and uses Python\u2019s subprocess module to spawn four C++ processes in parallel (using asyncio.gather in the async context). Each process is given a 60-second timeout. The JSON outputs are parsed and aggregated using the weighted formula. If the two uploaded files are detected as byte-for-byte identical before running the algorithms, the score is immediately overridden to 100% and the algorithm results are still populated for educational display. The GET /api/corpus endpoint invokes corpus_search.exe with the query file path and returns the ranked list of matches. Temporary files are deleted in a finally block after every request to prevent disk accumulation."),
        h2("7.9 Frontend (React)"),
        body("The React frontend is organized into five components. App.jsx manages global state and routing between the two modes. CompareMode.jsx handles the two-file upload form and renders the full comparison result. CorpusMode.jsx handles the single-file upload and displays the ranked corpus matches. SimilarityGauge.jsx renders an animated radial gauge using Recharts. DocumentViewer.jsx receives the character-position arrays from KMP and Rabin-Karp and applies inline highlighting to the raw document text. Visualizations.jsx renders the LCS DP heatmap sample and the algorithm complexity comparison table. The api.js module handles all HTTP communication with the FastAPI backend using the Fetch API."),
        pageBreak(),

        // ── ALGORITHM COMPLEXITIES ──
        h1("8. Algorithm Complexities"),
        new Table({
          width: { size: CONTENT_W, type: WidthType.DXA },
          columnWidths: [2400, 1600, 1600, 1600, 2160],
          rows: [
            row([
              cell("Algorithm", { header: true, width: 2400 }),
              cell("Time Complexity", { header: true, width: 1600 }),
              cell("Space Complexity", { header: true, width: 1600 }),
              cell("Weight", { header: true, width: 1600 }),
              cell("Primary Use", { header: true, width: 2160 })
            ]),
            row([
              cell("KMP", { width: 2400 }),
              cell("O(N + M) per pattern", { width: 1600 }),
              cell("O(M) — LPS array", { width: 1600 }),
              cell("25%", { width: 1600 }),
              cell("Exact phrase detection", { width: 2160 })
            ]),
            row([
              cell("Rabin-Karp", { width: 2400 }),
              cell("O(N + M) average", { width: 1600 }),
              cell("O(N) — hash set", { width: 1600 }),
              cell("25%", { width: 1600 }),
              cell("Multi-pattern / fast scan", { width: 2160 })
            ]),
            row([
              cell("Suffix Array + Kasai LCP", { width: 2400 }),
              cell("O(N log\u00b2 N) build", { width: 1600 }),
              cell("O(N) — SA + rank", { width: 1600 }),
              cell("20%", { width: 1600 }),
              cell("Longest common substring", { width: 2160 })
            ]),
            row([
              cell("LCS DP", { width: 2400 }),
              cell("O(N \u00d7 M) tokens", { width: 1600 }),
              cell("O(N \u00d7 M) — table", { width: 1600 }),
              cell("30%", { width: 1600 }),
              cell("Paraphrase detection", { width: 2160 })
            ]),
            row([
              cell("Trie + D&C Pruning", { width: 2400 }),
              cell("Sub-linear amortised", { width: 1600 }),
              cell("O(corpus \u00d7 tokens)", { width: 1600 }),
              cell("N/A (corpus)", { width: 1600 }),
              cell("Corpus indexing & pruning", { width: 2160 })
            ]),
          ]
        }),
        space(),
        body("The weighting formula reflects each algorithm\u2019s sensitivity to different forms of plagiarism. LCS receives the highest weight (30%) because it detects paraphrase-level similarity, which is the most sophisticated and commonly employed form of academic dishonesty. KMP and Rabin-Karp share equal weight (25% each) as they are complementary exact-match detectors with different internal mechanisms. Suffix Array receives 20% as it provides a precise measure of the longest verbatim copied block, which is strong evidence of direct copying."),
        pageBreak(),

        // ── OUTPUT SCREENSHOTS ──
        h1("9. Output Screenshots and Descriptions"),
        h2("9.1 Landing Page"),
        body("The application home screen presents two prominent mode cards: \u201c1-on-1 Document Comparison\u201d and \u201cCorpus Search.\u201d A brief description under each card explains what type of analysis will be performed. The visual design uses a dark-themed card layout with subtle gradients, built with TailwindCSS utility classes."),
        h2("9.2 One-on-One Comparison \u2014 File Upload"),
        body("The CompareMode view presents two side-by-side drag-and-drop upload zones, one for Document A and one for Document B. Both zones accept .txt and .md files. Once both files are selected, a \u201cRun Algorithmic Comparison\u201d button becomes active. A loading spinner with status text (e.g., \u201cRunning KMP...\u201d) is displayed while the backend processes the request."),
        h2("9.3 Similarity Gauge and Score Breakdown"),
        body("Upon receiving the API response, the main result panel renders a circular Recharts gauge displaying the composite similarity percentage from 0% to 100%, with colour-coded zones (green below 30%, amber 30\u201360%, red above 60%). Below the gauge, four algorithm cards show the individual similarity contribution from KMP, Rabin-Karp, Suffix Array, and LCS, along with their measured runtime in milliseconds and their assigned weight."),
        h2("9.4 Document Viewer with Highlights"),
        body("The DocumentViewer component renders both documents side-by-side. Character positions returned by the KMP and Rabin-Karp modules are used to apply inline <span> elements with background highlighting. Matched phrases appear highlighted in yellow in both documents simultaneously, allowing the user to visually trace exactly which sections triggered each algorithm\u2019s detection."),
        h2("9.5 LCS Heatmap and Complexity Table"),
        body("The Visualizations component renders a colour-coded grid representing a 20 \u00d7 20 sample of the LCS DP table. Cells with high values (close to the LCS length) are rendered in deep blue, while low-value cells are light grey, creating a clear visual pattern of the diagonal path that represents the reconstructed common subsequence. Below the heatmap, an algorithm complexity comparison table lists all five algorithms with their time complexity, space complexity, and detected match count for the current pair."),
        h2("9.6 Corpus Search Results"),
        body("The CorpusMode view displays a ranked list of corpus documents after a query file is uploaded. Each result card shows the document filename, a horizontal progress bar indicating its final similarity percentage, the number of hash fingerprints matched, the Jaccard similarity coefficient, and the length of the longest common substring identified by the Suffix Array refinement pass. A statistics banner at the top shows total documents in corpus, documents pruned, and total search time in milliseconds."),
        pageBreak(),

        // ── CONCLUSION ──
        h1("10. Conclusion"),
        body("The Algorithmic Document Similarity Analyzer successfully demonstrates how a suite of classical computer science algorithms can be integrated into a cohesive, production-quality application for a real-world problem. The project achieves all seven stated objectives: exact phrase detection via KMP and Rabin-Karp, longest-substring identification via Suffix Arrays, paraphrase detection via LCS Dynamic Programming, and efficient corpus-scale retrieval via Divide-and-Conquer pruning and Trie indexing."),
        body("The architectural decision to implement all computationally intensive components in C++17 and expose them through a FastAPI orchestration layer proved highly effective. The strict separation of concerns\u2014C++ for algorithms, Python for coordination, React for presentation\u2014results in a system that is easy to extend: a new algorithm can be added as a new .cpp file without modifying any other layer."),
        body("The project reveals a key insight about algorithm selection in practice: no single algorithm is sufficient for robust plagiarism detection. KMP and Rabin-Karp excel at finding verbatim copies but are defeated by simple word substitution. LCS catches paraphrase-level similarity but produces high scores for structurally similar documents with no copied intent. The combination of all four, weighted by their sensitivity profile, produces a significantly more robust and nuanced similarity assessment than any individual algorithm could provide."),
        body("Future work could extend the system in several directions. Replacing the O(N log\u00b2 N) prefix-doubling suffix array construction with the O(N) SA-IS algorithm would improve scalability for very large documents. Adding a semantic similarity layer using pre-trained word embeddings (such as Word2Vec or FastText) would allow the system to detect synonym-level paraphrase that no character-level algorithm can catch. Support for PDF parsing via pdfplumber would extend the system\u2019s applicability to the most common format for academic submissions. Finally, persisting comparison results in a SQLite database would enable longitudinal analysis and trend reporting across a student cohort."),
        pageBreak(),

        // ── REFERENCES ──
        h1("11. References"),
        numbered("Knuth, D. E., Morris, J. H., and Pratt, V. R. (1977). Fast Pattern Matching in Strings. SIAM Journal on Computing, 6(2), 323\u2013350."),
        numbered("Rabin, M. O., and Karp, R. M. (1987). Efficient Randomized Pattern-Matching Algorithms. IBM Journal of Research and Development, 31(2), 249\u2013260."),
        numbered("Kasai, T., Lee, G., Arimura, H., Arikawa, S., and Park, K. (2001). Linear-Time Longest-Common-Prefix Computation in Suffix Arrays and Its Applications. Proceedings of CPM 2001, LNCS 2089, 181\u2013192."),
        numbered("Cormen, T. H., Leiserson, C. E., Rivest, R. L., and Stein, C. (2022). Introduction to Algorithms, 4th Edition. MIT Press."),
        numbered("GeeksforGeeks. KMP Algorithm for Pattern Searching. https://www.geeksforgeeks.org/dsa/kmp-algorithm-for-pattern-searching/"),
        numbered("GeeksforGeeks. Rabin-Karp Algorithm for Pattern Searching. https://www.geeksforgeeks.org/rabin-karp-algorithm-for-pattern-searching/"),
        numbered("GeeksforGeeks. Suffix Array \u2014 Introduction. https://www.geeksforgeeks.org/suffix-array-set-1-introduction/"),
        numbered("GeeksforGeeks. Trie \u2014 Insert and Search. https://www.geeksforgeeks.org/trie-insert-and-search/"),
        numbered("GeeksforGeeks. Longest Common Subsequence (DP-4). https://www.geeksforgeeks.org/longest-common-subsequence-dp-4/"),
        numbered("FastAPI Documentation. https://fastapi.tiangolo.com/"),
        numbered("React Documentation. https://react.dev/"),
        space(200),
      ]
    }
  ]
});

Packer.toBuffer(doc).then(buf => {
  fs.writeFileSync('D:\\College\\sem-4\\pbls\\DAA\\report\\report.docx', buf);
  console.log('Done');
}).catch(console.error);
