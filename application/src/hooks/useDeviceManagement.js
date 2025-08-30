import { useState, useCallback, useEffect } from 'react';
import { deviceApi } from '../services/deviceApi';

export const useDeviceManagement = () => {
  const [devices, setDevices] = useState([]);
  const [isLoading, setIsLoading] = useState(false);
  const [error, setError] = useState(null);
  const [lastUpdate, setLastUpdate] = useState(null);

  // Get all devices
  const getDevices = useCallback(async () => {
    try {
      setIsLoading(true);
      setError(null);
      const deviceList = await deviceApi.getDevices();
      setDevices(deviceList);
      setLastUpdate(new Date());
      return deviceList;
    } catch (err) {
      console.warn('Failed to get devices:', err.message);
      setError(`Unable to load devices: ${err.message}`);
      // Return empty array instead of throwing
      return [];
    } finally {
      setIsLoading(false);
    }
  }, []);

  // Control a specific device
  const controlDevice = useCallback(async (deviceId, control) => {
    try {
      setError(null);
      const result = await deviceApi.controlDevice(deviceId, control);
      
      // Update local device state
      setDevices(prevDevices => 
        prevDevices.map(device => 
          device.id === deviceId 
            ? { ...device, power: control.power, last_updated: new Date().toISOString() }
            : device
        )
      );
      
      return result;
    } catch (err) {
      console.warn('Failed to control device:', err.message);
      setError(`Failed to control device: ${err.message}`);
      return null;
    }
  }, []);

  // Set power state for a device
  const setPower = useCallback(async (deviceId, powerState) => {
    try {
      setError(null);
      const result = await deviceApi.setPower(deviceId, powerState);
      
      // Update local device state
      setDevices(prevDevices => 
        prevDevices.map(device => 
          device.id === deviceId 
            ? { ...device, power: powerState, last_updated: new Date().toISOString() }
            : device
        )
      );
      
      return result;
    } catch (err) {
      console.warn('Failed to set device power:', err.message);
      setError(`Failed to set device power: ${err.message}`);
      return null;
    }
  }, []);

  // Toggle device power
  const toggleDevice = useCallback(async (deviceId) => {
    try {
      setError(null);
      const result = await deviceApi.toggleDevice(deviceId);
      
      // Update local device state
      setDevices(prevDevices => 
        prevDevices.map(device => 
          device.id === deviceId 
            ? { ...device, power: device.power === 'on' ? 'off' : 'on', last_updated: new Date().toISOString() }
            : device
        )
      );
      
      return result;
    } catch (err) {
      console.warn('Failed to toggle device:', err.message);
      setError(`Failed to toggle device: ${err.message}`);
      return null;
    }
  }, []);

  // Bulk control multiple devices
  const bulkControl = useCallback(async (deviceIds, control) => {
    try {
      setError(null);
      const result = await deviceApi.bulkControl(deviceIds, control);
      
      // Update local device states
      setDevices(prevDevices => 
        prevDevices.map(device => 
          deviceIds.includes(device.id)
            ? { ...device, power: control.power, last_updated: new Date().toISOString() }
            : device
        )
      );
      
      return result;
    } catch (err) {
      console.warn('Failed to bulk control devices:', err.message);
      setError(`Failed to bulk control devices: ${err.message}`);
      return null;
    }
  }, []);

  // Discover new devices
  const discoverDevices = useCallback(async () => {
    try {
      setIsLoading(true);
      setError(null);
      
      console.log('Starting device discovery with multi-threading...');
      const result = await deviceApi.discoverDevices();
      
      console.log('Device discovery completed successfully');
      
      // Refresh device list after discovery
      await getDevices();
      
      return result;
    } catch (err) {
      console.warn('Failed to discover devices:', err.message);
      setError(`Failed to discover devices: ${err.message}`);
      return null;
    } finally {
      setIsLoading(false);
    }
  }, [getDevices]);

  // Clear error
  const clearError = useCallback(() => {
    setError(null);
  }, []);

  // Load devices on mount
  useEffect(() => {
    getDevices();
  }, [getDevices]);

  return {
    devices,
    isLoading,
    error,
    lastUpdate,
    getDevices,
    controlDevice,
    setPower,
    toggleDevice,
    bulkControl,
    discoverDevices,
    clearError,
  };
};
