const API_BASE = '/api';

export async function checkHealth() {
  const res = await fetch(`${API_BASE}/health`);
  return res.json();
}

export async function listCorpus() {
  const res = await fetch(`${API_BASE}/corpus`);
  return res.json();
}

export async function preprocessFile(file, ngramSize = 3) {
  const formData = new FormData();
  formData.append('file', file);
  const res = await fetch(`${API_BASE}/preprocess?ngram_size=${ngramSize}`, {
    method: 'POST',
    body: formData,
  });
  return res.json();
}

export async function compareDocuments(fileA, fileB) {
  const formData = new FormData();
  formData.append('file_a', fileA);
  formData.append('file_b', fileB);
  const res = await fetch(`${API_BASE}/compare`, {
    method: 'POST',
    body: formData,
  });
  return res.json();
}

export async function corpusSearch(file, topK = 10) {
  const formData = new FormData();
  formData.append('file', file);
  const res = await fetch(`${API_BASE}/corpus-search?top_k=${topK}`, {
    method: 'POST',
    body: formData,
  });
  return res.json();
}
