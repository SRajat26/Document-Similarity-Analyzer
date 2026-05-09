import { useMemo, useRef, useEffect } from 'react';

/**
 * Side-by-side document viewer with synchronized scrolling and highlights.
 * Colors: red=exact, yellow=partial, blue=LCS
 */
export default function DocumentViewer({ textA, textB, kmpMatches, suffixMatches, lcsPositions, tokensA, tokensB }) {
  const panelARef = useRef(null);
  const panelBRef = useRef(null);
  const syncingRef = useRef(false);

  // Synchronized scrolling
  useEffect(() => {
    const panelA = panelARef.current;
    const panelB = panelBRef.current;
    if (!panelA || !panelB) return;

    const handleScrollA = () => {
      if (syncingRef.current) return;
      syncingRef.current = true;
      const ratio = panelA.scrollTop / (panelA.scrollHeight - panelA.clientHeight || 1);
      panelB.scrollTop = ratio * (panelB.scrollHeight - panelB.clientHeight);
      syncingRef.current = false;
    };

    const handleScrollB = () => {
      if (syncingRef.current) return;
      syncingRef.current = true;
      const ratio = panelB.scrollTop / (panelB.scrollHeight - panelB.clientHeight || 1);
      panelA.scrollTop = ratio * (panelA.scrollHeight - panelA.clientHeight);
      syncingRef.current = false;
    };

    panelA.addEventListener('scroll', handleScrollA);
    panelB.addEventListener('scroll', handleScrollB);

    return () => {
      panelA.removeEventListener('scroll', handleScrollA);
      panelB.removeEventListener('scroll', handleScrollB);
    };
  }, []);

  // Build highlighted text for document A
  const highlightedA = useMemo(() => {
    if (!textA) return '';
    return buildHighlightedText(textA, kmpMatches, suffixMatches, lcsPositions, 'a');
  }, [textA, kmpMatches, suffixMatches, lcsPositions]);

  // Build highlighted text for document B
  const highlightedB = useMemo(() => {
    if (!textB) return '';
    return buildHighlightedText(textB, kmpMatches, suffixMatches, lcsPositions, 'b');
  }, [textB, kmpMatches, suffixMatches, lcsPositions]);

  return (
    <div className="fade-in-up">
      <div style={{ display: 'flex', gap: '8px', marginBottom: '12px', flexWrap: 'wrap' }}>
        <span className="badge badge-rose">■ Exact Match (KMP)</span>
        <span className="badge badge-amber">■ Shared Substring (Suffix Array)</span>
        <span className="badge badge-cyan">■ Paraphrase / LCS</span>
      </div>

      <div className="side-by-side">
        <div>
          <div className="doc-panel-header">
            <span>Document A</span>
            <span style={{ fontSize: '0.75rem', color: 'var(--text-muted)' }}>
              {textA?.length || 0} chars
            </span>
          </div>
          <div
            ref={panelARef}
            className="doc-panel"
            dangerouslySetInnerHTML={{ __html: highlightedA }}
          />
        </div>
        <div>
          <div className="doc-panel-header">
            <span>Document B</span>
            <span style={{ fontSize: '0.75rem', color: 'var(--text-muted)' }}>
              {textB?.length || 0} chars
            </span>
          </div>
          <div
            ref={panelBRef}
            className="doc-panel"
            dangerouslySetInnerHTML={{ __html: highlightedB }}
          />
        </div>
      </div>
    </div>
  );
}

function buildHighlightedText(text, kmpMatches, suffixMatches, lcsPositions, side) {
  if (!text) return '';

  // Build highlight map: position -> type
  const highlights = new Map();

  // KMP matches (exact - red)
  if (kmpMatches?.matches) {
    for (const match of kmpMatches.matches) {
      const positions = side === 'a' ? match.positions_in_a : match.positions_in_b;
      if (positions) {
        for (const pos of positions) {
          for (let i = pos; i < pos + match.length && i < text.length; i++) {
            highlights.set(i, 'exact');
          }
        }
      }
    }
  }

  // Suffix array matches (partial - yellow)
  if (suffixMatches?.common_substrings) {
    for (const sub of suffixMatches.common_substrings) {
      const pos = side === 'a' ? sub.position_in_a : sub.position_in_b;
      if (pos >= 0) {
        for (let i = pos; i < pos + sub.length && i < text.length; i++) {
          if (!highlights.has(i)) {
            highlights.set(i, 'partial');
          }
        }
      }
    }
  }

  // Build HTML
  let html = '';
  let currentType = null;

  for (let i = 0; i < text.length; i++) {
    const type = highlights.get(i) || null;

    if (type !== currentType) {
      if (currentType) html += '</span>';
      if (type) {
        const cls = type === 'exact' ? 'highlight-exact' : type === 'partial' ? 'highlight-partial' : 'highlight-lcs';
        html += `<span class="${cls}">`;
      }
      currentType = type;
    }

    const ch = text[i];
    if (ch === '<') html += '&lt;';
    else if (ch === '>') html += '&gt;';
    else if (ch === '&') html += '&amp;';
    else if (ch === '\n') html += '<br/>';
    else html += ch;
  }

  if (currentType) html += '</span>';

  return html;
}
