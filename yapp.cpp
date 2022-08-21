#include <stdio.h>
#include<bits/stdc++.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/ip_icmp.h>
#include <time.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <ctime>
#include <iostream>
#include<poll.h>
#include <chrono> 
using namespace std; 

// Gives the timeout delay for receiving packets in seconds
#define TIMEOUT	4
#define PORT 8080


// ping packet structure
struct pingPacket{
	struct icmphdr hdr;
	char msg[64-sizeof(struct icmphdr)];
};

//Function for sending the ping
void send_ping(int sockfd, char * IP){
	struct sockaddr_in destination ; 
	destination.sin_family = AF_INET;
	destination.sin_port = htons(PORT);

	//Checking whether the IP address is valid or not
	if(inet_aton(IP, (struct in_addr *)&destination.sin_addr.s_addr) == 0){
		cout<<"Bad hostname\n";
		exit(1);
	}

	//Intializing the packet
	struct pingPacket packet, temp;
	bzero(&packet, sizeof(packet));

	packet.hdr.type = ICMP_ECHO;

	//Time instant at which packet is going to send
	auto start = std::chrono::high_resolution_clock::now();

	//Checking if it is successfully send or not
	if(sendto(sockfd, &packet, sizeof(packet), 0, (struct sockaddr *) &destination,sizeof(destination)) < 0){
		cout<<"Failed to send the packet\n";
		exit(1);
	}	


	//Limiting the time.
	struct pollfd pfd;
    pfd.fd = sockfd;
    pfd.events = POLLIN;
    int status = poll((struct pollfd*)&pfd , 1,	TIMEOUT*1000);
    if(status == 0) {
		cout<<"Request timed out or host unreacheable\n";
		exit(1);
	}

	//receive the packet
	int length = sizeof(temp);
	recvfrom(sockfd, &packet, sizeof(packet), 0, (struct sockaddr*)&temp, (socklen_t*)&length);

	auto end = std::chrono::high_resolution_clock::now();

	auto roundTripTime = std::chrono::duration<double, std::milli>(end-start);	//Caluculating round trip time

	//Printing the round trip time
	cout<<"Reply from "<< IP <<" RTT = " << roundTripTime.count() << " ms\n"; 
}

int main(int argc, char *argv[]){

	//Basic check
	if(argc!=2){
		cout<<"Usage ./yapp X.X.X.X\n";
		exit(0);
	}

	int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_ICMP);

	//checking if socket is failed.
	if (sockfd == -1) {
		printf("socket creation failed...\n");
		exit(0);
	}

	//Function for sending ping
	send_ping(sockfd, argv[1]);
	return 0;
}