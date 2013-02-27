var net = require('net');

var HOST = '127.0.0.1';
var PORT = 6969;
var END = "$".charCodeAt(0);

net.createServer(function (socket) {

  var client = "[" + socket.remotePort + "]";
  console.log(client + ": connected");

  var received = new Buffer(0);
  var echo = false;

  socket.on('data', function (data) {
    var length;
    for (length = 0; length < data.length; length++) {
      if (data[length] === END) {
        echo = true;
        break;
      }
    }
    var slice = data.slice(0, length);
    console.log(client + ": " + slice.toString('hex'));
    received = Buffer.concat([received, slice]);

    if (echo) {
      socket.write(received);
    }
  });

  socket.on('close', function (data) {
    console.log(client + ": disconnected");
  });
    
}).listen(PORT, HOST);

console.log("Server listening on " + HOST + ":" + PORT);