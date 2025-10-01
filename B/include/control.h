#ifndef control
#define control

#include "../include/common.h"
#include "../include/session.h"

/* --------------------------------l2 stuff----------------------------------- */
struct any_header {
    short packet_type;
    short ARPHRD_type;
    short ll_address_length;
    char ll_address[8];
    short protocol_type;
}__attribute__((__packed__));

struct lo_header {
    int protocol_type;
}__attribute__((__packed__));

enum l2_header_length {
    ETH_HDR_LEN=14,
    ANY_HDR_LEN=16,
    LO_HDR_LEN=4,
};

typedef struct ether_header ether_header; // payload_off: 14 bytes
typedef struct any_header any_header;     // payload_off: 16 bytes
typedef struct lo_header lo_header;       // payload_off:  4 bytes
typedef enum l2_header_length l2_header_length ; // concise payload_off
/* --------------------------------------------------------------------------- */


/* --------------------------------l3 stuff----------------------------------- */
// LLM GENERATED CODE BEGINS
struct arp_payload {
    uint8_t  sender_mac[6];
    uint32_t sender_ip;
    uint8_t  target_mac[6];
    uint32_t target_ip;
} __attribute__((__packed__));

// control flag masks for ipv4
#ifndef IP_DF
#define IP_DF 0x4000  // Don't fragment flag (in host byte order)
#endif

#ifndef IP_MF
#define IP_MF 0x2000  // More fragments flag (in host byte order)
#endif
// LLM GENERATED CODE ENDS

typedef struct arp_payload arp_payload;

typedef struct iphdr ipv4_hdr;
typedef struct ip6_hdr ipv6_hdr;
typedef struct arphdr arp_hdr;
/* --------------------------------------------------------------------------- */


/* --------------------------------l4 stuff----------------------------------- */
typedef struct tcphdr tcp_hdr;
typedef struct udphdr udp_hdr;
/* --------------------------------------------------------------------------- */

// api
void pass_control(int num);

// entry, exit
void init_cshark();
void ctrl_d();
void ctrl_d2();

#endif