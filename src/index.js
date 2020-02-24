var app = require('express')();
var http = require('http').createServer(app);
var io = require('socket.io')(http);

const PORT = 3000

app.get('/', (request, response) => {
  console.log("Received request: ", request);
  response.sendStatus(200);
})

app.get('/health', (request, response) => {
  console.log("Received health check from: ", request);
  response.sendStatus(200);
})

io.on('connection', function(socket) {
  console.log("Socket connection received.");
  socket.send("Hello, World");
})

http.listen(PORT, function(){
  console.log('listening on *:3000');
});