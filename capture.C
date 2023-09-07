#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

struct tcp_flow{
    in_addr saddr;
    in_addr daddr;
    unsigned short sport;
    unsigned short dport;
};

struct tcp_flow_node{
    struct tcp_flow flow;
    int packets;
    int bytes;
    struct tcp_flow_node *next;
};

struct tcp_flow_node *head = NULL;

void add_flow(struct tcp_flow *flow, int data_size){
    struct tcp_flow_node *node = head;
    while(node != NULL){
        if (memcmp(&node->flow, flow, sizeof(struct tcp_flow)) == 0){
            node->packets++;
            node->bytes += data_size;
            return;
        }
        node = node->next;
    }

    node =  (struct tcp_flow_node*)malloc(sizeof(struct tcp_flow_node));
    if (node == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    memcpy(&node->flow, flow, sizeof(struct tcp_flow));
    node->packets = 1;
    node->bytes = data_size;
    node->next = head;
    head = node;
}

void print_flows(){
    struct tcp_flow_node *node = head;
    while(node!=NULL){
        printf("Flow from %s:%d to %s:%d has %d packets and %d bytes\n", 
        inet_ntoa(node->flow.saddr), ntohs(node->flow.sport), inet_ntoa(node->flow.daddr), 
        ntohs(node->flow.dport), node->packets, node->bytes);
        node = node->next;
    }
}

void free_flows(){
    struct tcp_flow_node *node = head;
    struct tcp_flow_node *temp;
    while (node != NULL) {
        temp = node->next;
        free(node);
        node = temp;
    }
}

#define BUFSIZE 65536 // maximum packet size
unsigned char buffer[BUFSIZE]; // buffer to store packet data
struct sockaddr_in source, dest; // structures to store source and destination addresses
struct iphdr *iph; // pointer to IP header
struct tcphdr *tcph; // pointer to TCP header
int data_size; // number of bytes received


int main(){
    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    if (sock == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    strcpy(ifr.ifr_name, "wlo1");
    if (ioctl(sock, SIOCGIFFLAGS, &ifr) == -1) {
        perror("ioctl");
        exit(EXIT_FAILURE);
    }
    ifr.ifr_flags |= IFF_PROMISC;
    if (ioctl(sock, SIOCSIFFLAGS, &ifr) == -1) {
        perror("ioctl");
        exit(EXIT_FAILURE);
    }
    
    data_size = recvfrom(sock, buffer, BUFSIZE, 0, NULL, NULL);
    int n=0;
    while(n<50){
        if(data_size!=-1){
            iph = (struct iphdr*) buffer;

            struct tcp_flow flow;
            flow.saddr = *(struct in_addr*)(&(iph->saddr));
            flow.daddr = *(struct in_addr*)(&(iph->daddr));

            tcph = (struct tcphdr*) (buffer + iph->ihl * 4);
            flow.sport = tcph->source;
            flow.dport = tcph->dest;
            add_flow(&flow, data_size);

            source.sin_addr.s_addr = iph->saddr; // get the source IP address
            dest.sin_addr.s_addr = iph->daddr; // get the destination IP address
            printf("Packet from %s:%d to %s:%d\n", 
            inet_ntoa(source.sin_addr), ntohs(tcph->source), 
            inet_ntoa(dest.sin_addr), ntohs(tcph->dest));
        }
        else printf("%d\n", data_size);
        data_size = recvfrom(sock, buffer, BUFSIZE, 0, NULL, NULL);
        n++;
    }

    //print_flows();

    return 0;
}