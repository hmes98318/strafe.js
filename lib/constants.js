const fs = require('fs');
const path = require('path');


/**
 * Constants variables
 */
const cst = {
    SUCCESS_EXIT    : 0,
    ERROR_EXIT      : 1,

    LOGO            : fs.readFileSync(`${__dirname}/txt/logo`, 'utf-8'),

    UDP_HELP        : fs.readFileSync(`${__dirname}/txt/udp-help`, 'utf-8'),
    UDP_USAGE       : fs.readFileSync(`${__dirname}/txt/udp-usage`, 'utf-8'),
    UDP_ERROR_HELP  : `\n------------------------------\n${fs.readFileSync(`${__dirname}/txt/udp-help`, 'utf-8')}`,

    ICMP_HELP       : fs.readFileSync(`${__dirname}/txt/icmp-help`, 'utf-8'),
    ICMP_USAGE      : fs.readFileSync(`${__dirname}/txt/icmp-usage`, 'utf-8'),
    ICMP_ERROR_HELP : `\n------------------------------\n${fs.readFileSync(`${__dirname}/txt/icmp-help`, 'utf-8')}`,
}

module.exports = cst;