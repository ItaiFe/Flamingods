import React, { useState } from 'react';
import { useDeviceManagement } from '../../hooks/useDeviceManagement';
import { Power, PowerOff, CheckSquare, Square } from 'lucide-react';
import './BulkControl.css';

const BulkControl = ({ devices }) => {
  const { bulkControl } = useDeviceManagement();
  const [selectedDevices, setSelectedDevices] = useState([]);
  const [isControlling, setIsControlling] = useState(false);

  const onlineDevices = devices.filter(d => d.online);
  const allSelected = selectedDevices.length === onlineDevices.length && onlineDevices.length > 0;
  const someSelected = selectedDevices.length > 0 && !allSelected;

  const handleSelectAll = () => {
    if (allSelected) {
      setSelectedDevices([]);
    } else {
      setSelectedDevices(onlineDevices.map(d => d.id));
    }
  };

  const handleDeviceSelect = (deviceId) => {
    setSelectedDevices(prev => 
      prev.includes(deviceId) 
        ? prev.filter(id => id !== deviceId)
        : [...prev, deviceId]
    );
  };

  const handleBulkAction = async (powerState) => {
    if (selectedDevices.length === 0) return;
    
    setIsControlling(true);
    try {
      await bulkControl(selectedDevices, { power: powerState });
      setSelectedDevices([]); // Clear selection after action
    } catch (error) {
      console.error('Bulk control failed:', error);
    } finally {
      setIsControlling(false);
    }
  };

  if (onlineDevices.length === 0) {
    return null;
  }

  return (
    <div className="bulk-control">
      <div className="bulk-header">
        <h4>Bulk Control</h4>
        <div className="selection-info">
          {selectedDevices.length} of {onlineDevices.length} selected
        </div>
      </div>

      <div className="bulk-selection">
        <button 
          className={`select-all-button ${allSelected ? 'selected' : ''}`}
          onClick={handleSelectAll}
        >
          {allSelected ? <CheckSquare size={16} /> : <Square size={16} />}
          {allSelected ? 'Deselect All' : 'Select All'}
        </button>
      </div>

      {selectedDevices.length > 0 && (
        <div className="bulk-actions">
          <button
            className="bulk-on-button"
            onClick={() => handleBulkAction('on')}
            disabled={isControlling}
          >
            <Power size={16} />
            Turn All On
          </button>
          
          <button
            className="bulk-off-button"
            onClick={() => handleBulkAction('off')}
            disabled={isControlling}
          >
            <PowerOff size={16} />
            Turn All Off
          </button>
        </div>
      )}

      <div className="device-selection-list">
        {onlineDevices.map(device => (
          <label key={device.id} className="device-selection-item">
            <input
              type="checkbox"
              checked={selectedDevices.includes(device.id)}
              onChange={() => handleDeviceSelect(device.id)}
            />
            <span className="device-name">{device.name || device.id}</span>
            <span className={`device-status ${device.power}`}>
              {device.power === 'on' ? 'On' : 'Off'}
            </span>
          </label>
        ))}
      </div>
    </div>
  );
};

export default BulkControl;
