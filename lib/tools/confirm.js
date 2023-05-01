const readline = require('readline')
    .createInterface({
        input: process.stdin,
        output: process.stdout,
    });
const cst = require('../constants.js');


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

module.exports = confirm;