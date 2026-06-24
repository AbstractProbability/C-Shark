#include "./dialog.h"

char errbuf[PCAP_ERRBUF_SIZE+1];
pcap_if_t *alldevsp = NULL;
char *selected_name = NULL;
int idx = 0;
int shark_pgid = 0;
int packet_counter = 0;

void
ctrl_d() {
    if (feof(stdin)) kill(-shark_pgid, SIGQUIT);
}

void
ctrl_d2() {
    kill(-shark_pgid, SIGQUIT);
}

// LLM GENERATED CODE BEGIN
void
ctrl_c() {
    printf("\nCapture stopped. Returning to main menu...\n");
    if (selected != NULL) {
        pcap_breakloop(selected);
    }
}
// LLM GENERATED CODE END

void
init_cshark() {
    signal(SIGINT, SIG_IGN);
    setpgid(0, 0);
    shark_pgid = getpgrp();
}

int
main() {
    init_cshark();
    list_devices();
    ask_device();
    main_menu();
}