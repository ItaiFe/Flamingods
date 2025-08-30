import api from './api';

export const systemApi = {
  // Get overall system status
  getSystemStatus: async () => {
    try {
      const response = await api.get('/system/status');
      return response.data;
    } catch (error) {
      throw new Error(`Failed to get system status: ${error.message}`);
    }
  },

  // Get WebSocket client information
  getWebSocketClients: async () => {
    try {
      const response = await api.get('/system/clients');
      return response.data;
    } catch (error) {
      throw new Error(`Failed to get WebSocket clients: ${error.message}`);
    }
  },

  // Get basic health check
  getHealth: async () => {
    try {
      const response = await api.get('/health');
      return response.data;
    } catch (error) {
      throw new Error(`Failed to get health status: ${error.message}`);
    }
  },
};

export default systemApi;
