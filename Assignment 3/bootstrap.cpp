/*
** Author: Venkatesh Mangnale
** Roll no: CS22MTECH14006
*/

#include<iostream>
#include <stdint.h>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <unordered_map>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <vector>
#include <string>
#include <sys/mman.h>
#include <sys/wait.h>

using namespace std;

#define PORT "3490"  // the port users will be connecting to
#define BACKLOG 10
#define MAXDATASIZE 256 // max number of bytes we can get at once 

const int maxDataSize = 512;

#define MAXBUFLEN 256

// Max no of clients at a time to serve, default 100
int maxClients = 100;

void sigchld_handler(int s);
void *get_in_addr(struct sockaddr *sa);

int main(void)
{
	int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t addr_len;
	struct sigaction sa;
	char s[INET6_ADDRSTRLEN];
	char buf[MAXBUFLEN];
	int rv;
	unordered_map<string, string> serverList;

	char rcvDataBuf[maxDataSize], sendDataBuf[maxDataSize];
	memset(&rcvDataBuf, 0, maxDataSize);
	memset(&sendDataBuf, 0, maxDataSize);

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("server: bind");
			continue;
		}

		struct sockaddr_in *ipv4 = (struct sockaddr_in *) p->ai_addr;
		void *addr = &(ipv4->sin_addr);
		inet_ntop(p->ai_family, addr, s, sizeof s);
		printf("IP address of server: %s\n", s);

		break;
	}

	if (p == NULL)  {
		fprintf(stderr, "server: failed to bind\n");
		exit(1);
	}

	freeaddrinfo(servinfo); // all done with this structure

	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

	cout << "Server: waiting to recvfrom..." << endl;

	while(1) {
			// recv from server trying to register or client trying to discover
			int numbytes = 0, bytes;

			cout << "Waiting for recv \n";

			addr_len = sizeof their_addr;
			if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0, (struct sockaddr *)&their_addr, &addr_len)) == -1) {
				perror("recvfrom");
				exit(1);
			}

			inet_ntop(their_addr.ss_family,
				get_in_addr((struct sockaddr *)&their_addr),
				s, sizeof s);
			printf("server: got connection from %s\n", s);

			printf("server: packet is %d bytes long\n", numbytes);

			buf[numbytes] = '\0';
			printf("server: packet contains \"%s\"\n", buf);

			string rcvDataStr(buf);
			rcvDataStr.erase(0, 9);
			int begpos = 0;
			int pos = rcvDataStr.find(' ');
			string msgtype = rcvDataStr.substr(0, pos);

			if(msgtype == "REGISTRN") {
				cout << "RECEIVED MSGTYPE: REGISTRN \n";
				pos = rcvDataStr.find(' ');
				rcvDataStr.erase(0, pos + 1);
				pos = rcvDataStr.find(' ');
				string serverName = rcvDataStr.substr(0, pos);
				cout << serverName << "\n";
				serverList[serverName] = rcvDataStr;
				cout << serverList[serverName] << "\n";

				// string sendDataStr = "Successfully Registered Server";
				// memset(&buf, 0, MAXDATASIZE);
				// strcpy(buf, sendDataStr.c_str());
				// cout << buf << endl;

				// if ((numbytes = sendto(sockfd, buf, MAXDATASIZE-1, 0, p->ai_addr, p->ai_addrlen)) == -1) {
				// 	perror("client: sendto");
				// 	exit(1);
				// }
			} else if(msgtype == "DISCOVERY") {
				cout << "RECEIVED MSGTYPE: DISCOVERY \n";
				string sendDataStr = "";
				memset(&buf, 0, MAXDATASIZE);
				strcpy(buf, sendDataStr.c_str());

				for(auto& it: serverList) {
					cout << it.second << '\n';
					sendDataStr.append(it.second + " \n ");
				}
				
				// cout << sendDataStr << "\n";
				strcpy(buf, sendDataStr.c_str());

				if ((numbytes = sendto(sockfd, buf, MAXDATASIZE-1, 0,(struct sockaddr *) &their_addr, addr_len)) == -1) {
					perror("server: sendto");
					printf("Oh dear, something went wrong with read()! %s\n", strerror(errno));
					exit(1);
				}
			}
	}
	close(sockfd);
	return 0;
}

void sigchld_handler(int s) {
	(void)s; // quiet unused variable warning

	// waitpid() might overwrite errno, so we save and restore it:
	int saved_errno = errno;

	while(waitpid(-1, NULL, WNOHANG) > 0);

	errno = saved_errno;
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa) {
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
