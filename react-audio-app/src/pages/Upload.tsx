import React, { useState, useCallback } from 'react';
import { useAudio } from '../contexts/AudioContext';
import { Upload, Music, FileAudio, X, CheckCircle, AlertCircle, FolderOpen } from 'lucide-react';

const Upload: React.FC = () => {
  const { state, uploadSong, uploadMultipleSongs } = useAudio();
  const [dragActive, setDragActive] = useState(false);
  const [selectedFiles, setSelectedFiles] = useState<File[]>([]);
  const [category, setCategory] = useState<string>('');
  const [uploadProgress, setUploadProgress] = useState<{ [key: string]: number }>({});

  const handleDrag = useCallback((e: React.DragEvent) => {
    e.preventDefault();
    e.stopPropagation();
    if (e.type === 'dragenter' || e.type === 'dragover') {
      setDragActive(true);
    } else if (e.type === 'dragleave') {
      setDragActive(false);
    }
  }, []);

  const handleDrop = useCallback((e: React.DragEvent) => {
    e.preventDefault();
    e.stopPropagation();
    setDragActive(false);

    if (e.dataTransfer.files && e.dataTransfer.files[0]) {
      const files = Array.from(e.dataTransfer.files);
      const audioFiles = files.filter(file => 
        file.type.startsWith('audio/') || 
        file.name.match(/\.(mp3|wav|flac|ogg|m4a|aac)$/i)
      );
      setSelectedFiles(prev => [...prev, ...audioFiles]);
    }
  }, []);

  const handleFileSelect = (e: React.ChangeEvent<HTMLInputElement>) => {
    if (e.target.files) {
      const files = Array.from(e.target.files);
      const audioFiles = files.filter(file => 
        file.type.startsWith('audio/') || 
        file.name.match(/\.(mp3|wav|flac|ogg|m4a|aac)$/i)
      );
      setSelectedFiles(prev => [...prev, ...audioFiles]);
    }
  };

  const removeFile = (index: number) => {
    setSelectedFiles(prev => prev.filter((_, i) => i !== index));
  };

  const clearAllFiles = () => {
    setSelectedFiles([]);
    setUploadProgress({});
  };

  const handleUpload = async () => {
    if (selectedFiles.length === 0) return;

    if (selectedFiles.length === 1) {
      // Single file upload
      const file = selectedFiles[0];
      setUploadProgress({ [file.name]: 0 });
      
      try {
        await uploadSong(file, category || undefined);
        setUploadProgress({ [file.name]: 100 });
        setTimeout(() => {
          setSelectedFiles([]);
          setUploadProgress({});
        }, 1000);
      } catch (error) {
        setUploadProgress({ [file.name]: -1 });
      }
    } else {
      // Multiple files upload
      const progress: { [key: string]: number } = {};
      selectedFiles.forEach(file => {
        progress[file.name] = 0;
      });
      setUploadProgress(progress);

      try {
        await uploadMultipleSongs(selectedFiles, category || undefined);
        
        // Simulate progress for each file
        selectedFiles.forEach(file => {
          progress[file.name] = 100;
        });
        setUploadProgress(progress);
        
        setTimeout(() => {
          setSelectedFiles([]);
          setUploadProgress({});
        }, 1000);
      } catch (error) {
        selectedFiles.forEach(file => {
          progress[file.name] = -1;
        });
        setUploadProgress(progress);
      }
    }
  };

  const formatFileSize = (bytes: number): string => {
    if (bytes === 0) return '0 B';
    
    const k = 1024;
    const sizes = ['B', 'KB', 'MB', 'GB'];
    const i = Math.floor(Math.log(bytes) / Math.log(k));
    
    return parseFloat((bytes / Math.pow(k, i)).toFixed(2)) + ' ' + sizes[i];
  };

  const getFileIcon = (fileName: string) => {
    const ext = fileName.split('.').pop()?.toLowerCase();
    switch (ext) {
      case 'mp3':
        return <Music className="h-8 w-8 text-green-400" />;
      case 'wav':
        return <Music className="h-8 w-8 text-blue-400" />;
      case 'flac':
        return <Music className="h-8 w-8 text-purple-400" />;
      case 'ogg':
        return <Music className="h-8 w-8 text-orange-400" />;
      case 'm4a':
      case 'aac':
        return <Music className="h-8 w-8 text-red-400" />;
      default:
        return <FileAudio className="h-8 w-8 text-gray-400" />;
    }
  };

  const getProgressColor = (progress: number) => {
    if (progress === -1) return 'bg-red-500';
    if (progress === 100) return 'bg-green-500';
    return 'bg-primary-500';
  };

  const totalSize = selectedFiles.reduce((total, file) => total + file.size, 0);

  return (
    <div className="space-y-6">
      {/* Header */}
      <div>
        <h1 className="text-3xl font-bold text-white">Upload Music</h1>
        <p className="text-gray-400 mt-2">
          Add new music files to your library
        </p>
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
          <Upload className="h-16 w-16 mx-auto mb-4 text-gray-400" />
          <h3 className="text-lg font-medium text-white mb-2">
            Drop audio files here
          </h3>
          <p className="text-gray-400 mb-4">
            or click to select files from your computer
          </p>
          
          <input
            type="file"
            multiple
            accept="audio/*,.mp3,.wav,.flac,.ogg,.m4a,.aac"
            onChange={handleFileSelect}
            className="hidden"
            id="file-upload"
          />
          
          <label
            htmlFor="file-upload"
            className="btn-primary cursor-pointer inline-flex items-center space-x-2"
          >
            <FolderOpen className="h-4 w-4" />
            <span>Choose Files</span>
          </label>
          
          <p className="text-sm text-gray-500 mt-2">
            Supported formats: MP3, WAV, FLAC, OGG, M4A, AAC
          </p>
        </div>
      </div>

      {/* Category Selection */}
      <div className="card">
        <h3 className="text-lg font-medium text-white mb-4">Upload Category</h3>
        <select
          value={category}
          onChange={(e) => setCategory(e.target.value)}
          className="input-field w-full md:w-64"
        >
          <option value="">No Category</option>
          <option value="electronic">Electronic</option>
          <option value="ambient">Ambient</option>
          <option value="rock">Rock</option>
          <option value="jazz">Jazz</option>
          <option value="classical">Classical</option>
          <option value="folk">Folk</option>
          <option value="hip-hop">Hip-Hop</option>
          <option value="pop">Pop</option>
          <option value="other">Other</option>
        </select>
        <p className="text-sm text-gray-400 mt-2">
          Categorize your uploads for better organization
        </p>
      </div>

      {/* Selected Files */}
      {selectedFiles.length > 0 && (
        <div className="card">
          <div className="flex items-center justify-between mb-4">
            <h3 className="text-lg font-medium text-white">
              Selected Files ({selectedFiles.length})
            </h3>
            <div className="flex items-center space-x-2">
              <span className="text-sm text-gray-400">
                Total: {formatFileSize(totalSize)}
              </span>
              <button
                onClick={clearAllFiles}
                className="text-red-400 hover:text-red-300 text-sm"
              >
                Clear All
              </button>
            </div>
          </div>

          <div className="space-y-3">
            {selectedFiles.map((file, index) => (
              <div
                key={`${file.name}-${index}`}
                className="flex items-center space-x-3 p-3 bg-gray-700 rounded-lg"
              >
                {/* File Icon */}
                <div className="flex-shrink-0">
                  {getFileIcon(file.name)}
                </div>

                {/* File Info */}
                <div className="flex-1 min-w-0">
                  <div className="text-sm font-medium text-white truncate">
                    {file.name}
                  </div>
                  <div className="text-xs text-gray-400">
                    {formatFileSize(file.size)} • {file.type || 'Unknown type'}
                  </div>
                </div>

                {/* Progress Bar */}
                <div className="flex-1 max-w-xs">
                  <div className="w-full bg-gray-600 rounded-full h-2">
                    <div
                      className={`h-2 rounded-full transition-all duration-300 ${getProgressColor(uploadProgress[file.name] || 0)}`}
                      style={{ 
                        width: uploadProgress[file.name] === -1 
                          ? '100%' 
                          : `${uploadProgress[file.name] || 0}%` 
                      }}
                    />
                  </div>
                  <div className="text-xs text-gray-400 mt-1">
                    {uploadProgress[file.name] === -1 && 'Failed'}
                    {uploadProgress[file.name] === 100 && 'Complete'}
                    {uploadProgress[file.name] > 0 && uploadProgress[file.name] < 100 && `${uploadProgress[file.name]}%`}
                    {!uploadProgress[file.name] && 'Pending'}
                  </div>
                </div>

                {/* Status Icon */}
                <div className="flex-shrink-0">
                  {uploadProgress[file.name] === 100 && (
                    <CheckCircle className="h-5 w-5 text-green-400" />
                  )}
                  {uploadProgress[file.name] === -1 && (
                    <AlertCircle className="h-5 w-5 text-red-400" />
                  )}
                </div>

                {/* Remove Button */}
                <button
                  onClick={() => removeFile(index)}
                  className="flex-shrink-0 p-1 text-gray-400 hover:text-red-400 transition-colors"
                >
                  <X className="h-4 w-4" />
                </button>
              </div>
            ))}
          </div>

          {/* Upload Button */}
          <div className="mt-6">
            <button
              onClick={handleUpload}
              disabled={state.isUploading}
              className="btn-primary w-full md:w-auto disabled:opacity-50 disabled:cursor-not-allowed"
            >
              {state.isUploading ? (
                <>
                  <div className="animate-spin rounded-full h-4 w-4 border-b-2 border-white mr-2" />
                  Uploading...
                </>
              ) : (
                <>
                  <Upload className="h-4 w-4 mr-2" />
                  Upload {selectedFiles.length} File{selectedFiles.length !== 1 ? 's' : ''}
                </>
              )}
            </button>
          </div>
        </div>
      )}

      {/* Upload Status */}
      {state.isUploading && (
        <div className="card bg-primary-900/20 border-primary-500">
          <div className="flex items-center space-x-3">
            <div className="animate-spin rounded-full h-6 w-6 border-b-2 border-primary-400" />
            <div>
              <div className="text-white font-medium">Uploading...</div>
              <div className="text-primary-300 text-sm">
                Please wait while your files are being processed
              </div>
            </div>
          </div>
        </div>
      )}

      {/* Tips */}
      <div className="card bg-gray-800/50">
        <h3 className="text-lg font-medium text-white mb-3">Upload Tips</h3>
        <ul className="space-y-2 text-sm text-gray-300">
          <li>• Supported audio formats: MP3, WAV, FLAC, OGG, M4A, AAC</li>
          <li>• Files are automatically scanned for metadata (artist, album, etc.)</li>
          <li>• Large files may take longer to process</li>
          <li>• Use categories to organize your music library</li>
          <li>• You can upload multiple files at once</li>
        </ul>
      </div>
    </div>
  );
};

export default Upload;
