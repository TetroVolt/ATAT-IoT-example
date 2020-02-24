const express = require('express');
const http = require('http');
const socketio = require('socket.io');

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

  io.on('connection', function(socket){
    socket.on('chat message', function(msg){
      io.emit('chat message', msg);
    });
  });

  return { app, httpServer, io };
}

const devApp = initDevApp();
devApp.httpServer.listen(3001, () => {
  console.log("Dev server listening on port 3001");
})

/*
var server = net.createServer(function(socket) {
  console.log("Client connected. ");

  socket.on('close', () => {
    console.log("Client disconnected. ")
  })

  socket.on

  socket.write("Hello, World!\r\n");
	socket.pipe(socket);
});
*/