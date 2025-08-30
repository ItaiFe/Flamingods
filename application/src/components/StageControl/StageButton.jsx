import React from 'react';
import { Play, Loader } from 'lucide-react';
import './StageButton.css';

const StageButton = ({ stage, label, description, isActive, isLoading, isTransitioning, transitioningTo, stageControl }) => {
  const { setIdle, setSkip, setShow, setSpecial } = stageControl;

  // Debug logging
  console.log(`StageButton ${stage} props:`, { 
    stage, 
    label, 
    isActive, 
    isLoading, 
    isTransitioning, 
    transitioningTo,
    hasStageControl: !!stageControl,
    hasSetIdle: !!setIdle,
    hasSetSkip: !!setSkip,
    hasSetShow: !!setShow,
    hasSetSpecial: !!setSpecial
  });

  const handleClick = async () => {
    if (isLoading) return;
    
    console.log(`Button clicked for stage: ${stage}`);
    
    try {
      switch (stage) {
        case 'idle':
          console.log('Calling setIdle...');
          await setIdle();
          break;
        case 'skip':
          console.log('Calling setSkip...');
          await setSkip();
          break;
        case 'show':
          console.log('Calling setShow...');
          await setShow();
          break;
        case 'special':
          console.log('Calling setSpecial...');
          await setSpecial();
          break;
        default:
          console.error('Unknown stage:', stage);
      }
    } catch (error) {
      console.error(`Failed to set stage to ${stage}:`, error);
    }
  };

  const getStageColor = () => {
    switch (stage) {
      case 'idle': return '#6b7280'; // Gray
      case 'skip': return '#f59e0b'; // Amber
      case 'show': return '#3b82f6'; // Blue
      case 'special': return '#8b5cf6'; // Purple
      default: return '#6b7280';
    }
  };

  const getStageIcon = () => {
    switch (stage) {
      case 'idle': return '⏸️';
      case 'skip': return '⏭️';
      case 'show': return '🎭';
      case 'special': return '✨';
      default: return '⚡';
    }
  };

  return (
    <button
      className={`stage-button ${stage} ${isActive ? 'active' : ''} ${isLoading ? 'loading' : ''}`}
      onClick={handleClick}
      disabled={isLoading}
      style={{ '--stage-color': getStageColor() }}
    >
      <div className="button-content">
        <div className="stage-icon">{getStageIcon()}</div>
        <h3 className="stage-label">{label}</h3>
        <p className="stage-description">{description}</p>
        
        {isLoading && (
          <div className="loading-overlay">
            <Loader className="loading-spinner" size={24} />
            <span>Switching...</span>
          </div>
        )}
        
        {isActive && (
          <div className="active-indicator">
            <Play size={16} />
            <span>ACTIVE</span>
          </div>
        )}
        
        {isTransitioning && transitioningTo === stage && (
          <div className="transitioning-indicator">
            <Loader size={16} />
            <span>TRANSITIONING...</span>
          </div>
        )}
      </div>
    </button>
  );
};

export default StageButton;
