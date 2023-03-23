const flood = require('bindings')('syn');


module.exports = (host, port, timeout, delay, threads) => {
    flood.sendPacket(host, port, timeout, delay, threads);
};