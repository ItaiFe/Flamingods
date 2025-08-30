import React from 'react';
import { useStageControl } from '../../hooks/useStageControl';
import Header from './Header';
import Sidebar from './Sidebar';
import StageControlPanel from '../StageControl/StageControlPanel';
import DeviceList from '../DeviceManagement/DeviceList';
import SystemStatus from '../SystemMonitor/SystemStatus';
import LoadingOverlay from './LoadingOverlay';
import './Dashboard.css';

const Dashboard = () => {
  const stageControl = useStageControl();
  const { isTransitioning, transitioningTo } = stageControl;

  return (
    <div className="dashboard">
      <Header />
      <div className="dashboard-content">
        <main className="main-content">
          <StageControlPanel stageControl={stageControl} />
        </main>
        <Sidebar>
          <DeviceList />
          <SystemStatus />
        </Sidebar>
      </div>
      
      {/* Full-screen loading overlay during stage transitions */}
      {isTransitioning && <LoadingOverlay targetStage={transitioningTo} />}
    </div>
  );
};

export default Dashboard;
