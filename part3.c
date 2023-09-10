#include <netinet/in.h>
#include <errno.h>
#include <netdb.h>
#include <stdio.h>   //For standard things
#include <stdlib.h>  //malloc
#include <string.h>  //strlen
#include <netinet/udp.h>  //Provides declarations for udp header
#include <netinet/tcp.h>  //Provides declarations for tcp header
#include <netinet/ip.h>   //Provides declarations for ip header
#include <netinet/if_ether.h>  //For ETH_P_ALL
#include <net/ethernet.h>  //For ether_header
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h> // For opening /proc/net/tcp
#include <time.h>  // For measuring time

void ProcessPacket(unsigned char*, int);
void print_ip_header(unsigned char*, int);
void print_tcp_packet(unsigned char*, int);
void print_udp_packet(unsigned char*, int);
void map_ports_to_pid();

FILE* logfile;
struct sockaddr_in source, dest;
int tcp = 0, udp = 0, icmp = 0, others = 0, igmp = 0, total = 0, i, j;

int main()
{
    int saddr_size, data_size;
    struct sockaddr saddr;

    unsigned char* buffer = (unsigned char*)malloc(65536); // It's Big!

    logfile = fopen("log.txt", "w");
    if (logfile == NULL)
    {
        printf("Unable to create log.txt file.");
        return 1;
    }
    printf("Starting...\n");

    int sock_raw = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));

    if (sock_raw < 0)
    {
        // Print the error with a proper message
        perror("Socket Error");
        return 1;
    }

    // Set the start time
    time_t start_time = time(NULL);
    time_t duration = 30; // 30 seconds

    while (1)
    {
        saddr_size = sizeof saddr;
        // Receive a packet
        data_size = recvfrom(sock_raw, buffer, 65536, 0, &saddr, (socklen_t*)&saddr_size);
        if (data_size < 0)
        {
            printf("Recvfrom error, failed to get packets\n");
            return 1;
        }
        // Now process the packet
        ProcessPacket(buffer, data_size);

        // Check if the duration has elapsed
        time_t current_time = time(NULL);
        if (current_time - start_time >= duration)
        {
            break; // Exit the loop after 30 seconds
        }
    }

    close(sock_raw);
    printf("Finished");
    return 0;
}

void ProcessPacket(unsigned char* buffer, int size)
{
    // Get the IP Header part of this packet, excluding the ethernet header
    struct iphdr* iph = (struct iphdr*)(buffer + sizeof(struct ethhdr));
    ++total;
    switch (iph->protocol) // Check the Protocol and do accordingly...
    {
    case 6: // TCP Protocol
        ++tcp;
        print_tcp_packet(buffer, size);
        break;

    case 17: // UDP Protocol
        ++udp;
        print_udp_packet(buffer, size);
        break;

    default: // Some Other Protocol like ARP etc.
        ++others;
        break;
    }
    printf("TCP: %d   UDP: %d    Others: %d   Total: %d\r", tcp, udp, others, total);
}

void print_ip_header(unsigned char* Buffer, int Size)
{
    // print_ethernet_header(Buffer , Size);

    unsigned short iphdrlen;

    struct iphdr* iph = (struct iphdr*)(Buffer + sizeof(struct ethhdr));
    iphdrlen = iph->ihl * 4;

    memset(&source, 0, sizeof(source));
    source.sin_addr.s_addr = iph->saddr;

    memset(&dest, 0, sizeof(dest));
    dest.sin_addr.s_addr = iph->daddr;

    fprintf(logfile, "\n");
    fprintf(logfile, "IP Header\n");
    fprintf(logfile, "   |-Source IP        : %s\n", inet_ntoa(source.sin_addr));
    fprintf(logfile, "   |-Destination IP   : %s\n", inet_ntoa(dest.sin_addr));
}

void print_tcp_packet(unsigned char* Buffer, int Size)
{
    unsigned short iphdrlen;

    struct iphdr* iph = (struct iphdr*)(Buffer + sizeof(struct ethhdr));
    iphdrlen = iph->ihl * 4;

    struct tcphdr* tcph = (struct tcphdr*)(Buffer + iphdrlen + sizeof(struct ethhdr));

    int header_size = sizeof(struct ethhdr) + iphdrlen + tcph->doff * 4;

    fprintf(logfile, "\n\n***********************TCP Packet*************************\n");

    print_ip_header(Buffer, Size);

    fprintf(logfile, "\n");
    fprintf(logfile, "TCP Header\n");
    fprintf(logfile, "   |-Source Port      : %u\n", ntohs(tcph->source));
    fprintf(logfile, "   |-Destination Port : %u\n", ntohs(tcph->dest));

    fprintf(logfile, "\n###########################################################");

    // Map port number to process ID
    map_ports_to_pid();
}

void print_udp_packet(unsigned char* Buffer, int Size)
{

    unsigned short iphdrlen;

    struct iphdr* iph = (struct iphdr*)(Buffer + sizeof(struct ethhdr));
    iphdrlen = iph->ihl * 4;

    struct udphdr* udph = (struct udphdr*)(Buffer + iphdrlen + sizeof(struct ethhdr));

    int header_size = sizeof(struct ethhdr) + iphdrlen + sizeof udph;

    fprintf(logfile, "\n\n***********************UDP Packet*************************\n");

    print_ip_header(Buffer, Size);

    fprintf(logfile, "\nUDP Header\n");
    fprintf(logfile, "   |-Source Port      : %d\n", ntohs(udph->source));
    fprintf(logfile, "   |-Destination Port : %d\n", ntohs(udph->dest));

    fprintf(logfile, "\n###########################################################");
}

void map_ports_to_pid()
{
    // Open the /proc/net/tcp file to read the mapping of ports to process IDs
    FILE* fp = fopen("/proc/net/tcp", "r");
    if (fp == NULL)
    {
        perror("Unable to open /proc/net/tcp");
        return;
    }

    char line[256];
    fgets(line, sizeof(line), fp); // Skip the header line

    while (fgets(line, sizeof(line), fp))
    {
        unsigned int local_port;
        unsigned int inode;
        int ret = sscanf(line, "%*d: %*64s:%x %*64s:%*x %*X %*X:%*X %*X:%*X %*X %*d %*d %*d %*x %u", &local_port, &inode);
        if (ret == 2)
        {
            fprintf(logfile, "\nMapping - Local Port: %u, Process ID: %u", local_port, inode);
        }
    }

    fclose(fp);
}