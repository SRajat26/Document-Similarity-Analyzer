import { useState } from 'react';
import CompareMode from './components/CompareMode';
import CorpusMode from './components/CorpusMode';

function App() {
  const [activeTab, setActiveTab] = useState('compare'); // 'compare' or 'corpus'

  return (
    <>


      <div style={{ maxWidth: '1200px', margin: '0 auto', padding: '40px 20px' }}>
        <header style={{ textAlign: 'center', marginBottom: '50px' }}>
          <h1 style={{ fontSize: '3rem', margin: '0 0 16px 0', letterSpacing: '-0.03em' }}>
            Algorithmic Document Analyzer
          </h1>
          <p style={{ color: 'var(--text-secondary)', fontSize: '1.1rem', maxWidth: '700px', margin: '0 auto', lineHeight: 1.6 }}>
            A Design and Analysis of Algorithms (DAA) project demonstrating classical string matching, dynamic programming, and efficient search strategies.
          </p>
        </header>

        <div style={{ display: 'flex', justifyContent: 'center', marginBottom: '40px' }}>
          <div className="tab-group">
            <button
              className={`tab-btn ${activeTab === 'compare' ? 'active' : ''}`}
              onClick={() => setActiveTab('compare')}
            >
              Mode 1: 1-on-1 Comparison
            </button>
            <button
              className={`tab-btn ${activeTab === 'corpus' ? 'active' : ''}`}
              onClick={() => setActiveTab('corpus')}
            >
              Mode 2: Corpus Search
            </button>
          </div>
        </div>

        <main>
          {activeTab === 'compare' ? <CompareMode /> : <CorpusMode />}
        </main>
      </div>
    </>
  );
}

export default App;
