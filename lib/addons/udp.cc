#include <node.h>
#include <nan.h>

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <chrono>
#include <thread>

#define sleep_ms(milsec) usleep(milsec * 1000)

using namespace v8;


int sock; // UDP socket
char dst_ip[20] = {0};
int dst_port;

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

void *send_UDP(void *addr_info, int timeout = 60, int delay = 1000, int packetsize = 10)
{
    struct sockaddr *addr = (struct sockaddr *)addr_info;

    // char *data;
    int count = 0; // Packet sent count

    auto start_time = std::chrono::steady_clock::now();
    srand((unsigned)time(NULL));

    while (true)
    {
        char *data = (char *)generate_hex_array(packetsize);

        if (sendto(sock, data, sizeof(data), 0, addr, sizeof(struct sockaddr)) < 0)
        {
            perror("failed to send data");
            close(sock);
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

    int timeout = args[2]->Int32Value(isolate->GetCurrentContext()).ToChecked();
    int delay = args[3]->Int32Value(isolate->GetCurrentContext()).ToChecked();
    int packetsize = args[4]->Int32Value(isolate->GetCurrentContext()).ToChecked();
    int numThreads = args[5]->Int32Value(isolate->GetCurrentContext()).ToChecked();

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

    // Create UDP socket
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
    {
        perror("socket()");
        exit(1);
    }

    printf("start process.\n");

    // Create threads
    std::thread threads[numThreads];
    for (int i = 0; i < numThreads; i++)
    {
        threads[i] = std::thread(send_UDP, &addr, timeout, delay, packetsize);

        if (threads[i].get_id() == std::thread::id())
        {
            printf("Failed to create thread %d", i);
            exit(1);
        }
        printf("\n");
    }

    for (int i = 0; i < numThreads; i++)
    {
        threads[i].join();
    }

    printf("Stop all process.\n");
    close(sock);
}

void Initialize(Local<Object> exports)
{
    NODE_SET_METHOD(exports, "sendPacket", Method);
}

NODE_MODULE(addon, Initialize)
