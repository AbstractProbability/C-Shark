#include "../include/control.h"

/*TODO:
UPDATE CAPTURE_CALLBACK*/

int full_print = 0;

/*----------------------------------Index--------------------------------------*/
// caller
void pass_control(int num);

// info printers
void l7_info(const u_char *payload, int payload_len);
void l4_info(int l4_protocol, const u_char *l4_packet, int transport_len);
void l3_info(int l3_protocol, const u_char *l3_packet);
void l2_info(int linktype, const u_char *l2_packet);
void capture_callback(
    u_char *linktype_ptr,
    const struct pcap_pkthdr* pkthdr, 
    const u_char *packet
);

// handlers
void capture_all();
void apply_filter();
void capture_filter(const char *filter_expression);
void last_session();
// pcap_t *selected;

// extras
// LLM Generated Code BEGIN
void start_new_session();
void analyze_packet_in_depth(struct CapturedPacket *packet_to_analyze);

pcap_t *selected = NULL;

struct CapturedPacket g_packet_storage[MAX_PACKETS];
int g_packet_count = 0;
int g_session_linktype = 0; // To remember the linktype for the whole session
// LLM Generated Code END

/*-----------------------------------------------------------------------------*/
void
pass_control(int num)
{
    int pid = fork();
    if (pid == 0)
    {
        signal(SIGINT, SIG_DFL);
        if (num == 1) {
            start_new_session();
            capture_all();
        } else if (num == 2) {
            start_new_session();
            apply_filter();
        } else if (num == 3) {
            last_session();
        } else if (num == 4) {
            // fclose(stdin);
            ctrl_d2();
        }
    }
    else
    {
        wait(NULL);
    }
}

// LLM GENERATED CODE BEGIN
// Forward declaration for the analysis function


// Frees all previously stored packets and resets the counter.
void
start_new_session()
{
    printf("Clearing previous session data...\n");
    for (int i = 0; i < g_packet_count; i++) {
        free(g_packet_storage[i].data); // Free the copied packet data
        g_packet_storage[i].data = NULL;
    }
    g_packet_count = 0;
}

void
analyze_packet_in_depth(struct CapturedPacket *packet_to_analyze)
{
    printf("\n--------------------------------------------------------------\n");
    printf("In-Depth Analysis for Packet ID: %ld\n", (long)(packet_to_analyze - g_packet_storage));
    printf("----------------------------------------------------------------\n");

    // Reuse your existing parsing pipeline, passing the stored linktype and data
    full_print = 1;
    l2_info(g_session_linktype, packet_to_analyze->data);

    printf("\n--- Full Packet Hex Dump ---\n");
    l7_info(packet_to_analyze->data, packet_to_analyze->header.caplen);
    full_print = 0;
    
    printf("----------------------------------------------------------------\n");
}

void
last_session()
{
    if (g_packet_count == 0) {
        printf("\nError: No sniffing session has been run yet, or no packets were captured.\n");
        return;
    }

    printf("\n--- Stored Session Summary ---\n");
    for (int i = 0; i < g_packet_count; i++) {
        printf("  Packet ID: %-5d | Timestamp: %-12ld | Length: %d bytes\n",
               i,
               g_packet_storage[i].header.ts.tv_sec,
               g_packet_storage[i].header.len);
    }
    printf("--------------------------------\n");

    int selected_id = -1;
    printf("Enter Packet ID to inspect: ");
    scanf("%d", &selected_id);
    ctrl_d();

    if (selected_id >= 0 && selected_id < g_packet_count) {
        analyze_packet_in_depth(&g_packet_storage[selected_id]);
    } else {
        printf("Error: Invalid Packet ID.\n");
    }
}
// LLM GENERATED CODE END
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/*L7 STUFF*/
// LLM Generated Code BEGIN
// #include <ctype.h> // For isprint()

// This helper function prints the payload in the required "hex dump" format
void
l7_info(const u_char *payload, int payload_len)
{
    printf("\nL7 (Payload):\n");
    printf("    Payload Length: %d bytes\n", payload_len);

    if (payload_len <= 0) {
        return;
    }

    int bytes_per_line = 16, bytes_to_print;
    if (full_print) {
        printf("    Payload:\n");
        bytes_to_print = payload_len;
    } else {
        printf("    Payload (first 64 bytes):\n");
        bytes_to_print = (payload_len < 64) ? payload_len : 64;
    }
    
    for (int i = 0; i < bytes_to_print; i += bytes_per_line) {
        // Print hex offset
        printf("        %04x: ", i);

        // Print hex values for this line
        for (int j = 0; j < bytes_per_line; j++) {
            if (i + j < bytes_to_print) {
                printf("%02x ", payload[i + j]);
            } else {
                printf("   "); // Pad for alignment
            }
        }
        printf(" ");

        // Print ASCII characters for this line
        for (int j = 0; j < bytes_per_line; j++) {
            if (i + j < bytes_to_print) {
                // Use isprint() to check for printable characters
                printf("%c", isprint(payload[i + j]) ? payload[i + j] : '.');
            }
        }
        printf("\n");
    }
}
// LLM Generated Code END
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/*L4 STUFF*/
// LLM Generated code begin

// Helper function to identify L7 services based on port number
const char*
get_service_name(uint16_t port)
{
    switch (port)
    {
        case 80:   return "HTTP";
        case 443:  return "HTTPS";
        case 53:   return "DNS";
        case 20:
        case 21:   return "FTP";
        case 22:   return "SSH";
        case 25:   return "SMTP";
        case 110:  return "POP3";
        default:   return "";
    }
}

// Parses and prints TCP header details
// return header_len.
int
handle_tcp(const u_char *l4_packet)
{
    tcp_hdr *tcphdr = (tcp_hdr *)l4_packet;

    uint16_t source_port = ntohs(tcphdr->source);
    uint16_t dest_port = ntohs(tcphdr->dest);
    uint32_t seq_num = ntohl(tcphdr->seq);
    uint32_t ack_num = ntohl(tcphdr->ack_seq);
    uint16_t window_size = ntohs(tcphdr->window);
    uint16_t checksum = ntohs(tcphdr->check);
    // Header length is in 4-byte words, so multiply by 4
    uint8_t header_len = tcphdr->doff * 4;

    printf("TCP\n");
    printf("    Source Port:        %u (%s)\n",
                source_port, get_service_name(source_port));
    printf("    Destination Port:   %u (%s)\n",
                dest_port, get_service_name(dest_port));
    printf("    Sequence Number:    %u\n", seq_num);
    printf("    Acknowledgment Num: %u\n", ack_num);
    printf("    Header Length:      %u bytes\n", header_len);
    printf("    Window Size:        %u\n", window_size);
    printf("    Checksum:           0x%04x\n", checksum);
    
    // Decode the TCP flags
    printf("    Flags:\n");
    printf("        SYN: %d\n", tcphdr->syn);
    printf("        ACK: %d\n", tcphdr->ack);
    printf("        FIN: %d\n", tcphdr->fin);
    printf("        RST: %d\n", tcphdr->rst);
    printf("        PSH: %d\n", tcphdr->psh);
    printf("        URG: %d\n", tcphdr->urg);

    // TCP header length
    int tcp_header_len = tcphdr->doff * 4;
    return tcp_header_len;
}

// Parses and prints UDP header details
// return header_len
int
handle_udp(const u_char *l4_packet)
{
    udp_hdr *udphdr = (udp_hdr *)l4_packet;

    uint16_t source_port = ntohs(udphdr->source);
    uint16_t dest_port = ntohs(udphdr->dest);
    uint16_t length = ntohs(udphdr->len);
    uint16_t checksum = ntohs(udphdr->check);

    printf("UDP\n");
    printf("    Source Port:      %u (%s)\n",
                source_port, get_service_name(source_port));
    printf("    Destination Port: %u (%s)\n",
                dest_port, get_service_name(dest_port));
    printf("    Length:           %u bytes\n", length);
    printf("    Checksum:         0x%04x\n", checksum);

    // UDP header length is a fixed 8 bytes
    int udp_header_len = 8;
    return udp_header_len;
}

// The main L4 dispatcher function
/*calculate payload_len directly here, using l4_packet_len
  and returned l4_header_len */
void
l4_info(int l4_protocol, const u_char *l4_packet, int l4_packet_len)
{
    int l4_header_len = 0;
    int l7_packet_len = 0;

    printf("\nL4:\n");
    printf("    (L4) Type: ");

    if (l4_protocol == IPPROTO_TCP) {
        l4_header_len = handle_tcp(l4_packet);
    } else if (l4_protocol == IPPROTO_UDP) {
        l4_header_len = handle_udp(l4_packet);
    } else {
        printf("Other (%d)\n", l4_protocol);
        return;
    }
    l7_packet_len = l4_packet_len - l4_header_len;
    const u_char *l7_packet = l4_packet + l4_header_len;
    
    l7_info(l7_packet, l7_packet_len);
}
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/*L3 STUFF*/

uint8_t
handle_ipv4(const u_char *l3_packet, int *payload_offset)
{
    ipv4_hdr *ip_hdr = (ipv4_hdr *)l3_packet;
    char source_ip[INET_ADDRSTRLEN];
    char dest_ip[INET_ADDRSTRLEN];

    // Convert binary IP addresses to human-readable strings
    inet_ntop(AF_INET, &(ip_hdr->saddr), source_ip, INET_ADDRSTRLEN);
    inet_ntop(AF_INET, &(ip_hdr->daddr), dest_ip, INET_ADDRSTRLEN);

    // Calculate the header length in bytes (ihl is in 4-byte words)
    *payload_offset = ip_hdr->ihl * 4;

    printf("IPv4\n");
    printf("    Source IP:      %s\n", source_ip);
    printf("    Destination IP: %s\n", dest_ip);
    printf("    Header Length:  %u bytes\n", *payload_offset);
    printf("    Total Length:   %u bytes\n", ntohs(ip_hdr->tot_len));
    printf("    TTL:            %u\n", ip_hdr->ttl);
    printf("    Packet ID:      %u\n", ntohs(ip_hdr->id));
    
    // Decode flags from the fragment offset field
    uint16_t frag_off = ntohs(ip_hdr->frag_off);
    printf("    Flags:\n");
    printf("        Don't Fragment (DF): %s\n", (frag_off & IP_DF) ? "Set" : "Not set");
    printf("        More Fragments (MF): %s\n", (frag_off & IP_MF) ? "Set" : "Not set");
    
    return ip_hdr->protocol;
}

uint8_t
handle_ipv6(const u_char *l3_packet, int *payload_offset)
{
    ipv6_hdr *ip6_hdr = (ipv6_hdr *)l3_packet;
    char source_ip[INET6_ADDRSTRLEN];
    char dest_ip[INET6_ADDRSTRLEN];

    // The IPv6 header is a fixed 40 bytes
    *payload_offset = 40;

    // Convert binary IPv6 addresses to human-readable strings
    inet_ntop(AF_INET6, &(ip6_hdr->ip6_src), source_ip, INET6_ADDRSTRLEN);
    inet_ntop(AF_INET6, &(ip6_hdr->ip6_dst), dest_ip, INET6_ADDRSTRLEN);

    // Extract Traffic Class and Flow Label from the version/class/flow field
    uint32_t flow_val = ntohl(ip6_hdr->ip6_flow);
    uint8_t traffic_class = (flow_val >> 20) & 0xFF;
    uint32_t flow_label = flow_val & 0xFFFFF;

    printf("IPv6\n");
    printf("    Source IP:        %s\n", source_ip);
    printf("    Destination IP:   %s\n", dest_ip);
    printf("    Payload Length:   %u bytes\n", ntohs(ip6_hdr->ip6_plen));
    printf("    Hop Limit:        %u\n", ip6_hdr->ip6_hlim); // 1-byte field
    printf("    Traffic Class:    0x%02x\n", traffic_class);
    printf("    Flow Label:       0x%05x\n", flow_label);

    return ip6_hdr->ip6_nxt;
}

uint8_t
handle_arp(const u_char *l3_packet)
{
    arp_hdr *arp_packet = (arp_hdr *)l3_packet;
    // The addresses are stored immediately after the fixed-size arphdr
    const struct arp_payload *arp_data = 
    (const struct arp_payload *)(l3_packet + sizeof(arp_hdr));

    char sender_ip_str[INET_ADDRSTRLEN];
    char target_ip_str[INET_ADDRSTRLEN];
    
    // Note: ARP stores IPs in network byte order, so they are ready for inet_ntop
    inet_ntop(AF_INET, &(arp_data->sender_ip), sender_ip_str, INET_ADDRSTRLEN);
    inet_ntop(AF_INET, &(arp_data->target_ip), target_ip_str, INET_ADDRSTRLEN);

    printf("ARP\n");
    printf("    Operation:        %s (%d)\n",
           (ntohs(arp_packet->ar_op) == ARPOP_REQUEST) ? "Request" :
           (ntohs(arp_packet->ar_op) == ARPOP_REPLY) ? "Reply" : "Other",
           ntohs(arp_packet->ar_op));

    printf("    Hardware Type:    %u\n", ntohs(arp_packet->ar_hrd));
    printf("    Protocol Type:    0x%04x\n", ntohs(arp_packet->ar_pro));
    printf("    Hardware Length:  %u\n", arp_packet->ar_hln);
    printf("    Protocol Length:  %u\n", arp_packet->ar_pln);
    
    printf("    Sender MAC:       %s\n", ether_ntoa((const struct ether_addr *)arp_data->sender_mac));
    printf("    Sender IP:        %s\n", sender_ip_str);
    printf("    Target MAC:       %s\n", ether_ntoa((const struct ether_addr *)arp_data->target_mac));
    printf("    Target IP:        %s\n", target_ip_str);

    return 0; // ARP doesn't have a Layer 4 protocol
}

void
print_l4_protocol(uint8_t protocol)
{
    printf("    (L4) Protocol: ");
    if (protocol == IPPROTO_TCP) {
        printf("TCP\n");
    } else if (protocol == IPPROTO_UDP) {
        printf("UDP\n");
    } else {
        printf("Other (%u)\n", protocol);
    }
}

void
l3_info(int l3_protocol, const u_char *l3_packet)
{
    uint8_t l4_protocol = 0;
    int l3_header_len = 0; // this is payload_offset
    int l4_packet_len = 0; // l4 header + l7 payload length i.e. l4_packet_len

    printf("\nL3:\n");
    printf("    (L3) Type: ");

    if (l3_protocol == ETHERTYPE_IP)
    {
        ipv4_hdr *ipv4hdr = (ipv4_hdr *) l3_packet;
        l4_protocol = handle_ipv4(l3_packet, &l3_header_len);
        l4_packet_len = ntohs(ipv4hdr->tot_len) - l3_header_len;
    }
    else if (l3_protocol == ETHERTYPE_ARP)
    {
        l4_protocol = handle_arp(l3_packet);
        return;
    }
    else if (l3_protocol == ETHERTYPE_IPV6)
    {
        ipv6_hdr *ipv6hdr = (ipv6_hdr *) l3_packet;
        l4_protocol = handle_ipv6(l3_packet, &l3_header_len);
        l4_packet_len = ntohs(ipv6hdr->ip6_plen);
    }
    else
    {
        printf("Unknown L3\n");
        return;
    }

    print_l4_protocol(l4_protocol);
    const u_char *l4_packet = l3_packet + l3_header_len;
    l4_info(l4_protocol, l4_packet, l4_packet_len);
}
// LLM GENERATED CODE END
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/*L2 STUFF*/
int
handle_lo(const u_char *l2_packet)
{
    printf("loopback\n");
    lo_header *lo_ptr = (lo_header *) l2_packet;

    printf("    Source MAC (logically):      device mac");
    printf("    Destination MAC (logically): device mac");

    if (ntohl(lo_ptr->protocol_type) == AF_INET) {
        return ETHERTYPE_IP;
    } else if (ntohl(lo_ptr->protocol_type) == AF_INET6) {
        return ETHERTYPE_IPV6;
    }

    return 0;
}

int
handle_any(const u_char *l2_packet)
{
    printf("any\n");
    any_header *any_ptr = (any_header *) l2_packet;

    printf("    Source MAC:      %s\n", 
                ether_ntoa((struct ether_addr *)&any_ptr->ll_address));
    printf("    Destination MAC: -\n");

    return ntohs(any_ptr->protocol_type);
}

int
handle_eth(const u_char *l2_packet)
{
    printf("Ethernet\n");
    ether_header *eth_ptr = (ether_header *) l2_packet;

    printf("    Source MAC:      %s\n",
                ether_ntoa((struct ether_addr *)&eth_ptr->ether_shost));
    printf("    Destination MAC: %s\n", 
                ether_ntoa((struct ether_addr *)&eth_ptr->ether_dhost));
    
    return ntohs(eth_ptr->ether_type);
}

void
print_ethertype(int l3_protocol)
{
    printf("    (L3) EtherType:  ");
    if (l3_protocol == ETHERTYPE_IP) {
        printf("IPv4\n");
    } else if (l3_protocol == ETHERTYPE_ARP) {
        printf("ARP\n");
    } else if (l3_protocol == ETHERTYPE_IPV6) {
        printf("IPv6\n");
    } else {
        printf("Unknown L3\n");
    }
    printf("\n");
}

void
l2_info(int linktype, const u_char *l2_packet)
{
    int l3_protocol = 0;
    int l2_header_len = 0; // i.e. l2_header_len
    printf("\nL2:\n");

    printf("    (L2) Type:       ");
    if (linktype == DLT_NULL)
    {
        l3_protocol = handle_lo(l2_packet);
        l2_header_len = LO_HDR_LEN;
    }
    else if (linktype == DLT_LINUX_SLL)
    {
        l3_protocol = handle_any(l2_packet);
        l2_header_len = ANY_HDR_LEN;
    }
    else if (linktype == DLT_EN10MB)
    {
        l3_protocol = handle_eth(l2_packet);
        l2_header_len = ETH_HDR_LEN;
    }
    else
    {
        printf("Unknown\n");
        return;
    }

    print_ethertype(l3_protocol);
    const u_char *l3_packet = l2_packet + l2_header_len;
    l3_info(l3_protocol, l3_packet);
}
/*-----------------------------------------------------------------------------*/

u_int u_min(u_int a, u_int b)
{
    if (a < b)
    {
        return a;
    }
    return b;
}

void
capture_callback(
    u_char *linktype_ptr,
    const struct pcap_pkthdr* pkthdr, 
    const u_char *l2_packet
)
{
    ctrl_d();
    
    printf("----------------------------------------------------------------\n");
    printf("Packet details:\n\
    Packet Number:   %d\n\
    Timestamp:       %ld,\n\
    Captured_length: %d\n\
    Raw16:           ", packet_counter, pkthdr->ts.tv_sec, pkthdr->caplen);
    
    for (u_int i = 0; i<u_min(pkthdr->caplen, 16); i++) {
        printf("%02x ", l2_packet[i]);
    }
    printf("\n");
    
    int linktype = *(int *)linktype_ptr;
    l2_info(linktype, l2_packet);
    
    printf("----------------------------------------------------------------\n");
    packet_counter++;
    fflush(stdout);

    // LLM Generated code BEGIN
    if (g_packet_count < MAX_PACKETS)
    {
        // 1. Copy the pcap header
        g_packet_storage[g_packet_count].header = *pkthdr;

        // 2. Allocate new memory on the heap for the packet data
        g_packet_storage[g_packet_count].data = malloc(pkthdr->caplen);
        if (g_packet_storage[g_packet_count].data == NULL) {
            fprintf(stderr, "Failed to allocate memory for packet storage.\n");
            return; // Or handle error more gracefully
        }

        // 3. Copy the packet data into the new memory
        memcpy(g_packet_storage[g_packet_count].data, l2_packet, pkthdr->caplen);

        // 4. Increment the stored packet counter
        g_packet_count++;
    }
    // LLM Generated Code END
}

void
capture_all()
{
    selected = pcap_create(selected_name, errbuf);
    if (selected == NULL) {
        printf("pcap_create failed: %s\n", errbuf);
        return;
    }
    pcap_set_snaplen(selected, 65535);
    pcap_set_promisc(selected, 1);
    pcap_set_timeout(selected, 1);
    if (pcap_activate(selected) < 0) {
        printf("pcap_activate failed\n");
        pcap_close(selected);
        return;
    }
    int linktype = pcap_datalink(selected);
    g_session_linktype = linktype; // Store the linktype globally

    // LLM GENERATED CODE BEGIN
    signal(SIGINT, ctrl_c);
    // LLM GENERATED CODE END

    pcap_loop(selected, -1, capture_callback, (u_char *)&linktype);
    pcap_close(selected);
    // LLM GENERATED CODE BEGIN
    signal(SIGINT, SIG_IGN);
    selected = NULL;
    // LLM GENERATED CODE END
}

// LLM Generated Code BEGIN
// This function opens a handle, applies a filter, and starts the capture
void
capture_filter(const char *filter_expression)
{
    struct bpf_program fp; // compiled filter

    // 1. Open the handle (same as capture_all)
    selected = pcap_create(selected_name, errbuf);
    if (selected == NULL) {
        printf("pcap_create failed: %s\n", errbuf);
        return;
    }
    pcap_set_snaplen(selected, 65535);
    pcap_set_promisc(selected, 1);
    pcap_set_timeout(selected, 1);
    if (pcap_activate(selected) < 0) {
        printf("pcap_activate failed\n");
        pcap_close(selected);
        return;
    }

    // 2. Compile the filter string
    if (pcap_compile(selected, &fp, filter_expression, 1, PCAP_NETMASK_UNKNOWN) == -1)
    {
        printf("Couldn't parse filter\n");
        pcap_close(selected);
        return;
    }

    // 3. Apply the compiled filter
    if (pcap_setfilter(selected, &fp) == -1) {
        printf("Couldn't install filter\n");
        pcap_close(selected);
        pcap_freecode(&fp);
        return;
    }

    // 4. Start the capture loop (same as before)
    int linktype = pcap_datalink(selected);
    g_session_linktype = linktype; // Store the linktype globally

    // LLM GENERATED CODE BEGIN
    signal(SIGINT, ctrl_c);
    // LLM GENERATED CODE END

    pcap_loop(selected, -1, capture_callback, (u_char *)&linktype);

    // 5. Clean up
    pcap_freecode(&fp);
    pcap_close(selected);
    // LLM GENERATED CODE BEGIN
    signal(SIGINT, SIG_IGN);
    selected = NULL;
    // LLM GENERATED CODE END
}

void
apply_filter()
{
    int choice;
    char filter[100]; // Buffer for our filter string

    printf("\nSelect a filter:\n");
    printf("  1. HTTP\n");
    printf("  2. HTTPS\n");
    printf("  3. DNS\n");
    printf("  4. ARP\n");
    printf("  5. TCP\n");
    printf("  6. UDP\n");
    printf("Enter choice: ");
    scanf("%d", &choice);

    switch (choice)
    {
        case 1: strcpy(filter, "tcp port 80"); break;
        case 2: strcpy(filter, "tcp port 443"); break;
        case 3: strcpy(filter, "udp port 53"); break;
        case 4: strcpy(filter, "arp"); break;
        case 5: strcpy(filter, "tcp"); break;
        case 6: strcpy(filter, "udp"); break;
        default:
            printf("Invalid choice.\n");
            return;
    }

    printf("Starting capture with filter: \"%s\"\n", filter);
    capture_filter(filter);
}
// LLM Generated Code END
