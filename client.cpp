/*
** filename: client.cpp
** Author: Venkatesh Mangnale
** Roll no: CS22MTECH14006
*/

#include<iostream>
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

using namespace std;

#define PORT "3490" // the port client will be connecting to 
#define MAXDATASIZE 100 // max number of bytes we can get at once 

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

	char rcvDataBuf[maxDataSize], tempDataBuf[maxReceiveDataSize];

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
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

	//printf("IP address of server: %s\n", s);

	freeaddrinfo(servinfo);

	cout << "Some sample files on server:" << endl;
	cout << "v1.txt, phrases.txt" << endl;
	cout << "iit.jpg, tokyo_skytree.jpg, airbus.jpg" << endl;
	cout << "problems.pdf" << endl;

	string request;
	cout << "Enter filename with extension of any file in the server to receive: ";
	cin >> request;

	uint32_t strLen = htonl(request.size());
	send(sockfd, &strLen ,sizeof(uint32_t), MSG_CONFIRM);
	send(sockfd, request.c_str(), request.size(), MSG_CONFIRM);

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

	} else if(extension == ".jpg" || extension == ".pdf") {
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
	file = fopen(filepath.c_str(), "w");

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
