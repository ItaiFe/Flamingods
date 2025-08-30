import { useState, useCallback, useEffect } from 'react';
import { systemApi } from '../services/systemApi';

export const useSystemMonitor = () => {
  const [systemStatus, setSystemStatus] = useState(null);
  const [websocketClients, setWebsocketClients] = useState([]);
  const [health, setHealth] = useState(null);
  const [isLoading, setIsLoading] = useState(false);
  const [error, setError] = useState(null);
  const [lastUpdate, setLastUpdate] = useState(null);

  // Get system status
  const getSystemStatus = useCallback(async () => {
    try {
      setIsLoading(true);
      setError(null);
      const status = await systemApi.getSystemStatus();
      setSystemStatus(status);
      setLastUpdate(new Date());
      return status;
    } catch (err) {
      setError(err.message);
      throw err;
    } finally {
      setIsLoading(false);
    }
  }, []);

  // Get WebSocket clients
  const getWebSocketClients = useCallback(async () => {
    try {
      setError(null);
      const clients = await systemApi.getWebSocketClients();
      setWebsocketClients(clients);
      return clients;
    } catch (err) {
      setError(err.message);
      throw err;
    }
  }, []);

  // Get health status
  const getHealth = useCallback(async () => {
    try {
      setError(null);
      const healthStatus = await systemApi.getHealth();
      setHealth(healthStatus);
      return healthStatus;
    } catch (err) {
      setError(err.message);
      throw err;
    }
  }, []);

  // Refresh all system data
  const refreshAll = useCallback(async () => {
    try {
      setIsLoading(true);
      setError(null);
      
      await Promise.all([
        getSystemStatus(),
        getWebSocketClients(),
        getHealth()
      ]);
      
      setLastUpdate(new Date());
    } catch (err) {
      setError(err.message);
    } finally {
      setIsLoading(false);
    }
  }, [getSystemStatus, getWebSocketClients, getHealth]);

  // Clear error
  const clearError = useCallback(() => {
    setError(null);
  }, []);

  // Load system data on mount
  useEffect(() => {
    refreshAll();
  }, [refreshAll]);

  // Auto-refresh every 30 seconds
  useEffect(() => {
    const interval = setInterval(() => {
      refreshAll();
    }, 30000);

    return () => clearInterval(interval);
  }, [refreshAll]);

  return {
    systemStatus,
    websocketClients,
    health,
    isLoading,
    error,
    lastUpdate,
    getSystemStatus,
    getWebSocketClients,
    getHealth,
    refreshAll,
    clearError,
  };
};
