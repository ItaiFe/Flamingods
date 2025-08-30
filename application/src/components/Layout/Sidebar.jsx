import React from 'react';
import './Sidebar.css';

const Sidebar = ({ children }) => {
  return (
    <aside className="sidebar">
      {children}
    </aside>
  );
};

export default Sidebar;
