/*
** filename: serverB.cpp
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

const int maxDataSize = 512;

// Max no of clients at a time to serve, default 100
int maxClients = 100;

int send_file(int socket, string filename);

void sigchld_handler(int s);
void *get_in_addr(struct sockaddr *sa);

int main(void)
{
	int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	struct sigaction sa;
	int yes=1;
	char s[INET6_ADDRSTRLEN];
	int rv;

	char rcvDataBuf[maxDataSize], sendDataBuf[maxDataSize];
	memset(&rcvDataBuf, 0, maxDataSize);
	memset(&sendDataBuf, 0, maxDataSize);

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
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

		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
				sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
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

	freeaddrinfo(servinfo); // all done with this structure

	if (p == NULL)  {
		fprintf(stderr, "server: failed to bind\n");
		exit(1);
	}

	if (listen(sockfd, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}

	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

	//cout << "IP address of server: " << s << endl;

	cout << "Enter maximum number of clients which can be served in parallel" << endl;
	cin >> maxClients;

	cout << "Server: waiting for connections..." << endl;

	// Stores current number of clients
	int *shared = (int *) mmap(NULL, sizeof(int), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0); 

	while(1) {  // main accept() loop
		sin_size = sizeof their_addr;
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if (new_fd == -1) {
			perror("accept");
			continue;
		}
		//cout << "sockfd: " << sockfd << " " << new_fd << endl;		

		inet_ntop(their_addr.ss_family,
			get_in_addr((struct sockaddr *)&their_addr),
			s, sizeof s);
		printf("server: got connection from %s\n", s);

		if (!fork()) { // this is the child process
			close(sockfd); // child doesn't need the listener

			*shared = *shared + 1;
			cout << *shared << endl;

			uint32_t fail = 0;;
			uint32_t tmp;
			if(*shared >= 1 + maxClients) {
				fail = htonl(1);
				tmp = htonl(fail);
				send(new_fd, &tmp ,sizeof(uint32_t), 0);
				printf("MAX CLIENT LIMIT REACHED OF SERVER");
				*shared--;
				close(new_fd);
				exit(1);
			}

			tmp = htonl(fail);
			send(new_fd, &fail, sizeof(uint32_t), 0);

			int numbytes = 0, bytes;
			uint32_t strLen;
			recv(new_fd, &strLen, sizeof(uint32_t), 0);
			strLen = ntohl(strLen);

			std::vector<uint8_t> rcvBuf;
			rcvBuf.resize(strLen, 0x00);
			recv(new_fd, &(rcvBuf[0]), strLen, 0); // Receive the string data
			std::string receivedString(rcvBuf.begin(), rcvBuf.end());
			cout << "File to send: " << receivedString << endl;

			size_t pos = receivedString.find("."); // position of "." in string
			string extension = receivedString.substr(pos);
			string filename = receivedString.substr(0, pos);

			if(extension == ".txt") {
				fstream file;
				string filepath = ".//server_data//" + filename + extension;
				file.open(filepath.c_str(), ios::in | ios::binary);
				if(file.is_open()){
					cout << "[SERVER] : Ready to transmit file \n";
				}
				else{
					cout<<"[ERROR] : File loading failed, Exititng.\n";
					exit(EXIT_FAILURE);
				}

				unsigned int i = 0;

				std::string contents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
				cout << "[SERVER] : Transmission Data Size "<<contents.length() <<" Bytes.\n";

				string str = contents.c_str();

				while(i <= contents.length()) {
					cout<<"[SERVER] : Sending...\n";

					int ending = (i + maxDataSize - 1 < str.length()) ? maxDataSize : -1;
					//cout << contents.c_str() << endl;
					int bytes_sent = send(new_fd, str.substr(i, ending).c_str(), maxDataSize-1, 0);
					cout<<"[SERVER] : Transmitted Data Size "<<bytes_sent<<" Bytes.\n";

					i += bytes_sent;
				}
			} else if(extension == ".jpg" || extension == ".pdf") {
				send_file(new_fd, filename + extension);
			}
			*shared--;
			close(new_fd);
			exit(0);
		}
		close(new_fd);  // parent doesn't need this
	}

	return 0;
}

// send jpg or pdf
int send_file(int socket, string filename) {
	FILE *file;
	int size, read_size, stat, packet_index;
	// send buffer size 100024 bytes for jpg and pdf
	char send_buffer[1000240], read_buffer[256];
	packet_index = 1;

	string filepath = ".//server_data//" + filename;
	file = fopen(filepath.c_str(), "r");
	cout << "Getting File Size" << endl;   

	if(file == NULL) {
		cout << "Error Opening File" << endl;
	} 

	fseek(file, 0, SEEK_END);
	size = ftell(file);
	fseek(file, 0, SEEK_SET);
	cout << "Total File size: "  << size << endl;

	cout << "Sending File Size" << endl;
	write(socket, (void *)&size, sizeof(int));

	cout << "Sending PDF as Byte Array" << endl;
	do {
		stat = read(socket, &read_buffer , 255);
		printf("Bytes read: %i\n",stat);
	} while (stat < 0);

	cout << "Received data" << endl;
	cout << "Socket data: " << read_buffer << endl;

	while(!feof(file)) {
		//Reading the file into the send buffer
		read_size = fread(send_buffer, 1, sizeof(send_buffer)-1, file);

		//Sending data
		do{
			stat = write(socket, send_buffer, read_size);  
		} while (stat < 0);

		cout << "Packet Number: " << packet_index << endl;
		cout << "Packet Size Sent: " << read_size << endl;     

		packet_index++;  

		//Zeroing the send buffer
		bzero(send_buffer, sizeof(send_buffer));
	}
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
