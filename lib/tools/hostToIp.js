const dns = require('dns');


const hostToIp = (hostname) => {
    return new Promise((resolve, reject) => {
        dns.lookup(hostname, { family: 4 }, (err, address) => {
            if (err) {
                reject(err);
            }
            else {
                if (address.includes(':')) {
                    resolve(false);
                }
                else {
                    resolve(address);
                }
            }
        });
    });
}

module.exports = hostToIp;