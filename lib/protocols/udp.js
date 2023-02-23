const { Worker } = require('worker_threads');
const readline = require('readline');
const { setTimeout } = require('timers/promises');

let packetSend = 0;


module.exports = async (host, port, broadcast, timeout, threads, packetSize) => {
    if (broadcast) {
        host = '255.255.255.255';
    }

    const workers = [];
    const threadData = { host, port, broadcast, packetSize };

    for (let i = 0; i < Number(threads); i++) {
        workers.push(new Worker(`${__dirname}/udpWorker.js`, { workerData: threadData }));
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

};


const consoleOutputEditor = () => {
    setInterval(() => {
        readline.cursorTo(process.stdout, 0);
        process.stdout.write(`Sent UDP Packet: ${packetSend}`);
    }, 0)
};