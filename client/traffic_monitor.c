#include "traffic_monitor.h"

void processPackets(unsigned char* buffer,int size,int fd)
{
    struct ethhdr* eth=(struct ethhdr*)(buffer);

    // if(htons(eth->h_proto)==0x806)
    // {
    //     //printARP(buffer,size,logfile);
    //     return;
    // }

    //if(htons(eth->h_proto!=0x800)) return;

    struct iphdr* iph=(struct iphdr*) (buffer+sizeof(struct ethhdr));
    switch(iph->protocol)
    {   //TODO: toate functile de printare mai in detaliu pentru pachete
        case 1:
            //myWrite("ICMP Received",fd);
            write(fd,"ICMP",4);
            break;
        case 2:
            //myWrite("IGMP Received",fd);
            write(fd,"IGMP",4);
            break;
        case 6:
            //myWrite("TCP Received",fd);
            write(fd,"TCP",3);
            break;
        case 17:
            //myWrite("UDP Received",fd);
            write(fd,"UDP",3);
            break;
        default:
            //myWrite("Unkown Received",fd);
            write(fd,"UNKNOWN",7);
            break;
    }

    write(fd,"\n",1);
}