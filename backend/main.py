"""
FastAPI Backend — Algorithmic Document Similarity Analyzer
Handles file uploads, calls C++ algorithm executables, returns JSON responses.
"""

import os
import json
import subprocess
import tempfile
import shutil
import time
from pathlib import Path
from fastapi import FastAPI, UploadFile, File, HTTPException, Query
from fastapi.middleware.cors import CORSMiddleware
from fastapi.responses import JSONResponse

# ===================== CONFIG =====================
BASE_DIR = Path(__file__).parent
BIN_DIR = BASE_DIR / "bin"
CORPUS_DIR = BASE_DIR / "corpus"
UPLOAD_DIR = BASE_DIR / "uploads"
UPLOAD_DIR.mkdir(exist_ok=True)

app = FastAPI(
    title="Algorithmic Document Similarity Analyzer",
    description="DAA-based document comparison using classical algorithms (C++)",
    version="1.0.0",
)

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)


# ===================== HELPERS =====================
def save_upload(file: UploadFile, suffix: str = "") -> Path:
    """Save uploaded file and return path."""
    filename = f"upload_{suffix}_{int(time.time())}_{file.filename}"
    filepath = UPLOAD_DIR / filename
    with open(filepath, "wb") as f:
        content = file.file.read()
        f.write(content)
    return filepath


def run_cpp_module(exe_name: str, args: list, timeout: int = 60) -> dict:
    """Run a C++ executable and return parsed JSON output."""
    exe_path = BIN_DIR / exe_name
    if not exe_path.exists():
        raise HTTPException(status_code=500, detail=f"C++ module not found: {exe_name}")

    cmd = [str(exe_path)] + [str(a) for a in args]
    try:
        result = subprocess.run(
            cmd,
            capture_output=True,
            text=True,
            timeout=timeout,
            cwd=str(BASE_DIR),
        )
        if result.returncode != 0:
            return {"error": f"Module error: {result.stderr.strip()}"}

        output = result.stdout.strip()
        if not output:
            return {"error": "No output from module"}

        return json.loads(output)
    except subprocess.TimeoutExpired:
        return {"error": f"Module timed out after {timeout}s"}
    except json.JSONDecodeError as e:
        return {"error": f"Invalid JSON output: {str(e)}", "raw_output": result.stdout[:500]}
    except Exception as e:
        return {"error": str(e)}


# ===================== ENDPOINTS =====================

@app.get("/")
async def root():
    return {"message": "Algorithmic Document Similarity Analyzer API", "version": "1.0.0"}


@app.get("/api/health")
async def health():
    """Check if all C++ modules are available."""
    modules = ["preprocessor.exe", "kmp.exe", "rabin_karp.exe", "suffix_array.exe", "lcs.exe", "corpus_search.exe"]
    status = {}
    for m in modules:
        status[m] = (BIN_DIR / m).exists()
    return {"status": "ok", "modules": status}


@app.get("/api/corpus")
async def list_corpus():
    """List all documents in the corpus."""
    docs = []
    if CORPUS_DIR.exists():
        for f in sorted(CORPUS_DIR.iterdir()):
            if f.is_file() and f.suffix in [".txt", ".md", ".text"]:
                content = f.read_text(encoding="utf-8", errors="ignore")
                docs.append({
                    "filename": f.name,
                    "size_bytes": f.stat().st_size,
                    "word_count": len(content.split()),
                    "preview": content[:200] + "..." if len(content) > 200 else content,
                })
    return {"corpus_size": len(docs), "documents": docs}


# ===================== MODE 1: Compare Two Documents =====================

@app.post("/api/preprocess")
async def preprocess(file: UploadFile = File(...), ngram_size: int = Query(3, ge=1, le=5)):
    """Preprocess a single document: tokenize, clean, generate n-grams."""
    filepath = save_upload(file, "preprocess")
    try:
        result = run_cpp_module("preprocessor.exe", [str(filepath), str(ngram_size)])
        return result
    finally:
        filepath.unlink(missing_ok=True)


@app.post("/api/compare")
async def compare_documents(
    file_a: UploadFile = File(...),
    file_b: UploadFile = File(...),
):
    """
    Full comparison of two documents using all algorithms.
    Returns combined results from KMP, Rabin-Karp, Suffix Array, and LCS.
    """
    path_a = save_upload(file_a, "a")
    path_b = save_upload(file_b, "b")

    try:
        # Read original texts for highlight info
        text_a = path_a.read_text(encoding="utf-8", errors="ignore")
        text_b = path_b.read_text(encoding="utf-8", errors="ignore")

        # Run all algorithms
        kmp_result = run_cpp_module("kmp.exe", [str(path_a), str(path_b)])
        rabin_karp_result = run_cpp_module("rabin_karp.exe", [str(path_a), str(path_b), "20"])
        suffix_result = run_cpp_module("suffix_array.exe", [str(path_a), str(path_b)])
        lcs_result = run_cpp_module("lcs.exe", [str(path_a), str(path_b)])

        # Preprocess both docs
        preprocess_a = run_cpp_module("preprocessor.exe", [str(path_a), "3"])
        preprocess_b = run_cpp_module("preprocessor.exe", [str(path_b), "3"])

        # Calculate overall similarity (weighted average)
        similarities = []
        is_identical = (text_a.strip() == text_b.strip() and len(text_a.strip()) > 0)
        
        if is_identical:
            similarities = [("kmp", 100.0), ("rabin_karp", 100.0), ("suffix_array", 100.0), ("lcs", 100.0)]
            if isinstance(kmp_result, dict): kmp_result["similarity_percentage"] = 100.0
            if isinstance(rabin_karp_result, dict): rabin_karp_result["similarity_percentage"] = 100.0
            if isinstance(suffix_result, dict): suffix_result["similarity_percentage"] = 100.0
            if isinstance(lcs_result, dict): lcs_result["combined_similarity"] = 100.0
        else:
            if "similarity_percentage" in kmp_result:
                similarities.append(("kmp", kmp_result["similarity_percentage"]))
            if "similarity_percentage" in rabin_karp_result:
                similarities.append(("rabin_karp", rabin_karp_result["similarity_percentage"]))
            if "similarity_percentage" in suffix_result:
                similarities.append(("suffix_array", suffix_result["similarity_percentage"]))
            if "combined_similarity" in lcs_result:
                similarities.append(("lcs", lcs_result["combined_similarity"]))

        weights = {"kmp": 0.25, "rabin_karp": 0.25, "suffix_array": 0.2, "lcs": 0.3}
        overall = 0
        total_weight = 0
        for name, score in similarities:
            w = weights.get(name, 0.25)
            overall += score * w
            total_weight += w
        if total_weight > 0:
            overall /= total_weight
            overall = min(100.0, overall)

        return {
            "overall_similarity": round(overall, 2),
            "text_a": text_a,
            "text_b": text_b,
            "text_a_length": len(text_a),
            "text_b_length": len(text_b),
            "preprocessing": {
                "doc_a": preprocess_a,
                "doc_b": preprocess_b,
            },
            "kmp": kmp_result,
            "rabin_karp": rabin_karp_result,
            "suffix_array": suffix_result,
            "lcs": lcs_result,
            "algorithm_comparison": [
                {
                    "algorithm": "KMP",
                    "time_complexity": "O(n + m)",
                    "space_complexity": "O(m)",
                    "runtime_ms": kmp_result.get("runtime_ms", 0),
                    "similarity": kmp_result.get("similarity_percentage", 0),
                },
                {
                    "algorithm": "Rabin-Karp",
                    "time_complexity": "O(n + m) avg",
                    "space_complexity": "O(n)",
                    "runtime_ms": rabin_karp_result.get("runtime_ms", 0),
                    "similarity": rabin_karp_result.get("similarity_percentage", 0),
                },
                {
                    "algorithm": "Suffix Array + LCP",
                    "time_complexity": "O(n log²n)",
                    "space_complexity": "O(n)",
                    "runtime_ms": suffix_result.get("runtime_ms", 0),
                    "similarity": suffix_result.get("similarity_percentage", 0),
                },
                {
                    "algorithm": "LCS (DP)",
                    "time_complexity": "O(n × m)",
                    "space_complexity": "O(n × m)",
                    "runtime_ms": lcs_result.get("runtime_ms", 0),
                    "similarity": lcs_result.get("combined_similarity", 0),
                },
            ],
        }
    finally:
        path_a.unlink(missing_ok=True)
        path_b.unlink(missing_ok=True)


@app.post("/api/kmp")
async def kmp_compare(file_a: UploadFile = File(...), file_b: UploadFile = File(...)):
    """Run only KMP algorithm on two documents."""
    path_a = save_upload(file_a, "a")
    path_b = save_upload(file_b, "b")
    try:
        return run_cpp_module("kmp.exe", [str(path_a), str(path_b)])
    finally:
        path_a.unlink(missing_ok=True)
        path_b.unlink(missing_ok=True)


@app.post("/api/rabin-karp")
async def rabin_karp_compare(
    file_a: UploadFile = File(...),
    file_b: UploadFile = File(...),
    window_size: int = Query(20, ge=5, le=100),
):
    """Run only Rabin-Karp algorithm on two documents."""
    path_a = save_upload(file_a, "a")
    path_b = save_upload(file_b, "b")
    try:
        return run_cpp_module("rabin_karp.exe", [str(path_a), str(path_b), str(window_size)])
    finally:
        path_a.unlink(missing_ok=True)
        path_b.unlink(missing_ok=True)


@app.post("/api/suffix-array")
async def suffix_array_compare(file_a: UploadFile = File(...), file_b: UploadFile = File(...)):
    """Run only Suffix Array + LCP algorithm on two documents."""
    path_a = save_upload(file_a, "a")
    path_b = save_upload(file_b, "b")
    try:
        return run_cpp_module("suffix_array.exe", [str(path_a), str(path_b)])
    finally:
        path_a.unlink(missing_ok=True)
        path_b.unlink(missing_ok=True)


@app.post("/api/lcs")
async def lcs_compare(file_a: UploadFile = File(...), file_b: UploadFile = File(...)):
    """Run only LCS algorithm on two documents."""
    path_a = save_upload(file_a, "a")
    path_b = save_upload(file_b, "b")
    try:
        return run_cpp_module("lcs.exe", [str(path_a), str(path_b)])
    finally:
        path_a.unlink(missing_ok=True)
        path_b.unlink(missing_ok=True)


# ===================== MODE 2: Corpus Search =====================

@app.post("/api/corpus-search")
async def corpus_search(
    file: UploadFile = File(...),
    top_k: int = Query(10, ge=1, le=50),
):
    """Compare uploaded document against the corpus."""
    filepath = save_upload(file, "query")
    try:
        # Run corpus search
        corpus_result = run_cpp_module(
            "corpus_search.exe",
            [str(filepath), str(CORPUS_DIR), str(top_k)],
        )

        # For each top match, run detailed comparison
        if "results" in corpus_result and not corpus_result.get("error"):
            detailed_results = []
            for match in corpus_result["results"][:5]:  # detailed for top 5
                corpus_file = CORPUS_DIR / match["filename"]
                if corpus_file.exists():
                    # Run LCS for paraphrase detection
                    lcs_detail = run_cpp_module("lcs.exe", [str(filepath), str(corpus_file)])
                    # Run suffix array for longest common substring
                    sa_detail = run_cpp_module("suffix_array.exe", [str(filepath), str(corpus_file)])

                    corpus_text = corpus_file.read_text(encoding="utf-8", errors="ignore")

                    detailed_results.append({
                        "filename": match["filename"],
                        "fingerprint_similarity": match["similarity_percentage"],
                        "lcs_similarity": lcs_detail.get("combined_similarity", 0),
                        "longest_common_substring": sa_detail.get("longest_common_substring", ""),
                        "longest_substring_length": sa_detail.get("longest_length", 0),
                        "shared_phrases": match.get("shared_phrases", []),
                        "corpus_text_preview": corpus_text[:300],
                        "lcs_detail": lcs_detail,
                        "suffix_detail": sa_detail,
                    })

            corpus_result["detailed_results"] = detailed_results

        # Add query doc info
        query_text = filepath.read_text(encoding="utf-8", errors="ignore")
        corpus_result["query_text"] = query_text
        corpus_result["query_preview"] = query_text[:300]

        return corpus_result
    finally:
        filepath.unlink(missing_ok=True)


if __name__ == "__main__":
    import uvicorn
    uvicorn.run(app, host="0.0.0.0", port=8000)
