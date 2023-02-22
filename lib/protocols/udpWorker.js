const dgram = require('dgram');
const { parentPort, workerData } = require('worker_threads');
const { setTimeout } = require('timers/promises');

const client = dgram.createSocket('udp4');

const { host, port, packetSize } = workerData;
let message = Buffer.alloc(Number(packetSize)); // bytes


const setupSocket = (broadcast) => {
    if (broadcast) {
        client.bind(() => {
            client.setBroadcast(broadcast);
        });
    }
};

const sendUDP = async () => {
    await setTimeout(0); // set send speed
    client.send(message, 0, message.length, port, host, async (err, bytes) => {
        if (err) throw err;

        parentPort.postMessage(1);
        await sendUDP();
    });
};


parentPort.on('message', message => {
    if (message === 'stop') {
        client.close();
        process.exit(0);
    }
});


setupSocket(workerData.broadcast);
sendUDP();