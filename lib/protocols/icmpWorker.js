const { parentPort, workerData } = require('worker_threads');
const { exec } = require('child_process');
const { setTimeout } = require('timers/promises');

const { host, packetSize } = workerData;


const sendICMP = async () => {

    await setTimeout(0);
    exec(`ping ${host} -q -p 0 -s ${packetSize} -w 5`, () => {
        parentPort.postMessage(1);
    });
    sendICMP();
}


parentPort.on('message', message => {
    if (message === 'stop') {
        client.close();
        process.exit(0);
    }
});


sendICMP();