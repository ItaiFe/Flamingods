import axios from 'axios';

// Create axios instance with base configuration
// Use relative URLs to work with the proxy configuration in package.json
const api = axios.create({
  baseURL: '', // Empty baseURL to use relative paths with proxy
  timeout: 60000, // Increased to 60 seconds for device discovery
  headers: {
    'Content-Type': 'application/json',
  },
});

// Request interceptor for logging
api.interceptors.request.use(
  (config) => {
    console.log(`API Request: ${config.method?.toUpperCase()} ${config.url}`);
    return config;
  },
  (error) => {
    console.error('API Request Error:', error);
    return Promise.reject(error);
  }
);

// Response interceptor for error handling
api.interceptors.response.use(
  (response) => {
    return response;
  },
  (error) => {
    if (error.response) {
      // Server responded with error status
      console.error('API Error Response:', error.response.data);
      console.error('Status:', error.response.status);
    } else if (error.request) {
      // Request made but no response received
      console.error('API No Response:', error.request);
    } else {
      // Something else happened
      console.error('API Error:', error.message);
    }
    return Promise.reject(error);
  }
);

export default api;
