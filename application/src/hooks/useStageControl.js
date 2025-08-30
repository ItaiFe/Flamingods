import { useState, useCallback, useEffect } from 'react';
import { stageApi } from '../services/stageApi';

export const useStageControl = () => {
  const [currentStage, setCurrentStage] = useState('idle');
  const [isLoading, setIsLoading] = useState(false);
  const [isTransitioning, setIsTransitioning] = useState(false);
  const [transitioningTo, setTransitioningTo] = useState(null);
  const [error, setError] = useState(null);
  const [lastUpdate, setLastUpdate] = useState(null);

  // Debug: Log state changes
  useEffect(() => {
    console.log('Stage state changed:', { currentStage, isTransitioning, transitioningTo, lastUpdate });
  }, [currentStage, isTransitioning, transitioningTo, lastUpdate]);

  // Get current stage status
  const getStageStatus = useCallback(async () => {
    try {
      setIsLoading(true);
      setError(null);
      console.log('Fetching stage status...');
      const status = await stageApi.getStatus();
      console.log('Received stage status:', status);
      
      // Convert numeric stage values to string names for consistency
      let stageName = 'idle';
      if (status.current_plan !== undefined) {
        switch (status.current_plan) {
          case 0:
            stageName = 'idle';
            break;
          case 1:
            stageName = 'skip';
            break;
          case 2:
            stageName = 'show';
            break;
          case 3:
            stageName = 'special';
            break;
          default:
            stageName = 'idle';
        }
      }
      
      console.log('Setting stage to:', stageName);
      setCurrentStage(stageName);
      setLastUpdate(new Date());
      return status;
    } catch (err) {
      console.warn('Failed to get stage status:', err.message);
      setError(`Unable to get stage status: ${err.message}`);
      // Don't throw the error, just set it in state
      return null;
    } finally {
      setIsLoading(false);
    }
  }, []);

  // Poll status until it changes to the target stage
  const pollStatusUntilChange = useCallback(async (targetStage, maxAttempts = 30) => {
    console.log(`Polling status until change to ${targetStage}...`);
    
    let attempts = 0;
    const pollInterval = 250; // Check every 250ms
    
    while (attempts < maxAttempts) {
      try {
        await new Promise(resolve => setTimeout(resolve, pollInterval));
        
        const status = await stageApi.getStatus();
        console.log(`Poll attempt ${attempts + 1}:`, status);
        
        // Convert numeric stage values to string names
        let currentStageName = 'idle';
        if (status.current_plan !== undefined) {
          switch (status.current_plan) {
            case 0:
              currentStageName = 'idle';
              break;
            case 1:
              currentStageName = 'skip';
              break;
            case 2:
              currentStageName = 'show';
              break;
            case 3:
              currentStageName = 'special';
              break;
            default:
              currentStageName = 'idle';
          }
        }
        
        console.log(`Current stage: ${currentStageName}, Target: ${targetStage}`);
        
        // Check if the stage has changed to the target
        if (currentStageName === targetStage) {
          console.log(`Stage successfully changed to ${targetStage} after ${attempts + 1} attempts`);
          setCurrentStage(targetStage);
          setLastUpdate(new Date());
          return true;
        }
        
        attempts++;
      } catch (err) {
        console.warn(`Poll attempt ${attempts + 1} failed:`, err.message);
        attempts++;
      }
    }
    
    console.warn(`Failed to confirm stage change to ${targetStage} after ${maxAttempts} attempts`);
    return false;
  }, []);

  // Set stage to IDLE
  const setIdle = useCallback(async () => {
    try {
      setIsLoading(true);
      setError(null);
      console.log('Setting stage to IDLE...');
      
      // Set transition state
      setIsTransitioning(true);
      setTransitioningTo('idle');
      
      const result = await stageApi.setIdle();
      console.log('IDLE result:', result);
      
      // Poll status until it actually changes to IDLE
      console.log('Polling for IDLE status change...');
      const success = await pollStatusUntilChange('idle');
      
      if (success) {
        console.log('IDLE status change confirmed successfully');
        // Force refresh the status to ensure UI updates
        await getStageStatus();
        // Keep transition state for a moment to show completion
        setTimeout(() => {
          setIsTransitioning(false);
          setTransitioningTo(null);
        }, 500);
      } else {
        console.warn('Failed to confirm IDLE status change');
        setError('Stage change may not have completed successfully');
        // Clear transition state immediately on failure
        setIsTransitioning(false);
        setTransitioningTo(null);
      }
      
      return result;
    } catch (err) {
      console.warn('Failed to set stage to IDLE:', err.message);
      setError(`Failed to set stage to IDLE: ${err.message}`);
      setIsTransitioning(false);
      setTransitioningTo(null);
      return null;
    } finally {
      setIsLoading(false);
    }
  }, [pollStatusUntilChange]);

  // Set stage to SKIP
  const setSkip = useCallback(async () => {
    try {
      setIsLoading(true);
      setError(null);
      console.log('Setting stage to SKIP...');
      
      // Set transition state
      setIsTransitioning(true);
      setTransitioningTo('skip');
      
      const result = await stageApi.setSkip();
      console.log('SKIP result:', result);
      
      // Poll status until it actually changes to SKIP
      console.log('Polling for SKIP status change...');
      const success = await pollStatusUntilChange('skip');
      
      if (success) {
        console.log('SKIP status change confirmed successfully');
        // Force refresh the status to ensure UI updates
        await getStageStatus();
        // Keep transition state for a moment to show completion
        setTimeout(() => {
          setIsTransitioning(false);
          setTransitioningTo(null);
        }, 500);
      } else {
        console.warn('Failed to confirm SKIP status change');
        setError('Stage change may not have completed successfully');
        // Clear transition state immediately on failure
        setIsTransitioning(false);
        setTransitioningTo(null);
      }
      
      return result;
    } catch (err) {
      console.warn('Failed to set stage to SKIP:', err.message);
      setError(`Failed to set stage to SKIP: ${err.message}`);
      setIsTransitioning(false);
      setTransitioningTo(null);
      return null;
    } finally {
      setIsLoading(false);
    }
  }, [pollStatusUntilChange]);

  // Set stage to SHOW
  const setShow = useCallback(async () => {
    try {
      setIsLoading(true);
      setError(null);
      console.log('Setting stage to SHOW...');
      
      // Set transition state
      setIsTransitioning(true);
      setTransitioningTo('show');
      
      const result = await stageApi.setShow();
      console.log('SHOW result:', result);
      
      // Poll status until it actually changes to SHOW
      console.log('Polling for SHOW status change...');
      const success = await pollStatusUntilChange('show');
      
      if (success) {
        console.log('SHOW status change confirmed successfully');
        // Force refresh the status to ensure UI updates
        await getStageStatus();
        // Keep transition state for a moment to show completion
        setTimeout(() => {
          setIsTransitioning(false);
          setTransitioningTo(null);
        }, 500);
      } else {
        console.warn('Failed to confirm SHOW status change');
        setError('Stage change may not have completed successfully');
        // Clear transition state immediately on failure
        setIsTransitioning(false);
        setTransitioningTo(null);
      }
      
      return result;
    } catch (err) {
      console.warn('Failed to set stage to SHOW:', err.message);
      setError(`Failed to set stage to SHOW: ${err.message}`);
      setIsTransitioning(false);
      setTransitioningTo(null);
      return null;
    } finally {
      setIsLoading(false);
    }
  }, [pollStatusUntilChange]);

  // Set stage to SPECIAL
  const setSpecial = useCallback(async () => {
    try {
      setIsLoading(true);
      setError(null);
      console.log('Setting stage to SPECIAL...');
      
      // Set transition state
      setIsTransitioning(true);
      setTransitioningTo('special');
      
      const result = await stageApi.setSpecial();
      console.log('SPECIAL result:', result);
      
      // Poll status until it actually changes to SPECIAL
      console.log('Polling for SPECIAL status change...');
      const success = await pollStatusUntilChange('special');
      
      if (success) {
        console.log('SPECIAL status change confirmed successfully');
        // Force refresh the status to ensure UI updates
        await getStageStatus();
        // Keep transition state for a moment to show completion
        setTimeout(() => {
          setIsTransitioning(false);
          setTransitioningTo(null);
        }, 500);
      } else {
        console.warn('Failed to confirm SPECIAL status change');
        setError('Stage change may not have completed successfully');
        // Clear transition state immediately on failure
        setIsTransitioning(false);
          setTransitioningTo(null);
      }
      
      return result;
    } catch (err) {
      console.warn('Failed to set stage to SPECIAL:', err.message);
      setError(`Failed to set stage to SPECIAL: ${err.message}`);
      setIsTransitioning(false);
      setTransitioningTo(null);
      return null;
    } finally {
      setIsLoading(false);
    }
  }, [pollStatusUntilChange]);

  // Clear error
  const clearError = useCallback(() => {
    setError(null);
  }, []);

  return {
    currentStage,
    isLoading,
    isTransitioning,
    transitioningTo,
    error,
    lastUpdate,
    getStageStatus,
    pollStatusUntilChange,
    setIdle,
    setSkip,
    setShow,
    setSpecial,
    clearError,
  };
};
