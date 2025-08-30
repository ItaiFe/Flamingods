import React from 'react';
import { useAudio } from '../contexts/AudioContext';
import { AlertCircle, X } from 'lucide-react';

interface ErrorAlertProps {
  message: string;
}

const ErrorAlert: React.FC<ErrorAlertProps> = ({ message }) => {
  const { clearError } = useAudio();

  return (
    <div className="bg-red-900/20 border border-red-700 rounded-lg p-4">
      <div className="flex items-start space-x-3">
        <AlertCircle className="h-5 w-5 text-red-400 flex-shrink-0 mt-0.5" />
        
        <div className="flex-1">
          <h3 className="text-sm font-medium text-red-400">
            Error
          </h3>
          <div className="mt-1 text-sm text-red-300">
            {message}
          </div>
        </div>

        <button
          onClick={clearError}
          className="flex-shrink-0 p-1 bg-red-900/50 hover:bg-red-900/70 rounded-lg transition-colors"
        >
          <X className="h-4 w-4 text-red-400" />
        </button>
      </div>
    </div>
  );
};

export default ErrorAlert;
