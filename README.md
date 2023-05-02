# strafe.js

Strafe.js is a simple Node.js script designed for educational and penetration testing purposes.  
It can be used to conduct flood attacks in various protocols such as UDP, TCP, and ICMP.  

This script can only be run on Linux.  
For Windows users, you will need to install [**WSL**](https://docs.microsoft.com/en-us/windows/wsl/install) and run the script within the WSL environment.  


## Installation
```
$ npm i strafe -g
```

## How to use?
**strafe [protocols] [options] host**  
* strafe udp -p 8080 -t 20 -d 300 -s 50 192.168.1.10
* strafe udp -t 60 domain.com


## To do list
- [x] udp  
- [x] syn
- [ ] icmp (incomplete)
- [ ] http


## Disclaimer
This script is meant for personal learning and penetration testing purposes only.  
It should not be used for illegal or malicious activities.  
Use the script at your own risk and only with prior permission from the target host.  
The author takes no responsibility for any misuse or damage caused by this script.  
BY USING THIS SOFTWARE YOU AGREE WITH THESE TERMS.  

## License
This project is licensed under the GNU General Public License v3.0. See the [LICENSE](LICENSE) file for details.
