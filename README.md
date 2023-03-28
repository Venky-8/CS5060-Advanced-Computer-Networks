# CS5060-Advanced-Computer-Networks
This repository contains code for the assignments done as part of the course "CS5060: Advanced Computer Networks" at IIT Hyderabad

## Assignment 2 - Socket Programming

*See problem statement in Assignment 2/Assignment2.pdf*

**This assignment has two parts.  
In part A, I had to write a file server which sends any file (.txt/.png/.pdf) to the client depending on the requested file.  
In part B, I had to extend the file server program in part A to serve multiple clients in parallel without making any client wait.  
Additionally, the maximum number of clients which can parallely download files can be set at the server.**

## Assignment 3 - Socket Programming

*See problem statement in Assignment 3/Assignment3.pdf*

**In this assignment I had to create a bootstrap server which serves the following functions:**  
1. Registeration of services by the file servers at bootstrap server (MSGTYPE: REGISTRN).
2. Discovery of the services by the client at bootstrap server (MSGTYPE: REGISTRN).  

**Was able to implement both functions using only a single socket at bootstrap server,**  
**by distingishing the incoming message type by putting the corresponding MSGTYPE in UDP Payload.**
