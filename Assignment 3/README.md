# Assignment 3 - Socket Programming

*See problem statement in Assignment3.pdf*

Code files are bootstrap.cpp, video_server.cpp, image_server.cpp, pdf_server.cpp, text_server.cpp, client.cpp and client2.cpp

Files in server which can be sent to client are: record.mp4, moments.mp3, sample.mp4 (which can be requested by replacing image_server in below steps with video_server) etc. can be found in server_data directory.

To compile use following commands:
$ g++ bootstrap.cpp -o bootstrap
And run using,
$ ./bootstrap

On another terminal do:
$ g++ image_server.cpp -o image_server
$ ./image_server
It will ask maximum no of client to honor, put say 4
$ 4

On another terminal do:
$ g++ client.cpp -o client
$ ./client
We will get list of servers sent by bootstrap server. Enter port number of required server (here image server) mentioned before access token in the entry in list. Ex. 3493
$ 3493
It will ask access token of server. Access tokens can be found at the end of the entries of the list sent by bootstrap server. In our case, ijkl is access token for image server.
$ ijkl
It will ask for filename of file to send
$ iit.jpg
Thus the requested file is received at client and saved in client_data directory.

If you input file say iit.txt it will ask for filename ending with .jpg.

Check that iit.jpg received in client_data folder.

-------------------------------------------------------

Similarly on client2 [also works on client1] test by putting wrong access token:
$ g++ client2.cpp -o client2
$ ./client2
We will get list of servers sent by bootstrap server. Enter port number of server mentioned before access token in each entry in list. Ex. 3491 for video
$ 3493
It will ask access token of server. Access tokens can be found in the list. ijkl is access token for image server. Putting incorrect access token. Say, abcd.
$ abcd
It will outout "Invalid client" at client side and exit.

-------------------------------------------------------

For testing max client limit at server:

To compile use following commands:
$ g++ bootstrap.cpp -o bootstrap
And run using,
$ ./bootstrap

On another terminal do:
$ g++ video_server.cpp -o video_server
$ ./video_server
It will ask maximum no of client to honor, put say 1
$ 1

On another terminal do:
$ g++ client.cpp -o client
$ ./client
We will get list of servers sent by bootstrap server. Enter port number of required server (here image server) mentioned before access token in the entry in list. Ex. 3491
$ 3491
It will ask access token of server.

On another terminal do:
$ g++ client.cpp -o client
$ ./client
We will get list of servers sent by bootstrap server. Enter port number of required server (here image server) mentioned before access token in the entry in list. Ex. 3491
$ 3491

This client will output: "Client limit reached at server" and exit (i.e. Program exits and terminal asks next command).
Server will output same and stop receiving new client requests.


**PLAGIARISM STATEMENT**: I certify that this assignment/report is my own work, based on my personal study and/or research and that I have acknowledged all material and sources used in its preparation, whether they be books, articles, reports, lecture notes, and any other kind of document, electronic or personal communication. I also certify that this assignment/report has not previously been submitted for assessment in any other course, except where specific permission has been granted from all course instructors involved, or at any other time in this course, and that I have not copied in part or whole or otherwise plagiarized the work of other students and/or persons. I pledge to uphold the principles of honesty and responsibility at CSE@IITH. In addition, I understand my responsibility to report honor violations by other students if I become aware of it.
Name of the student: Venkatesh Mangnale
Roll No: CS22MTECH14006
