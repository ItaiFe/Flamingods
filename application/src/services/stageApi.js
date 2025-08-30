import api from './api';

export const stageApi = {
  // Get current stage status
  getStatus: async () => {
    try {
      console.log('API: Calling /stage/status...');
      const response = await api.get('/stage/status');
      console.log('API: /stage/status response:', response.data);
      return response.data;
    } catch (error) {
      console.error('API: /stage/status error:', error);
      throw new Error(`Failed to get stage status: ${error.message}`);
    }
  },

  // Get stage health information
  getHealth: async () => {
    try {
      const response = await api.get('/stage/health');
      return response.data;
    } catch (error) {
      throw new Error(`Failed to get stage health: ${error.message}`);
    }
  },

  // Switch stage to IDLE lighting plan
  setIdle: async () => {
    try {
      const response = await api.post('/stage/idle');
      return response.data;
    } catch (error) {
      throw new Error(`Failed to set stage to IDLE: ${error.message}`);
    }
  },

  // Switch stage to SKIP lighting plan
  setSkip: async () => {
    try {
      const response = await api.post('/stage/skip');
      return response.data;
    } catch (error) {
      throw new Error(`Failed to set stage to SKIP: ${error.message}`);
    }
  },

  // Switch stage to SHOW lighting plan
  setShow: async () => {
    try {
      console.log('API: Calling POST /stage/show...');
      const response = await api.post('/stage/show');
      console.log('API: POST /stage/show response:', response.data);
      return response.data;
    } catch (error) {
      console.error('API: POST /stage/show error:', error);
      throw new Error(`Failed to set stage to SHOW: ${error.message}`);
    }
  },

  // Switch stage to SPECIAL lighting plan
  setSpecial: async () => {
    try {
      const response = await api.post('/stage/special');
      return response.data;
    } catch (error) {
      throw new Error(`Failed to set stage to SPECIAL: ${error.message}`);
    }
  },
};

export default stageApi;
