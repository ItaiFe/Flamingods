import api from './api';

export const deviceApi = {
  // Get all discovered devices
  getDevices: async () => {
    try {
      const response = await api.get('/devices');
      return response.data;
    } catch (error) {
      throw new Error(`Failed to get devices: ${error.message}`);
    }
  },

  // Get specific device information
  getDevice: async (deviceId) => {
    try {
      const response = await api.get(`/devices/${deviceId}`);
      return response.data;
    } catch (error) {
      throw new Error(`Failed to get device ${deviceId}: ${error.message}`);
    }
  },

  // Control a specific device
  controlDevice: async (deviceId, control) => {
    try {
      const response = await api.post(`/devices/${deviceId}/control`, control);
      return response.data;
    } catch (error) {
      throw new Error(`Failed to control device ${deviceId}: ${error.message}`);
    }
  },

  // Set power state for a device
  setPower: async (deviceId, powerState) => {
    try {
      const response = await api.post(`/devices/${deviceId}/power/${powerState}`);
      return response.data;
    } catch (error) {
      throw new Error(`Failed to set power for device ${deviceId}: ${error.message}`);
    }
  },

  // Toggle device power
  toggleDevice: async (deviceId) => {
    try {
      const response = await api.post(`/devices/${deviceId}/toggle`);
      return response.data;
    } catch (error) {
      throw new Error(`Failed to toggle device ${deviceId}: ${error.message}`);
    }
  },

  // Bulk control multiple devices
  bulkControl: async (devices, control) => {
    try {
      const response = await api.post('/devices/bulk/control', {
        devices,
        control
      });
      return response.data;
    } catch (error) {
      throw new Error(`Failed to bulk control devices: ${error.message}`);
    }
  },

  // Discover new devices
  discoverDevices: async (forceRefresh = false) => {
    try {
      // Send minimal request body with only required fields
      const requestBody = {
        force_refresh: forceRefresh
      };
      
      console.log('Sending device discovery request:', requestBody);
      console.log('Device discovery may take up to 60 seconds with multi-threading...');
      
      // Use the existing api instance but with a custom timeout for discovery
      const response = await api.post('/discover', requestBody, {
        timeout: 90000 // 90 seconds for device discovery
      });
      console.log('Device discovery response:', response.data);
      return response.data;
    } catch (error) {
      console.error('Device discovery error:', error);
      if (error.response?.status === 422) {
        throw new Error(`Validation error: ${error.response.data?.detail || 'Invalid request data'}`);
      }
      if (error.code === 'ECONNABORTED') {
        throw new Error('Device discovery timed out after 90 seconds. Please try again.');
      }
      throw new Error(`Failed to discover devices: ${error.message}`);
    }
  },
};

export default deviceApi;
