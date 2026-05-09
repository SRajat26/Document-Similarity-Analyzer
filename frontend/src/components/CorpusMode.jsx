import { useState, useEffect } from 'react';
import FileUpload from './FileUpload';
import SimilarityGauge from './SimilarityGauge';
import { corpusSearch, listCorpus } from '../api';

export default function CorpusMode() {
  const [file, setFile] = useState(null);
  const [loading, setLoading] = useState(false);
  const [results, setResults] = useState(null);
  const [error, setError] = useState(null);
  const [corpusStats, setCorpusStats] = useState(null);

  useEffect(() => {
    async function fetchCorpus() {
      try {
        const data = await listCorpus();
        setCorpusStats(data);
      } catch (err) {
        console.error("Failed to load corpus stats", err);
      }
    }
    fetchCorpus();
  }, []);

  const handleSearch = async () => {
    if (!file) return;

    setLoading(true);
    setError(null);
    try {
      const data = await corpusSearch(file, 10);
      if (data.error) throw new Error(data.error);
      setResults(data);
    } catch (err) {
      setError(err.message || 'An error occurred during corpus search.');
    } finally {
      setLoading(false);
    }
  };

  return (
    <div>
      <div style={{ display: 'grid', gridTemplateColumns: '1fr 300px', gap: '20px', marginBottom: '30px' }}>
        <div>
          <FileUpload id="upload-query" label="Query Document" file={file} onFile={setFile} />

          <div style={{ marginTop: '20px' }}>
            <button
              className="btn-primary"
              onClick={handleSearch}
              disabled={!file || loading}
              style={{ padding: '16px 40px', fontSize: '1.1rem', width: '100%' }}
            >
              {loading ? (
                <span style={{ display: 'flex', alignItems: 'center', justifyContent: 'center', gap: '10px' }}>
                  <div className="spinner" style={{ width: '20px', height: '20px', borderWidth: '2px' }}></div>
                  Searching Corpus...
                </span>
              ) : (
                <span>Search Against Corpus</span>
              )}
            </button>
            {error && (
              <div style={{ color: 'var(--accent-rose)', marginTop: '16px', background: 'rgba(244, 63, 94, 0.1)', padding: '12px', borderRadius: '8px' }}>
                {error}
              </div>
            )}
          </div>
        </div>

        <div className="metric-card" style={{ display: 'flex', flexDirection: 'column', gap: '16px' }}>
          <h3 style={{ margin: 0, color: 'var(--text-primary)' }}>Corpus Database</h3>

          <div>
            <div className="metric-label">Total Documents</div>
            <div className="metric-value" style={{ fontSize: '1.5rem', color: 'var(--accent-cyan)' }}>
              {corpusStats ? corpusStats.corpus_size : '...'}
            </div>
          </div>

          <div>
            <div className="metric-label">Indexing Algorithm</div>
            <div style={{ fontSize: '0.9rem', color: 'var(--text-primary)', marginTop: '4px' }}>
              <span className="badge badge-violet">Rolling Hash Fingerprints</span>
            </div>
          </div>

          <div>
            <div className="metric-label">Search Strategy</div>
            <div style={{ fontSize: '0.9rem', color: 'var(--text-primary)', marginTop: '4px' }}>
              <span className="badge badge-emerald">Divide & Conquer Pruning</span>
            </div>
          </div>
        </div>
      </div>

      {results && (
        <div style={{ display: 'flex', flexDirection: 'column', gap: '30px' }}>

          {/* Search Stats */}
          <div className="glass-card" style={{ padding: '20px' }}>
            <h3 style={{ margin: '0 0 16px 0', color: 'var(--text-primary)' }}>Algorithm Performance (Divide & Conquer)</h3>
            <div style={{ display: 'grid', gridTemplateColumns: 'repeat(4, 1fr)', gap: '16px', textAlign: 'center' }}>
              <div>
                <div className="metric-value" style={{ fontSize: '1.5rem' }}>{results.runtime_ms.toFixed(2)}<span style={{ fontSize: '0.8rem' }}>ms</span></div>
                <div className="metric-label">Runtime</div>
              </div>
              <div>
                <div className="metric-value" style={{ fontSize: '1.5rem', color: 'var(--accent-rose)' }}>{results.documents_pruned}</div>
                <div className="metric-label">Docs Pruned (O(1))</div>
              </div>
              <div>
                <div className="metric-value" style={{ fontSize: '1.5rem', color: 'var(--accent-cyan)' }}>{results.documents_searched}</div>
                <div className="metric-label">Full Comparisons</div>
              </div>
              <div>
                <div className="metric-value" style={{ fontSize: '1.5rem', color: 'var(--accent-amber)' }}>{results.hash_lookups}</div>
                <div className="metric-label">Hash Lookups</div>
              </div>
            </div>
          </div>

          {/* Top Matches */}
          <h3 style={{ margin: 0, color: 'var(--text-primary)' }}>Top Ranked Matches</h3>

          {results.detailed_results?.map((match, idx) => (
            <div key={idx} className="glass-card" style={{ padding: '24px', display: 'flex', gap: '24px' }}>

              <div style={{ flexShrink: 0, display: 'flex', flexDirection: 'column', alignItems: 'center' }}>
                <div style={{ fontSize: '1.2rem', fontWeight: 800, color: 'var(--text-secondary)', marginBottom: '12px' }}>
                  #{idx + 1}
                </div>
                <SimilarityGauge percentage={match.fingerprint_similarity} />
              </div>

              <div style={{ flexGrow: 1, display: 'flex', flexDirection: 'column', gap: '12px' }}>
                <div style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'flex-start' }}>
                  <h4 style={{ margin: 0, fontSize: '1.2rem', color: 'var(--accent-cyan)' }}>{match.filename}</h4>
                  <div style={{ display: 'flex', gap: '8px' }}>
                    <span className="badge badge-cyan">LCS Sim: {match.lcs_similarity?.toFixed(1)}%</span>
                    <span className="badge badge-amber">LCP Len: {match.longest_substring_length}</span>
                  </div>
                </div>

                <div className="mono" style={{ fontSize: '0.85rem', color: 'var(--text-secondary)', background: 'var(--bg-card-hover)', padding: '12px', borderRadius: '4px' }}>
                  {match.corpus_text_preview}...
                </div>

                {match.longest_common_substring && (
                  <div>
                    <div className="metric-label" style={{ marginBottom: '4px' }}>Longest Exact Shared Substring (Suffix Array)</div>
                    <div style={{ fontSize: '0.9rem', borderLeft: '3px solid var(--accent-amber)', paddingLeft: '12px', color: 'var(--text-primary)' }}>
                      "{match.longest_common_substring}"
                    </div>
                  </div>
                )}

                {match.shared_phrases && match.shared_phrases.length > 0 && (
                  <div>
                    <div className="metric-label" style={{ marginBottom: '4px' }}>Shared Key Phrases (Trie Matches)</div>
                    <div style={{ display: 'flex', flexWrap: 'wrap', gap: '6px' }}>
                      {match.shared_phrases.map((phrase, pidx) => (
                        <span key={pidx} style={{ fontSize: '0.75rem', background: 'rgba(16, 185, 129, 0.1)', color: 'var(--accent-emerald)', padding: '2px 6px', borderRadius: '4px' }}>
                          {phrase}
                        </span>
                      ))}
                    </div>
                  </div>
                )}
              </div>
            </div>
          ))}

          {(!results.detailed_results || results.detailed_results.length === 0) && (
            <div className="glass-card" style={{ padding: '40px', textAlign: 'center', color: 'var(--text-secondary)' }}>
              No significant matches found in the corpus.
            </div>
          )}

        </div>
      )}
    </div>
  );
}
