const express = require('express');
const app = express();
const PORT = 3000;

// Middleware to parse JSON
app.use(express.json());

// RFID endpoint - receives data from ESP32
app.post('/api/rfid', (req, res) => {
  const { deviceID, rfid } = req.body;
  
  console.log('\n========================================');
  console.log('📡 RFID DATA RECEIVED');
  console.log('========================================');
  console.log('Device ID:', deviceID);
  console.log('RFID Code:', rfid);
  console.log('Timestamp:', new Date().toLocaleString());
  console.log('========================================\n');
  
  // Send success response
  res.status(200).json({ 
    success: true, 
    message: 'RFID data received',
    deviceID,
    rfid
  });
});

// Health check endpoint
app.get('/api/health', (req, res) => {
  res.json({ status: 'Server is running', timestamp: new Date().toISOString() });
});

// Start server
app.listen(PORT, () => {
  console.log('\n========================================');
  console.log('🚀 L.I.G.T.A.S RFID Server Started');
  console.log('========================================');
  console.log(`Server running on port ${PORT}`);
  console.log(`Endpoint: http://localhost:${PORT}/api/rfid`);
  console.log('Waiting for RFID data...');
  console.log('========================================\n');
});
