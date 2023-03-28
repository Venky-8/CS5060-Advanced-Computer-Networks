# Assignment 2 - Socket Programming

*See problem statement in Assignment2.pdf*

**This assignment has two parts.
In part A, I had to write a file server which sends any file (.txt/.png/.pdf) to the client depending on the requested file.
In part B, I had to extend the file server program in part A to serve multiple clients in parallel without making any client wait.
Additionally, the maximum number of clients which can parallely download files can be set at the server.**

## Part A

Code files are serverA.cpp and client.cpp

To compile serverA.cpp and client.cpp, use following commands:
$ g++ serverA.cpp -o serverA
$ g++ client.cpp -o client

To run the client and server program use following commands:

On server side,
$ ./serverA

Then on client side,
$ ./client

Enter complete filename of any file(*.txt, *.jpg, *.pdf) in the ./server_data directory. Ex. v1.txt
$ Enter filename of any file in the server to receive: v1.txt

File will be received and saved inside ./client_data directory

## Part B

Code files are serverB.cpp and client.cpp

To compile serverB.cpp and clientB.cpp, use following commands:
$ g++ serverB.cpp -o serverB
$ g++ client.cpp -o client

To run the client and server program use following commands:

On server side,
$ ./serverB

2. Part A. a) Server asks Maximum number of clients to honor:
$ Enter maximum number of clients which can be served in parallel
For example lets put 2
$  2

Then on client side,
$ ./client
On another terminal
$ ./client
On another terminal
$ ./client

2. Part B. b) answer:
So on the third client if we request some file say iit.jpg, the child process of server serving this client will output "MAX CLIENT LIMIT REACHED OF SERVER" and exits by calling exit(1).
Before the child process serving this client exits, it tells the client to exit (via send and recv functions), then the client program will terminate by executing exit(1) and take next command in terminal, because if client limit at server reached then client executes exit(1).
$ Enter filename of any file in the server to receive: iit.jpg
$ // Program exits and terminal asks next command.

For the previous two terminals proceed by entering filename with extension of any file(*.txt, *.jpg, *.pdf) in the ./server_data directory. Ex. v1.txt
$ Enter filename of any file in the server to receive: v1.txt
File will be received and saved in ./client_data directory


PLAGIARISM STATEMENT: I certify that this assignment/report is my own work, based on my personal study and/or research and that I have acknowledged all material and sources used in its preparation, whether they be books, articles, reports, lecture notes, and any other kind of document, electronic or personal communication. I also certify that this assignment/report has not previously been submitted for assessment in any other course, except where specific permission has been granted from all course instructors involved, or at any other time in this course, and that I have not copied in part or whole or otherwise plagiarized the work of other students and/or persons. I pledge to uphold the principles of honesty and responsibility at CSE@IITH. In addition, I understand my responsibility to report honor violations by other students if I become aware of it.
Name of the student: Venkatesh Mangnale
Roll No: CS22MTECH14006
