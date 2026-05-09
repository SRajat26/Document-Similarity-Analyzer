export default function SimilarityGauge({ percentage, label, color = 'cyan' }) {
  const radius = 70;
  const stroke = 10;
  const normalizedRadius = radius - stroke / 2;
  const circumference = normalizedRadius * 2 * Math.PI;
  const strokeDashoffset = circumference - (percentage / 100) * circumference;

  const colors = {
    cyan: { main: '#3b82f6', glow: 'rgba(59, 130, 246, 0.3)' },
    violet: { main: '#2563eb', glow: 'rgba(37, 99, 235, 0.3)' },
    emerald: { main: '#60a5fa', glow: 'rgba(96, 165, 250, 0.3)' },
    rose: { main: '#1e40af', glow: 'rgba(30, 64, 175, 0.3)' },
    amber: { main: '#93c5fd', glow: 'rgba(147, 197, 253, 0.3)' },
  };

  const c = colors[color] || colors.cyan;

  // Color changes based on percentage
  const getColor = () => {
    if (percentage >= 75) return colors.rose.main;
    if (percentage >= 50) return colors.violet.main;
    if (percentage >= 25) return colors.cyan.main;
    return colors.emerald.main;
  };

  const actualColor = getColor();

  return (
    <div style={{ display: 'flex', flexDirection: 'column', alignItems: 'center', gap: '8px' }}>
      <div style={{ position: 'relative', width: radius * 2, height: radius * 2 }}>
        <svg width={radius * 2} height={radius * 2} style={{ transform: 'rotate(-90deg)' }}>
          {/* Background circle */}
          <circle
            cx={radius}
            cy={radius}
            r={normalizedRadius}
            fill="none"
            stroke="rgba(148, 163, 184, 0.1)"
            strokeWidth={stroke}
          />
          {/* Progress circle */}
          <circle
            cx={radius}
            cy={radius}
            r={normalizedRadius}
            fill="none"
            stroke={actualColor}
            strokeWidth={stroke}
            strokeDasharray={circumference}
            strokeDashoffset={strokeDashoffset}
            strokeLinecap="round"
            style={{}}
          />
        </svg>
        {/* Center text */}
        <div
          style={{
            position: 'absolute',
            inset: 0,
            display: 'flex',
            flexDirection: 'column',
            alignItems: 'center',
            justifyContent: 'center',
          }}
        >
          <span style={{ fontSize: '1.8rem', fontWeight: 800, color: actualColor }}>
            {percentage.toFixed(1)}
          </span>
          <span style={{ fontSize: '0.7rem', color: 'var(--text-secondary)', fontWeight: 600 }}>
            %
          </span>
        </div>
      </div>
      {label && (
        <span style={{ fontSize: '0.8rem', color: 'var(--text-secondary)', fontWeight: 500 }}>
          {label}
        </span>
      )}
    </div>
  );
}
