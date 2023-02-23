const util = require('util');
const readline = require('readline');
const { Worker } = require('worker_threads');

const pre_exec = util.promisify(require('child_process').exec);
const { setTimeout } = require('timers/promises');

let packetSend = 0;


module.exports = async (host, timeout, threads, packetSize) => {
    /*
     * -q: hide output
     * -p: packet fill content
     * -s: packet size (bytes)
     * -w: deadline (seconds)
     * -c: stop after <count> replies
    */
    process.stdout.write('pretest...');
    await pre_exec(`ping ${host} -q -p 0 -s ${packetSize} -w 5 -c 1`, async (error, stdout, stderr) => {
        if (error) {
            //console.log(error);
            console.log('\ntarget does not exist.');
            console.log('stop all process.');
            process.exit(1);
        }


        const workers = [];
        const threadData = { host, packetSize };

        for (let i = 0; i < Number(threads); i++) {
            workers.push(new Worker(`${__dirname}/icmpWorker.js`, { workerData: threadData }));
        }

        workers.forEach(worker => {
            worker.on('message', packetCount => { packetSend += packetCount });
        });

        consoleOutputEditor();

        await setTimeout(Number(timeout) * 1000).then(() => {
            console.log('\nstop all process.');
            workers.forEach(worker => worker.postMessage('stop'));
            process.exit(0);
        });
    });
}


const consoleOutputEditor = () => {
    setInterval(() => {
        readline.cursorTo(process.stdout, 0);
        process.stdout.write(`Sent ICMP Packet: ${packetSend}`);
    }, 0)
};