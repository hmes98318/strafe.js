const dgram = require('dgram');
const readline = require('readline');
const { setTimeout } = require('timers/promises');

const client = dgram.createSocket('udp4');
const startTime = new Date();


module.exports = async (host, port, broadcast, timeout, threads, content) => {

    let packetSend = 0;

    if (broadcast) {
        host = '255.255.255.255';
        client.bind(() => {
            client.setBroadcast(broadcast);
        });
    }


    while (true) {
        let nowTime = new Date();
        let message = Buffer.from(content);

        if (timeout && nowTime.getTime() >= (startTime.getTime() + (Number(timeout) * 1000))) {
            console.log('\nstop all process.');
            client.close();
            process.exit(0);
        }

        await setTimeout(0);
        for (let i = 0; i < Number(threads); i++) {
            client.send(message, 0, message.length, port, host, (err, bytes) => {
                if (err) throw err;

                consoleOutputEditor(packetSend++);
            });
        }
    }
}


const consoleOutputEditor = (packetSend) => {
    //readline.clearLine(process.stdout);
    readline.cursorTo(process.stdout, 0);
    process.stdout.write(`Sent UDP Packet: ${packetSend}`);
}