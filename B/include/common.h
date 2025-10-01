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


#define MAX_PACKETS 10000

extern pcap_if_t *alldevsp;
extern char errbuf[PCAP_ERRBUF_SIZE+1];
extern char *selected_name;
extern int idx;
extern int shark_pgid;
extern int packet_counter;

void ctrl_d();
void ctrl_d2();
void init_cshark();

#endif