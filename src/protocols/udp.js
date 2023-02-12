const dgram = require('dgram');
const client = dgram.createSocket('udp4');
const readline = require('readline');
const { setTimeout } = require('timers/promises');

const startTime = new Date();


module.exports = async (host, port, timeout, threads, packet, broadcast) => {

    let packetSend = 0;

    client.bind(function () {
        client.setBroadcast(broadcast);
    });

    if (broadcast) host = '255.255.255.255';


    while (true) {
        let nowTime = new Date();
        let message = Buffer.alloc(Number(packet)); // If the Packet payload limit exceed, the network layer will fragment the data.

        if (timeout && nowTime.getTime() >= (startTime.getTime() + (Number(timeout) * 1000))) {
            console.log('\nstop all process.');
            client.close();
            process.exit();
        }

        await setTimeout(0);
        for (let i = 0; i < Number(threads); i++) {
            client.send(message, 0, message.length, port, host, async function (err, bytes) {
                if (err) throw err;

                consoleOutputEditor(packetSend++);
            });
        }
    }
}


function consoleOutputEditor(packetSend) {
    //readline.clearLine(process.stdout);
    readline.cursorTo(process.stdout, 0);
    process.stdout.write(`Sent UDP Packet: ${packetSend}`);
}