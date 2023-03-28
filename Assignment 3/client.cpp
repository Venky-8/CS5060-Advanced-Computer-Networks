/*
** Author: Venkatesh Mangnale
** Roll no: CS22MTECH14006
*/

#include <iostream>
#include<fstream>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>

using namespace std;

#define BOOTSTRAPPORT "3490" // the port client will be connecting to 
#define MAXDATASIZE 256 // max number of bytes we can get at once 

const int maxDataSize = 10000;
const int maxReceiveDataSize = 512;

int receive_file(int socket, string filename);
void *get_in_addr(struct sockaddr *sa);

fstream file;

int main(int argc, char *argv[])
{
	int sockfd, numbytes;  
	char buf[MAXDATASIZE];
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];
	string sendDataStr;
	char rcvDataBuf[maxDataSize], tempDataBuf[maxReceiveDataSize];
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t addr_len;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	// hints.ai_flags = AI_PASSIVE;

	if ((rv = getaddrinfo(NULL, BOOTSTRAPPORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	cout << "sockfd: " << sockfd << endl;

	// looping through all the results and connecting to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "client: failed to create socket\n");
		return 2;
	}

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
			s, sizeof s);
	printf("client: connecting to %s\n", s);

	// Send message type discovery to bootstrap server
	sendDataStr = "MSGTYPE: DISCOVERY ";
	memset(&buf, 0, MAXDATASIZE);
	strcpy(buf, sendDataStr.c_str());

	if ((numbytes = sendto(sockfd, buf, MAXDATASIZE-1, 0, p->ai_addr, p->ai_addrlen)) == -1) {
		perror("client: sendto");
		exit(1);
	}

	cout << "Sent packet of " << numbytes << "\n";

	addr_len = sizeof their_addr;
	if ((numbytes = recvfrom(sockfd, buf, MAXDATASIZE-1 , 0, (struct sockaddr *)&their_addr, &addr_len)) == -1) {
		perror("recvfrom");
		exit(1);
	}

	printf("server: packet is %d bytes long\n", numbytes);

	buf[numbytes] = '\0';

	// Received server list from bootstrap server
	printf("List of servers sent: \n\"%s\"\n", buf);

	freeaddrinfo(servinfo);
	close(sockfd);

	// Intitializing TCP connection to respective server

	// Video or image or text file

	// unordered_map<string, string> serverList;
	// std::stringstream ss(buf);
	// std::string to;

	// if (buf != NULL) {
	// 	while(std::getline(ss, to, '\n')){
	// 		cout << to <<endl;
	// 		to = to.erase(to.size()-2, 2);
	// 		string portn(to.end() - 4, to.end());
	// 		int pos1 = to.find(' ');
	// 		string serverName = to.substr(0, pos1);
	// 		serverList[serverName] = portn;
	// 	}
	// }

	string port;
	// string serverName;

	// cout << "Enter server name of the file server you want to connect to: \n";
	// cin >> serverName;
	cout << "Enter port number of the file server you want to connect to: \n";
	cin >> port;
	// port = serverList[serverName];

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if ((rv = getaddrinfo(NULL, port.c_str(), &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	cout << "sockfd: " << sockfd << endl;

	// looping through all the results and connecting to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			perror("client: connect");
			close(sockfd);
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		return 2;
	}

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
			s, sizeof s);
	printf("client: connecting to %s\n", s);

	freeaddrinfo(servinfo);

	uint32_t fail = 0;

	cout << "Waiting to recv" << endl;

	numbytes = 0;
	do {
		numbytes += recv(sockfd, &fail, sizeof(uint32_t), 0);
	} while(numbytes < sizeof(uint32_t));

	fail = ntohl(fail);

	cout << fail << endl;
	if(fail == 1) {
		cout << "Client limit reached at server \n";
		exit(1);
	}

	string token;
	cout << "Enter access token of server: ";
	cin >> token;

	if ((numbytes = send(sockfd, token.c_str(), token.size(), 0)) == -1) {
		perror("client: sendto");
		exit(1);
	}

	uint32_t isValid = 1;
	do {
		numbytes = recv(sockfd, &isValid, sizeof(uint32_t), 0); // Receive the string data
	} while(numbytes != sizeof(uint32_t));
	isValid = ntohl(isValid);
	if(!isValid) {
		cout << "Invalid Client \n";
		close(sockfd);
		exit(1);
	}

	string request;
	cout << "Enter filename with extension of any file in the server to receive: ";
	cin >> request;

	uint32_t strLen = htonl(request.size());

	if ((numbytes = send(sockfd, &strLen ,sizeof(uint32_t), 0)) == -1) {
		perror("client: sendto");
		exit(1);
	}

	if ((numbytes = send(sockfd, request.c_str(), request.size(), 0)) == -1) {
		perror("client: sendto");
		exit(1);
	}

	size_t pos = request.find("."); // position of "." in string
	string extension = request.substr(pos);
	string filename = request.substr(0, pos);

	if(extension == ".txt") {
		memset(&rcvDataBuf, 0, maxDataSize);
		unsigned int i = 0;

		fstream file;

		string filepath = "./client_data/" + filename + extension;
		file.open(filepath.c_str(), ios::out | ios::trunc | ios::binary);

		if(file.is_open()){
			cout<<"[CLIENT] : Created File.\n";
		}
		else{
			cout<<"[ERROR] : File creation failed, Exititng.\n";
			exit(EXIT_FAILURE);
		}

		while(1){
			memset(&tempDataBuf, 0, maxReceiveDataSize);

			if ((numbytes = recv(sockfd, &tempDataBuf, maxReceiveDataSize-1, 0)) == -1) {
				printf("client: connecting to %s\n", s);
				perror("recv");
				exit(1);
			}
			if(numbytes == 0) {
				break;
			}
			tempDataBuf[maxReceiveDataSize-1] = '\0';
			strcat(rcvDataBuf, tempDataBuf);
		}

		cout << "client received: " << rcvDataBuf << endl;
		file << rcvDataBuf;
		cout<<"[CLIENT] : File Saved.\n";

	} else if(extension == ".jpg" || extension == ".pdf" || extension == ".mp4" || extension == ".mp3") {
		receive_file(sockfd, filename + extension);
	}

	close(sockfd);
	return 0;
}

int receive_file(int socket, string filename) {
	int buffersize = 0, recv_size = 0, size = 0, read_size, write_size, packet_index =1,stat;

	char filearray[1000241],verify = '1';
	FILE *file;

	//Find size of file
	do{
		stat = read(socket, &size, sizeof(int));
	}while(stat < 0);

	cout << "Packet size: " << stat << endl;
	cout << "File size: " << size << endl;

	char buffer[] = "Received";

	do{
	stat = write(socket, &buffer, sizeof(int));
	}while(stat<0);

	cout << "Reply sent" << endl;

	string filepath = "./client_data/" + filename;
	file = fopen(filepath.c_str(), "wb");

	if(file == NULL) {
		cout << "Error has occurred. File could not be opened" << endl;
		return -1;
	}

	//Looping till we do not receive the entire file
	int need_exit = 0;
	struct timeval timeout = {10,0};

	fd_set fds;
	int buffer_fd, buffer_out;

	while(recv_size < size) {
		FD_ZERO(&fds);
		FD_SET(socket,&fds);

		buffer_fd = select(FD_SETSIZE,&fds,NULL,NULL,&timeout);

		if (buffer_fd < 0)
		cout << "error: bad file descriptor set." << endl;

		if (buffer_fd == 0)
		cout << "error: buffer read timeout expired" << endl;

		if (buffer_fd > 0)
		{
			do{
				read_size = read(socket, filearray, 10241);
			} while(read_size <0);

			cout << "Received packet number: " << packet_index << endl;
			cout << "Size of packet: " << read_size << endl;

			//Write the currently read data into our file
			write_size = fwrite(filearray, 1, read_size, file);
			cout << "Written image size: " << write_size << endl; 
				if(read_size != write_size) {
					printf("error in read write\n");    }

				//Increment the total number of bytes read
				recv_size += read_size;
				packet_index++;
				cout << "Total received image size:" << recv_size << endl;
		}

	}
	fclose(file);
	return 0;
}

// get sockaddr either for IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
