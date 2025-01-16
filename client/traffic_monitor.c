#include "traffic_monitor.h"

void printIPs(unsigned char* buffer, int size,int fd)
{   
    struct ethhdr* eth=(struct ethhdr*)(buffer);

    struct iphdr* iph=(struct iphdr*) (buffer+sizeof(struct ethhdr));

    char ipSource[INET_ADDRSTRLEN];
    inet_ntop(AF_INET,&(iph->saddr),ipSource,INET_ADDRSTRLEN);

    char ipDestination[INET_ADDRSTRLEN];
    inet_ntop(AF_INET,&(iph->daddr),ipDestination,INET_ADDRSTRLEN);

    char output[MAXLINE];

    snprintf(output,MAXLINE,"Source IP Address: %s Destination IP Address: %s\r\n",ipSource,ipDestination);
    myWrite(output,fd);
    //fprintf(logfile,"Source IP Address: %s Destination IP Address: %s\r\n",ipSource,ipDestination);
}

void printTCP(unsigned char* buffer, int size,int fd)
{       

    unsigned short iphdrlen;
    struct iphdr* iph=(struct iphdr*) buffer+sizeof(struct ethhdr);
    iphdrlen=iph->ihl*4;

    struct tcphdr* tcph=(struct tcphdr*)(buffer+iphdrlen+sizeof(struct ethhdr));


    // if(isDNS(buffer,size,1))
    // {
    //     printDNS(buffer,size,1,logfile);
    //     return 1;
    // }

    //fprintf(logfile,"TCP RECEIVED\n");

    //printIPs(buffer,size,logfile);

    printIPs(buffer,size,fd);

    char header_info[MAXLINE];
    char aux[MAXLINE];

    header_info[0]=0;

    //fprintf(logfile,"Source port:%u Destination port:%u Seq. number:%u Ack seq:%u\n",tcph->source,tcph->dest,tcph->seq,tcph->ack_seq);
    snprintf(aux,MAXLINE,"Source port:%u Destination port:%u Seq. number:%u Ack seq:%u\n",tcph->source,tcph->dest,tcph->seq,tcph->ack_seq);
    strcat(header_info,aux);
    //fprintf(logfile,"FLAGS: ");
    snprintf(header_info,MAXLINE,"FLAGS: ");
    if((int)tcph->urg) 
    {
        snprintf(aux,MAXLINE,"URG ");
        strcat(header_info,aux);
    }
    if((int)tcph->ack) 
    {
        snprintf(aux,MAXLINE,"ACK ");
        strcat(header_info,aux);
    }
    if((int)tcph->psh) 
    {
        snprintf(aux,MAXLINE,"PSH ");
        strcat(header_info,aux);
    }
    if((int)tcph->rst) 
    {
        snprintf(aux,MAXLINE,"RST ");
        strcat(header_info,aux);
    }
    if((int)tcph->syn) 
    {
        snprintf(aux,MAXLINE,"SYN ");
        strcat(header_info,aux);
    }
    if((int)tcph->fin) 
    {
        snprintf(aux,MAXLINE,"FIN ");
        strcat(header_info,aux);
    }
    //fprintf(logfile,"\n");
    snprintf(aux,MAXLINE,"\n");
    strcat(header_info,aux);
    //fprintf(logfile,"Window: %d\n",tcph->window);
    snprintf(aux,MAXLINE,"Window: %d\n",tcph->window);
    strcat(header_info,aux);
    //fprintf(logfile,"Checksum: %d\n",tcph->check);
    snprintf(aux,MAXLINE,"Checksum: %d\n",tcph->check);
    strcat(header_info,aux);
    //fprintf(logfile,"Urgent Pointer: %d\n",tcph->urg_ptr);
    snprintf(aux,MAXLINE,"Urgent Pointer: %d\n",tcph->urg_ptr);
    strcat(header_info,aux);
    //DumpHex(buffer,size,logfile);

    myWrite(header_info,fd);

    //fprintf(logfile,"--------------------------------------------------------------------------------------------\n");
}

void printUDP(unsigned char* buffer, int size, int fd)
{
    unsigned short iphdrlen;
    struct iphdr* iph=(struct iphdr*)buffer+sizeof(struct ethhdr);
    iphdrlen=iph->ihl*4;

    struct udphdr* udph=(struct udphdr*)(buffer+iphdrlen+sizeof(struct ethhdr));

    // if(isDNS(buffer,size,0))
    // {
    //     printDNS(buffer,size,0,logfile);
    //     return 1;
    // }


    //fprintf(logfile,"UDP RECEIVED\n");
    printIPs(buffer,size,fd);
    
    char header_data[MAXLINE];

    snprintf(header_data,MAXLINE,"Source port:%u Destination port:%u Length:%u Checksum:%u\n",udph->source,udph->dest,udph->len,udph->check);

    myWrite(header_data,fd);
}

void printICMP(unsigned char* buffer, int size,int fd)
{
    unsigned short iphdrlen;
    struct iphdr* iph=(struct iphdr*) buffer+sizeof(struct ethhdr);
    iphdrlen=iph->ihl*4;

    struct icmphdr* icmph=(struct icmphdr*) buffer+iphdrlen+sizeof(struct ethhdr);
    
    //fprintf(logfile,"ICMP RECEIVED\n");

    printIPs(buffer,size,fd);

    char header_data[MAXLINE];
    char aux[MAXLINE];

    header_data[0]=0;

    snprintf(aux,MAXLINE,"Type: %d ",(unsigned int)icmph->type);

    strcat(header_data,aux);

    if((unsigned int)icmph->type==11) snprintf(aux,MAXLINE," TTL EXPIRED\n");
    else if ((unsigned int)icmph->type==ICMP_ECHOREPLY) snprintf(aux,MAXLINE," ICMP ECHO REPLY\n");
    else snprintf(aux,MAXLINE,"\n");
    strcat(header_data,aux);

    snprintf(aux,MAXLINE,"Code: %d\n",(unsigned int)icmph->code);
    strcat(header_data,aux);
    snprintf(aux,MAXLINE,"Checksum: %d\n",(unsigned int)icmph->checksum);
    strcat(header_data,aux);

    myWrite(header_data,fd);
}

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
            printICMP(buffer,size,fd);
            break;
        case 2:
            //myWrite("IGMP Received",fd);
            write(fd,"IGMP",4);
            break;
        case 6:
            //myWrite("TCP Received",fd);
            write(fd,"TCP",3);
            printTCP(buffer,size,fd);
            break;
        case 17:
            //myWrite("UDP Received",fd);
            write(fd,"UDP",3);
            printUDP(buffer,size,fd);
            break;
        default:
            //myWrite("Unkown Received",fd);
            write(fd,"UNKNOWN",7);
            break;
    }

    //write(fd,"\n",1);
}