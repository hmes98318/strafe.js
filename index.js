#!/usr/bin/env node
'use strict';

const { program } = require('commander');
const sender = require('./src/sender');
const readline = require('readline')
    .createInterface({
        input: process.stdin,
        output: process.stdout,
    });




program
    .helpOption('--help')
    .requiredOption('-h, --host <string>', 'Host name')
    .option('-a, --protocols [string]', 'Transfer Protocol', 'udp') // default UDP Protocol
    .option('-p, --port [string]', 'Port numbers', '80') // default port 80
    .option('-t, --timeout [string]', 'Usage time(s)', '60') // default 60s
    .option('-T, --threads [string]', 'Number of Threads', '10') // default 10 Threads
    .option('-S, --packet [string]', 'Set packet payload size(bytes)', '0') // default 0 Bytes, maximum 1472 Bytes
    .option('-B, --broadcast', 'Set Broadcast Address (UDP)', false) // default turn off Broadcast Address
    .version('0.0.1', '-v, --version')

program.showHelpAfterError('add --help for additional information');
program.parse();


const options = program.opts();




readline.question('CAUTION: Press Y to start the process. [Y/N] ', yesNo => {
    readline.close();

    if (yesNo.toUpperCase() == 'Y') {
        console.log('start process.');
        //console.log(options.protocols, options.host, options.port, options.timeout, options.threads, options.packet, options.broadcast)
        sender(options.protocols, options.host, options.port, options.timeout, options.threads, options.packet, options.broadcast);
    }
    else {
        console.log('stop process.');
        process.exit();
    }
});