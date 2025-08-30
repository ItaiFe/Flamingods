import React from 'react';
import { Clock, Loader } from 'lucide-react';
import './StageStatus.css';

const StageStatus = ({ currentStage, lastUpdate, isLoading, isTransitioning, transitioningTo }) => {
  // Debug logging
  console.log('StageStatus props:', { currentStage, lastUpdate, isLoading, isTransitioning, transitioningTo });
  const formatLastUpdate = (date) => {
    if (!date) return 'Never';
    
    const now = new Date();
    const diff = now - date;
    const minutes = Math.floor(diff / 60000);
    const seconds = Math.floor((diff % 60000) / 1000);
    
    if (minutes > 0) {
      return `${minutes}m ${seconds}s ago`;
    }
    return `${seconds}s ago`;
  };

  const getStageDisplayName = (stage) => {
    // Handle both string and numeric stage values from ESP
    switch (stage) {
      case 'idle':
      case 0:
        return 'IDLE';
      case 'skip':
      case 1:
        return 'SKIP';
      case 'show':
      case 2:
        return 'SHOW';
      case 'special':
      case 3:
        return 'SPECIAL';
      default:
        return 'UNKNOWN';
    }
  };

  const getStageColor = (stage) => {
    // Handle both string and numeric stage values from ESP
    switch (stage) {
      case 'idle':
      case 0:
        return '#6b7280';
      case 'skip':
      case 1:
        return '#f59e0b';
      case 'show':
      case 2:
        return '#3b82f6';
      case 'special':
      case 3:
        return '#8b5cf6';
      default:
        return '#6b7280';
    }
  };

  return (
    <div className="stage-status">
      <div className="status-header">
        <h4>Current Status</h4>
        {isLoading && <Loader className="status-spinner" size={16} />}
      </div>
      
      <div className="status-content">
        <div className="current-stage">
          <span className="stage-label">Stage:</span>
          <span 
            className="stage-value"
            style={{ 
              color: getStageColor(isTransitioning ? transitioningTo : currentStage)
            }}
          >
            {isTransitioning ? (
              <span className="transitioning">
                {getStageDisplayName(transitioningTo)} (TRANSITIONING...)
              </span>
            ) : (
              getStageDisplayName(currentStage)
            )}
          </span>
        </div>
        
        <div className="last-update">
          <Clock size={14} />
          <span>
            {isTransitioning ? 'Transitioning...' : `Last updated: ${formatLastUpdate(lastUpdate)}`}
          </span>
        </div>
        
        {isTransitioning && (
          <div className="transition-progress">
            <div className="progress-bar">
              <div className="progress-fill"></div>
            </div>
            <span className="progress-text">Waiting for ESP response...</span>
          </div>
        )}
        

      </div>
    </div>
  );
};

export default StageStatus;
