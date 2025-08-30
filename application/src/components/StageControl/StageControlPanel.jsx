import React, { useEffect } from 'react';
import StageButton from './StageButton';
import StageStatus from './StageStatus';
import './StageControlPanel.css';

const StageControlPanel = ({ stageControl }) => {
  const {
    currentStage,
    isLoading,
    isTransitioning,
    transitioningTo,
    error,
    lastUpdate,
    getStageStatus,
    clearError
  } = stageControl;

  // Debug logging
  console.log('StageControlPanel state:', { 
    currentStage, 
    isLoading, 
    isTransitioning, 
    transitioningTo, 
    error, 
    lastUpdate 
  });

  useEffect(() => {
    getStageStatus();
  }, [getStageStatus]);

  const handleErrorDismiss = () => {
    clearError();
  };

  return (
    <div className="stage-control-panel">
      <div className="panel-header">
        <h2>Stage Control</h2>
        <StageStatus 
          currentStage={currentStage}
          lastUpdate={lastUpdate}
          isLoading={isLoading}
          isTransitioning={isTransitioning}
          transitioningTo={transitioningTo}
        />
      </div>

      {error && (
        <div className="error-banner" onClick={handleErrorDismiss}>
          <span className="error-message">{error}</span>
          <span className="error-dismiss">Click to dismiss</span>
        </div>
      )}

      <div className="stage-buttons-grid">
        <StageButton
          stage="idle"
          label="IDLE"
          description="Default standby lighting"
          isActive={currentStage === 'idle'}
          isLoading={isLoading}
          isTransitioning={isTransitioning}
          transitioningTo={transitioningTo}
          stageControl={stageControl}
        />
        
        <StageButton
          stage="skip"
          label="SKIP"
          description="Transition lighting"
          isActive={currentStage === 'skip'}
          isLoading={isLoading}
          isTransitioning={isTransitioning}
          transitioningTo={transitioningTo}
          stageControl={stageControl}
        />
        
        <StageButton
          stage="show"
          label="SHOW"
          description="Performance lighting"
          isActive={currentStage === 'show'}
          isLoading={isLoading}
          isTransitioning={isTransitioning}
          transitioningTo={transitioningTo}
          stageControl={stageControl}
        />
        
        <StageButton
          stage="special"
          label="SPECIAL"
          description="Special effects lighting"
          isActive={currentStage === 'special'}
          isLoading={isLoading}
          isTransitioning={isTransitioning}
          transitioningTo={transitioningTo}
          stageControl={stageControl}
        />
      </div>

      <div className="panel-footer">
        <button 
          className="refresh-button"
          onClick={getStageStatus}
          disabled={isLoading}
        >
          {isLoading ? 'Refreshing...' : 'Refresh Status'}
        </button>
      </div>
    </div>
  );
};

export default StageControlPanel;
