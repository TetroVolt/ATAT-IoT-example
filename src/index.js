const express = require('express');
const http = require('http');
const socketio = require('socket.io');
const net = require('net');

function initDevApp() {
  const app = express();
  const httpServer = http.createServer(app);
  const io = socketio(httpServer);

  app.get('/health', function(req, res){
    res.sendStatus(200);
  });

  app.get('/', function(req, res){
    res.sendFile(__dirname + '/index.html');
  });

  return { app, httpServer, io };
}

const devApp = initDevApp();

devApp.io.on('connection', function(socket){
  console.log('[Dev Server] developer connected.');
  socket.on('disconnect', function() {
    console.log('[Dev Server] developer disconnected.')
  })
});

var rfidServer = net.createServer(function(socket) {
  console.log("[RFID Server] Client connected. ");

  socket.on('end', () => {
    console.log("[RFID Server] Client disconnected. ")
  })

  socket.on('data', (chunk) => {
    console.log(`[RFID Server] received data from client: ${chunk.toString()}`);
    devApp.io.emit('rfid event', chunk.toString());
  })
});

devApp.httpServer.listen(3001, () => {
  console.log("[Dev Server] listening on port 3001");
})

rfidServer.listen(3000, () => {
  console.log("[RFID server] listening on port 3000")
})
