import { useCallback } from 'react';
import { useDropzone } from 'react-dropzone';

export default function FileUpload({ onFile, file, label, id }) {
  const onDrop = useCallback(
    (acceptedFiles) => {
      if (acceptedFiles.length > 0) {
        onFile(acceptedFiles[0]);
      }
    },
    [onFile]
  );

  const { getRootProps, getInputProps, isDragActive } = useDropzone({
    onDrop,
    accept: { 'text/plain': ['.txt', '.md', '.text'] },
    multiple: false,
  });

  const classes = [
    'dropzone',
    isDragActive && 'active',
    file && 'has-file',
  ]
    .filter(Boolean)
    .join(' ');

  return (
    <div id={id} {...getRootProps()} className={classes}>
      <input {...getInputProps()} />
      <div style={{ display: 'flex', flexDirection: 'column', alignItems: 'center', gap: '12px' }}>

        <div style={{ fontWeight: 600, fontSize: '1rem', color: 'var(--text-primary)' }}>
          {file ? file.name : label || 'Drop your document here'}
        </div>
        <div style={{ fontSize: '0.85rem', color: 'var(--text-secondary)' }}>
          {file
            ? `${(file.size / 1024).toFixed(1)} KB`
            : 'Drag & drop or click to browse (.txt, .md)'}
        </div>
        {file && (
          <span className="badge badge-emerald">Ready</span>
        )}
      </div>
    </div>
  );
}
