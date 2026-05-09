import { BarChart, Bar, XAxis, YAxis, CartesianGrid, Tooltip, Legend, ResponsiveContainer, RadarChart, PolarGrid, PolarAngleAxis, PolarRadiusAxis, Radar } from 'recharts';

export function RuntimeChart({ data }) {
  if (!data || data.length === 0) return null;

  return (
    <div className="glass-card" style={{ padding: '20px', height: '350px' }}>
      <h3 style={{ marginTop: 0, marginBottom: '20px', color: 'var(--text-primary)' }}>Algorithm Runtime Comparison (ms)</h3>
      <ResponsiveContainer width="100%" height="85%">
        <BarChart data={data} margin={{ top: 5, right: 30, left: 20, bottom: 5 }}>
          <CartesianGrid strokeDasharray="3 3" stroke="var(--border)" />
          <XAxis dataKey="algorithm" stroke="var(--text-secondary)" />
          <YAxis stroke="var(--text-secondary)" />
          <Tooltip 
            contentStyle={{ backgroundColor: 'var(--bg-card)', borderColor: 'var(--border)', color: 'var(--text-primary)' }}
            itemStyle={{ color: 'var(--accent-cyan)' }}
          />
          <Legend />
          <Bar dataKey="runtime_ms" fill="var(--accent-cyan)" name="Runtime (ms)" radius={[4, 4, 0, 0]} />
        </BarChart>
      </ResponsiveContainer>
    </div>
  );
}

export function SimilarityRadar({ data }) {
  if (!data || data.length === 0) return null;

  return (
    <div className="glass-card" style={{ padding: '20px', height: '350px' }}>
      <h3 style={{ marginTop: 0, marginBottom: '20px', color: 'var(--text-primary)' }}>Algorithm Similarity Scores</h3>
      <ResponsiveContainer width="100%" height="85%">
        <RadarChart cx="50%" cy="50%" outerRadius="80%" data={data}>
          <PolarGrid stroke="var(--border)" />
          <PolarAngleAxis dataKey="algorithm" tick={{ fill: 'var(--text-secondary)', fontSize: 12 }} />
          <PolarRadiusAxis angle={30} domain={[0, 100]} stroke="var(--text-muted)" />
          <Radar name="Similarity %" dataKey="similarity" stroke="var(--accent-violet)" fill="var(--accent-violet)" fillOpacity={0.5} />
          <Tooltip 
            contentStyle={{ backgroundColor: 'var(--bg-card)', borderColor: 'var(--border)', color: 'var(--text-primary)' }}
          />
        </RadarChart>
      </ResponsiveContainer>
    </div>
  );
}

export function ComplexityTable({ data }) {
  if (!data || data.length === 0) return null;

  return (
    <div className="glass-card" style={{ padding: '20px', overflowX: 'auto' }}>
      <h3 style={{ marginTop: 0, marginBottom: '20px', color: 'var(--text-primary)' }}>Complexity Analysis</h3>
      <table className="data-table">
        <thead>
          <tr>
            <th>Algorithm</th>
            <th>Time Complexity</th>
            <th>Space Complexity</th>
          </tr>
        </thead>
        <tbody>
          {data.map((item, index) => (
            <tr key={index}>
              <td style={{ fontWeight: 600, color: 'var(--accent-cyan)' }}>{item.algorithm}</td>
              <td className="mono">{item.time_complexity}</td>
              <td className="mono">{item.space_complexity}</td>
            </tr>
          ))}
        </tbody>
      </table>
    </div>
  );
}

export function MatrixVisualization({ data, title = "LCS DP Table Sample" }) {
  if (!data || data.length === 0) return null;

  return (
    <div className="glass-card" style={{ padding: '20px', overflowX: 'auto' }}>
      <h3 style={{ marginTop: 0, marginBottom: '16px', color: 'var(--text-primary)' }}>{title}</h3>
      <div style={{ display: 'grid', gap: '2px', backgroundColor: 'var(--border)', padding: '2px', borderRadius: 'var(--radius-sm)', width: 'fit-content' }}>
        {data.map((row, i) => (
          <div key={i} style={{ display: 'flex', gap: '2px' }}>
            {row.map((cell, j) => {
              // Calculate color intensity based on value
              const maxVal = Math.max(...data.flat());
              const intensity = maxVal > 0 ? cell / maxVal : 0;
              const bgColor = `rgba(139, 92, 246, ${0.1 + intensity * 0.8})`; // Violet tint
              
              return (
                <div 
                  key={`${i}-${j}`} 
                  style={{ 
                    width: '32px', 
                    height: '32px', 
                    display: 'flex', 
                    alignItems: 'center', 
                    justifyContent: 'center',
                    backgroundColor: bgColor,
                    color: intensity > 0.5 ? 'white' : 'var(--text-primary)',
                    fontSize: '0.8rem',
                    fontWeight: 600,
                    borderRadius: '2px'
                  }}
                  title={`[${i},${j}] = ${cell}`}
                >
                  {cell}
                </div>
              );
            })}
          </div>
        ))}
      </div>
    </div>
  );
}
