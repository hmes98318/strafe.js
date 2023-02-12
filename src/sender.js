module.exports = function (protocols, host, port, timeout, threads, packet, broadcast) {

    const exec = require(`./protocols/${protocols.toLowerCase()}`);
    exec(host, port, timeout, threads, packet, broadcast);
}