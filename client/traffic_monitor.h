#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pcap.h>
#include <netinet/ip_icmp.h>
#include <netinet/udp.h>
#include <netinet/tcp.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/ethernet.h>
#include "utils.h"
#include "../commonutils.h"

void printIPs(unsigned char* buffer, int size,int fd);

void printTCP(unsigned char* buffer, int size,int fd);

void printUDP(unsigned char* buffer, int size, int fd);

void printICMP(unsigned char* buffer, int size,int fd);

void processPackets(unsigned char* buffer,int size,int fd);