// Generated by CoffeeScript 1.8.0
(function() {
  var Encryptor, KEY, LOCAL_ADDRESS, METHOD, PORT, REMOTE_PORT, SCHEME, SERVER, WebSocket, config, configContent, configFromArgs, fs, getServer, http, inetNtoa, k, net, options, parseArgs, path, prepareServer, s, server, timeout, url, v, _ref;

  net = require("net");

  url = require("url");

  http = require("http");

  fs = require("fs");

  path = require("path");

  WebSocket = require('ws');

  parseArgs = require("minimist");

  Encryptor = require("./encrypt").Encryptor;

  moment = require("moment");

  util = require("util");

  wstream = fs.createWriteStream('/tmp/shadowsocks.log', {flags : 'w'});
  //var log_stdout = process.stdout;
  console.log = function(d) {
      wstream.write("[" + moment().utc().format("YYYY-MM-DDTHH:mm") + "] " + util.format(d) + '\n');
      //log_stdout.write("[" + moment().utc().format("YYYY-MM-DDTHH:mm") + "] " + util.format(d) + '\n');
  };

  console.warn = function(d) {
      wstream.write("[" + moment().utc().format("YYYY-MM-DDTHH:mm") + "][WARN] " + util.format(d) + '\n');
      //log_stdout.write("[" + moment().utc().format("YYYY-MM-DDTHH:mm") + "][WARN] " + util.format(d) + '\n');
  };

  options = {
    alias: {
      'b': 'local_address',
      'l': 'local_port',
      's': 'server',
      'r': 'remote_port',
      'k': 'password',
      'c': 'config_file',
      'm': 'method'
    },
    string: ['local_address', 'server', 'password', 'config_file', 'method', 'scheme'],
    "default": {
      'config_file': path.resolve(__dirname, "config.json")
    }
  };

  inetNtoa = function(buf) {
    return buf[0] + "." + buf[1] + "." + buf[2] + "." + buf[3];
  };

  configFromArgs = parseArgs(process.argv.slice(2), options);

  configContent = fs.readFileSync(configFromArgs.config_file);

  config = JSON.parse(configContent);

  for (k in configFromArgs) {
    v = configFromArgs[k];
    config[k] = v;
  }

  SCHEME = config.scheme;

  SERVER = config.server;

  REMOTE_PORT = config.remote_port;

  LOCAL_ADDRESS = config.local_address;

  PORT = config.local_port;

  KEY = config.password;

  METHOD = config.method.toLowerCase();

  timeout = Math.floor(config.timeout * 1000);

  console.log("get method: " + METHOD + ", key: " + KEY);

  if (METHOD === "" || METHOD === "null") {
    METHOD = null;
  }

  prepareServer = function(address) {
    var serverUrl;
    serverUrl = url.parse(address);
    serverUrl.slashes = true;
    if (serverUrl.protocol == null) {
      serverUrl.protocol = SCHEME;
    }
    if (serverUrl.hostname === null) {
      serverUrl.hostname = address;
      serverUrl.pathname = '/';
    }
    if (serverUrl.port == null) {
      serverUrl.port = REMOTE_PORT;
    }
    return url.format(serverUrl);
  };

  if (SERVER instanceof Array) {
    SERVER = (function() {
      var _i, _len, _results;
      _results = [];
      for (_i = 0, _len = SERVER.length; _i < _len; _i++) {
        s = SERVER[_i];
        _results.push(prepareServer(s));
      }
      return _results;
    })();
  } else {
    SERVER = prepareServer(SERVER);
  }

  getServer = function() {
    if (SERVER instanceof Array) {
      return SERVER[Math.floor(Math.random() * SERVER.length)];
    } else {
      return SERVER;
    }
  };

  setInterval(function() {
    if (global.gc) {
      return gc();
    }
  }, 1000);

  server = net.createServer(function(connection) {
    var aServer, addrLen, addrToSend, cachedPieces, encryptor, headerLength, ping, remoteAddr, remotePort, stage, ws;
    //console.log("local connected");
    //server.getConnections(function(err, count) {
    //  console.log("concurrent connections:" + count);
    //});
    encryptor = new Encryptor(KEY, METHOD);
    stage = 0;
    headerLength = 0;
    cachedPieces = [];
    addrLen = 0;
    ws = null;
    ping = null;
    remoteAddr = null;
    remotePort = null;
    addrToSend = "";
    aServer = getServer();
    connection.on("data", function(data) {
      var addrtype, buf, cmd, e, reply, tempBuf;

      //console.log("[1]get " + data.length + " data from local" + ", stage: " + stage);

      if (stage === 5) {
        data = encryptor.encrypt(data);
        if (ws.readyState === WebSocket.OPEN) {
          //console.log("[2]send " + data.length + " data to server, get bufferedAmount: " + ws.bufferedAmount + ", stage: " + stage);
          ws.send(data, {
            binary: true
          });
          //if (ws.bufferedAmount > 0) {
            //console.log("[23]pause local, get bufferedAmount: " + ws.bufferedAmount);
            //connection.pause();
          //}
        }
        return;
      }
      if (stage === 0) {
        tempBuf = new Buffer(2);
        tempBuf.write("\u0005\u0000", 0);
        //console.log("[3]send " + tempBuf.length + " data to local" + ", stage: " + stage);
        connection.write(tempBuf);
        stage = 1;
        return;
      }
      if (stage === 1) {
        try {
          cmd = data[1];
          addrtype = data[3];
          if (cmd !== 1) {
            console.warn("[4]unsupported cmd:" + cmd + ", stage: " + stage);
            reply = new Buffer("\u0005\u0007\u0000\u0001", "binary");
            connection.end(reply);
            return;
          }
          if (addrtype === 3) {
            addrLen = data[4];
          } else if (addrtype !== 1) {
            console.warn("[5]unsupported addrtype:" + addrtype + ", stage: " + stage);
            connection.end();
            return;
          }
          addrToSend = data.slice(3, 4).toString("binary");
          if (addrtype === 1) {
            remoteAddr = inetNtoa(data.slice(4, 8));
            addrToSend += data.slice(4, 10).toString("binary");
            remotePort = data.readUInt16BE(8);
            headerLength = 10;
          } else {
            remoteAddr = data.slice(5, 5 + addrLen).toString("binary");
            addrToSend += data.slice(4, 5 + addrLen + 2).toString("binary");
            remotePort = data.readUInt16BE(5 + addrLen);
            headerLength = 5 + addrLen + 2;
          }
          buf = new Buffer(10);
          buf.write("\u0005\u0000\u0000\u0001", 0, 4, "binary");
          buf.write("\u0000\u0000\u0000\u0000", 4, 4, "binary");
          buf.writeInt16BE(remotePort, 8);
          //console.log("[6]send " + buf.length + " data to local" + ", stage: " + stage);
          connection.write(buf);
          ws = new WebSocket(aServer, {
            protocol: "binary"
          });
          ws.on("open", function() {
            var addrToSendBuf, i, piece;
            ws._socket.on("error", function(e) {
              console.warn("[7]remote " + remoteAddr + ":" + remotePort + " " + e + ", stage: " + stage);
              connection.destroy();
              return server.getConnections(function(err, count) {
                console.log("[8]concurrent connections:" + count + ", stage: " + stage);
              });
            });
            //console.log("[9]connecting " + remoteAddr + " via " + aServer + ", stage: " + stage);
            addrToSendBuf = new Buffer(addrToSend, "binary");
            addrToSendBuf = encryptor.encrypt(addrToSendBuf);
            //console.log("[10]send " + addrToSendBuf.length + " data to server" + ", stage: " + stage);
            ws.send(addrToSendBuf, {
              binary: true
            });
            i = 0;
            while (i < cachedPieces.length) {
              piece = cachedPieces[i];
              piece = encryptor.encrypt(piece);
              //console.log("[11]send " + piece.length + " data to server" + ", stage: " + stage);
              ws.send(piece, {
                binary: true
              });
              i++;
            }
            cachedPieces = null;
            stage = 5;
            ping = setInterval(function() {
              return ws.ping("", null, true);
            }, 50 * 1000);
            ws._socket.on("drain", function() {
              //console.log("[24]resume local");
              return connection.resume();
            });
          });
          ws.on("message", function(data, flags) {
            //console.log("[12]recv " + data.length + " data from server" + ", stage: " + stage);
            data = encryptor.decrypt(data);
            //console.log("[13]send " + data.length + " data to local" + ", stage: " + stage);
            if (!connection.write(data)) {
              //console.log("[14]pause ws");
              //return ws._socket.pause();
              return;
            }
          });
          ws.on("close", function() {
            clearInterval(ping);
            //console.log("[15]remote disconnected" + ", stage: " + stage);
            return connection.destroy();
          });
          ws.on("error", function(e) {
            console.warn("[16]remote " + remoteAddr + ":" + remotePort + " error: " + e + ", stage: " + stage);
            connection.destroy();
            return server.getConnections(function(err, count) {
              console.log("[17]concurrent connections:" + count + ", stage: " + stage);
            });
          });
          if (data.length > headerLength) {
            buf = new Buffer(data.length - headerLength);
            data.copy(buf, 0, headerLength);
            cachedPieces.push(buf);
            buf = null;
          }
          return stage = 4;
        } catch (_error) {
          e = _error;
          console.warn("[18]" + e + ", stage: " + stage);
          return connection.destroy();
        }
      } else {
        if (stage === 4) {
          return cachedPieces.push(data);
        }
      }
    });
    connection.on("end", function() {
      //console.log("[19]local disconnected" + ", stage: " + stage);
      if (ws) {
        ws.terminate();
      }
      //return server.getConnections(function(err, count) {
      //  console.log("[20]concurrent connections:" + count + ", stage: " + stage);
      //});
      return;
    });
    connection.on("error", function(e) {
      //console.log("[21]local error: " + e + ", stage: " + stage);
      if (ws) {
        ws.terminate();
      }
      return;
      //return server.getConnections(function(err, count) {
        //console.log("concurrent connections:" + count + ", stage: " + stage);
      //});
    });
    connection.on("drain", function() {
      if (ws && ws._socket) {
        //console.log("[25]resume ws");
        //return ws._socket.resume();
        return;
      }
    });
    return connection.setTimeout(timeout, function() {
      //console.log("[22]local timeout" + ", stage: " + stage);
      connection.destroy();
      if (ws) {
        return ws.terminate();
      }
    });
  });

  server.listen(PORT, LOCAL_ADDRESS, function() {
    var address;
    address = server.address();
    return console.log("server listening at #" + address.address + ":" + address.port + "#");
  });

  server.on("error", function(e) {
    if (e.code === "EADDRINUSE") {
      console.warn("address in use, aborting");
    }
    return process.exit(1);
  });

}).call(this);
