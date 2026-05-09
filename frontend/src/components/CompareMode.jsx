import { useState } from 'react';
import FileUpload from './FileUpload';
import SimilarityGauge from './SimilarityGauge';
import DocumentViewer from './DocumentViewer';
import { ComplexityTable } from './Visualizations';
import { compareDocuments } from '../api';

export default function CompareMode() {
  const [fileA, setFileA] = useState(null);
  const [fileB, setFileB] = useState(null);
  const [loading, setLoading] = useState(false);
  const [results, setResults] = useState(null);
  const [error, setError] = useState(null);

  const handleCompare = async () => {
    if (!fileA || !fileB) return;
    
    setLoading(true);
    setError(null);
    try {
      const data = await compareDocuments(fileA, fileB);
      if (data.error) throw new Error(data.error);
      setResults(data);
    } catch (err) {
      setError(err.message || 'An error occurred during comparison.');
    } finally {
      setLoading(false);
    }
  };

  return (
    <div>
      <div style={{ display: 'grid', gridTemplateColumns: '1fr 1fr', gap: '20px', marginBottom: '30px' }}>
        <FileUpload id="upload-a" label="Document A (Source)" file={fileA} onFile={setFileA} />
        <FileUpload id="upload-b" label="Document B (Target)" file={fileB} onFile={setFileB} />
      </div>

      <div style={{ textAlign: 'center', marginBottom: '40px' }}>
        <button 
          className="btn-primary" 
          onClick={handleCompare}
          disabled={!fileA || !fileB || loading}
          style={{ padding: '16px 40px', fontSize: '1.1rem' }}
        >
          {loading ? (
            <span style={{ display: 'flex', alignItems: 'center', gap: '10px' }}>
              <div className="spinner" style={{ width: '20px', height: '20px', borderWidth: '2px' }}></div>
              Analyzing Documents...
            </span>
          ) : (
            <span>Run Algorithmic Comparison</span>
          )}
        </button>
        {error && (
          <div style={{ color: 'var(--accent-rose)', marginTop: '16px', background: 'rgba(244, 63, 94, 0.1)', padding: '12px', borderRadius: '8px' }}>
            {error}
          </div>
        )}
      </div>

      {results && (
        <div style={{ display: 'flex', flexDirection: 'column', gap: '40px' }}>
          
          {/* Main Stats Row */}
          <div style={{ display: 'grid', gridTemplateColumns: 'repeat(auto-fit, minmax(250px, 1fr))', gap: '20px' }}>
            <div className="glass-card" style={{ display: 'flex', justifyContent: 'center', padding: '30px' }}>
              <SimilarityGauge percentage={results.overall_similarity} label="Overall Algorithmic Similarity" />
            </div>
            
            <div className="metric-card" style={{ display: 'flex', flexDirection: 'column', justifyContent: 'center' }}>
              <div className="metric-label">Document A Length</div>
              <div className="metric-value">{results.text_a_length} <span style={{ fontSize: '1rem', color: 'var(--text-secondary)' }}>chars</span></div>
              
              <div className="metric-label" style={{ marginTop: '16px' }}>Tokens (after stop words)</div>
              <div className="metric-value" style={{ color: 'var(--accent-violet)' }}>
                {results.preprocessing?.doc_a?.token_count_filtered || 0}
              </div>
            </div>

            <div className="metric-card" style={{ display: 'flex', flexDirection: 'column', justifyContent: 'center' }}>
              <div className="metric-label">Document B Length</div>
              <div className="metric-value">{results.text_b_length} <span style={{ fontSize: '1rem', color: 'var(--text-secondary)' }}>chars</span></div>
              
              <div className="metric-label" style={{ marginTop: '16px' }}>Tokens (after stop words)</div>
              <div className="metric-value" style={{ color: 'var(--accent-cyan)' }}>
                {results.preprocessing?.doc_b?.token_count_filtered || 0}
              </div>
            </div>
          </div>



          {/* Complexity Table */}
          <ComplexityTable data={results.algorithm_comparison} />

          {/* Document Viewer */}
          <div className="glass-card" style={{ padding: '20px' }}>
            <h3 style={{ marginTop: 0, marginBottom: '20px', color: 'var(--text-primary)' }}>Algorithm Match Highlights</h3>
            <DocumentViewer 
              textA={results.text_a} 
              textB={results.text_b}
              kmpMatches={results.kmp}
              suffixMatches={results.suffix_array}
              lcsPositions={results.lcs?.matched_positions}
            />
          </div>



        </div>
      )}
    </div>
  );
}
