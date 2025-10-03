#include "../include/dialog.h"

void
list_devices()
{
    printf("|| [C-Shark] ||\n");
    printf("---------------\n\n");
    printf("Available Interfaces:\n");

    int i = pcap_findalldevs(&alldevsp, errbuf);
    if (i == -1) {
        printf("%s\n", errbuf);
        exit(1);
    }
    pcap_if_t *list = alldevsp;
    while (list != NULL) {
        idx++;
        printf("%d. %s\n", idx, list->name);
        list = list->next;
    }
}

void
ask_device()
{
    printf("Select an interface to sniff (1-%d): ", idx);
    int num;
    scanf("%d", &num); ctrl_d();
    while (num > idx || num < 1) {
        printf("Select an interface to sniff (1-%d) only: ", idx);
        scanf("%d", &num);
        ctrl_d();
    }
    num--;

    pcap_if_t *list = alldevsp;
    while(num) {
        list = list->next;
        num--;
    }
    selected_name = malloc(sizeof(char) * (strlen(list->name) + 1));
    strcpy(selected_name, list->name);
    pcap_freealldevs(alldevsp);
}

void
main_menu()
{
    // make sure this doesnt temrinate on ctrl_c
    while(1) {
        printf("Main Menu:\n");
        printf("----------\n");
        printf("Interface '%s' selected.\n", selected_name);
        printf("Select 1-4:\n");
        printf("1. Start Sniffing (All Packets)\n");
        printf("2. Start Sniffing (With Filters)\n");
        printf("3. Inspect Last Session\n");
        printf("4. Exit C-Shark\n");
        
        int num = -1;
        printf("Selection: ");
        //int bytes_read = 
        scanf("%d", &num);
        // error checking is not correct acc to scanf maybe switch to getline?
        ctrl_d();
        if (num > 4 || num < 1) {
            printf("Invalid Input.\n");
        } else {
            pass_control(num);
        }
    }
}