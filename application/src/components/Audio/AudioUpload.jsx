import React, { useState, useRef } from 'react';
import { useAudio } from './AudioContext';
import { Upload, Music, FileAudio, X, Check, AlertCircle } from 'lucide-react';

const AudioUpload = () => {
  const { uploadSong } = useAudio();
  const [dragActive, setDragActive] = useState(false);
  const [uploading, setUploading] = useState(false);
  const [uploadedFiles, setUploadedFiles] = useState([]);
  const [errors, setErrors] = useState([]);
  const fileInputRef = useRef(null);

  const handleDrag = (e) => {
    e.preventDefault();
    e.stopPropagation();
    if (e.type === "dragenter" || e.type === "dragover") {
      setDragActive(true);
    } else if (e.type === "dragleave") {
      setDragActive(false);
    }
  };

  const handleDrop = (e) => {
    e.preventDefault();
    e.stopPropagation();
    setDragActive(false);
    
    if (e.dataTransfer.files && e.dataTransfer.files[0]) {
      handleFiles(e.dataTransfer.files);
    }
  };

  const handleFiles = async (files) => {
    setUploading(true);
    setErrors([]);
    const newUploadedFiles = [];

    for (let i = 0; i < files.length; i++) {
      const file = files[i];
      
      // Validate file type
      if (!file.type.startsWith('audio/')) {
        setErrors(prev => [...prev, `${file.name} is not an audio file`]);
        continue;
      }

      // Validate file size (100MB limit)
      if (file.size > 100 * 1024 * 1024) {
        setErrors(prev => [...prev, `${file.name} is too large (max 100MB)`]);
        continue;
      }

      try {
        const result = await uploadSong(file);
        newUploadedFiles.push({
          name: file.name,
          size: file.size,
          status: 'success',
          result
        });
      } catch (error) {
        setErrors(prev => [...prev, `Failed to upload ${file.name}: ${error.message}`]);
        newUploadedFiles.push({
          name: file.name,
          size: file.size,
          status: 'error',
          error: error.message
        });
      }
    }

    setUploadedFiles(prev => [...prev, ...newUploadedFiles]);
    setUploading(false);
  };

  const handleFileInput = (e) => {
    if (e.target.files && e.target.files[0]) {
      handleFiles(e.target.files);
    }
  };

  const removeFile = (index) => {
    setUploadedFiles(prev => prev.filter((_, i) => i !== index));
  };

  const clearAll = () => {
    setUploadedFiles([]);
    setErrors([]);
  };

  const formatFileSize = (bytes) => {
    if (bytes === 0) return '0 B';
    const k = 1024;
    const sizes = ['B', 'KB', 'MB', 'GB'];
    const i = Math.floor(Math.log(bytes) / Math.log(k));
    return parseFloat((bytes / Math.pow(k, i)).toFixed(1)) + ' ' + sizes[i];
  };

  return (
    <div className="space-y-6">
      {/* Header */}
      <div className="flex items-center justify-between">
        <div>
          <h1 className="text-3xl font-bold text-white">Upload Music</h1>
          <p className="text-gray-400 mt-2">
            Add new music files to your library
          </p>
        </div>
        <div className="flex items-center space-x-2 text-primary-400">
          <Upload className="h-8 w-8" />
        </div>
      </div>

      {/* Upload Area */}
      <div className="card">
        <div
          className={`border-2 border-dashed rounded-lg p-8 text-center transition-colors ${
            dragActive 
              ? 'border-primary-500 bg-primary-500/10' 
              : 'border-gray-600 hover:border-gray-500'
          }`}
          onDragEnter={handleDrag}
          onDragLeave={handleDrag}
          onDragOver={handleDrag}
          onDrop={handleDrop}
        >
          <Upload className="h-16 w-16 text-gray-400 mx-auto mb-4" />
          <h3 className="text-lg font-medium text-white mb-2">
            Drop audio files here
          </h3>
          <p className="text-gray-400 mb-4">
            or click to browse your files
          </p>
          
          <button
            onClick={() => fileInputRef.current?.click()}
            className="btn-primary"
            disabled={uploading}
          >
            {uploading ? 'Uploading...' : 'Choose Files'}
          </button>
          
          <input
            ref={fileInputRef}
            type="file"
            multiple
            accept="audio/*"
            onChange={handleFileInput}
            className="hidden"
          />
          
          <p className="text-sm text-gray-500 mt-4">
            Supports: MP3, WAV, FLAC, AAC, OGG (Max 100MB per file)
          </p>
        </div>
      </div>

      {/* Upload Progress */}
      {uploading && (
        <div className="card">
          <h3 className="text-lg font-medium text-white mb-4">Uploading...</h3>
          <div className="w-full bg-gray-700 rounded-full h-2">
            <div className="bg-primary-500 h-2 rounded-full animate-pulse"></div>
          </div>
        </div>
      )}

      {/* Uploaded Files */}
      {uploadedFiles.length > 0 && (
        <div className="card">
          <div className="flex items-center justify-between mb-4">
            <h3 className="text-lg font-medium text-white">Uploaded Files</h3>
            <button
              onClick={clearAll}
              className="text-gray-400 hover:text-white text-sm"
            >
              Clear All
            </button>
          </div>
          
          <div className="space-y-3">
            {uploadedFiles.map((file, index) => (
              <div 
                key={index}
                className="flex items-center space-x-3 p-3 bg-gray-700 rounded-lg"
              >
                <div className="flex-shrink-0">
                  {file.status === 'success' ? (
                    <Check className="h-5 w-5 text-green-400" />
                  ) : file.status === 'error' ? (
                    <AlertCircle className="h-5 w-5 text-red-400" />
                  ) : (
                    <Music className="h-5 w-5 text-gray-400" />
                  )}
                </div>
                
                <div className="flex-1 min-w-0">
                  <div className="text-white font-medium truncate">{file.name}</div>
                  <div className="text-gray-400 text-sm">
                    {formatFileSize(file.size)}
                  </div>
                </div>
                
                <div className="flex-shrink-0">
                  {file.status === 'success' ? (
                    <span className="text-green-400 text-sm">Success</span>
                  ) : file.status === 'error' ? (
                    <span className="text-red-400 text-sm">Failed</span>
                  ) : (
                    <span className="text-gray-400 text-sm">Processing</span>
                  )}
                </div>
                
                <button
                  onClick={() => removeFile(index)}
                  className="p-1 text-gray-400 hover:text-white"
                >
                  <X className="h-4 w-4" />
                </button>
              </div>
            ))}
          </div>
        </div>
      )}

      {/* Errors */}
      {errors.length > 0 && (
        <div className="card border border-red-600 bg-red-900/20">
          <h3 className="text-lg font-medium text-red-400 mb-4">Upload Errors</h3>
          <div className="space-y-2">
            {errors.map((error, index) => (
              <div key={index} className="flex items-center space-x-2 text-red-400">
                <AlertCircle className="h-4 w-4" />
                <span className="text-sm">{error}</span>
              </div>
            ))}
          </div>
        </div>
      )}

      {/* Tips */}
      <div className="card bg-blue-900/20 border border-blue-700">
        <h3 className="text-lg font-medium text-blue-300 mb-3">Upload Tips</h3>
        <ul className="space-y-2 text-blue-200 text-sm">
          <li>• Supported formats: MP3, WAV, FLAC, AAC, OGG</li>
          <li>• Maximum file size: 100MB per file</li>
          <li>• Files are automatically scanned for metadata</li>
          <li>• Duplicate files will be detected and handled</li>
          <li>• Upload progress is shown in real-time</li>
        </ul>
      </div>
    </div>
  );
};

export default AudioUpload;
