#ifndef common
#define common

// Usual
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <ctype.h>
#include <sys/wait.h>
#include <sys/types.h>

// General
#include <pcap.h>
#include <arpa/inet.h>
#include <netinet/in.h>         // for tcp, udp identification

// L2
#include <net/ethernet.h>
#include <netinet/ether.h>

// L3
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <net/if_arp.h>
#include <netinet/ip_icmp.h>

// L4
#include <netinet/tcp.h>
#include <netinet/udp.h>

extern pcap_if_t *alldevsp;
extern char errbuf[PCAP_ERRBUF_SIZE+1];
extern char *selected_name;
extern int idx;
extern int shark_pgid;
extern int packet_counter;

// Logging stuff

// LLM Generated Code BEGIN
// A struct to hold a single captured packet's data and its metadata
struct CapturedPacket {
    struct pcap_pkthdr header; // The header from pcap (timestamp, length)
    u_char *data;              // A heap-allocated copy of the packet data
};
// Global storage for the last session
#define MAX_PACKETS 10000
extern struct CapturedPacket g_packet_storage[MAX_PACKETS];
extern int g_packet_count;
extern int g_session_linktype; // To remember the linktype for the whole session
extern pcap_t *selected;
// LLM Generated Code END

// entry, exit
void init_cshark();
void ctrl_d();
void ctrl_d2();
void ctrl_c();
#endif