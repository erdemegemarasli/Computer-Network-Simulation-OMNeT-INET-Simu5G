# Computer-Network-Simulation-OMNeT-INET-Simu5G
Computer network simulation assignments written with OMNeT++, INET and Simu5G for Master Practical Course: Computer Network Simulation.

Each assignment has extended implementation when it's compared with the previous assignment. Each assignment has a guide, implementation, results, and theory reports to give information and answer questions about the assignment.

Assignments 1 and 2 are implemented with OMNeT++ and INET.

In addition to that, assignments 3 and 4 were also implemented with Simu5G in addition to OMNeT++ and INET to achieve 5G and MEC.

Below there is a short description from the assignment 1 implementation section for better understanding. Please check the implementation report of each assignment separately for better understanding.

--------------------------------------------------------------------------------------------

A robust messaging app for people stuck in bunkers after a catastrophe like a nuclear
warfare.

The application has two major aims as follows:
1. Finding the location of a person in a bunker.
2. Communicating with the other people in the bunkers via text messages.

The application runs on an infrastructure that connect multiple bunkers to each other via Ethernet
connections. Each bunker has a bunker router that acts as a gateway between the people in that
bunker and the other bunkers.

There is a main router that has connection to all of the bunkers. All of the traffic between bunkers
goes through this main router. There is a server connected to the main router, which acts as a
rendezvous point.

At regular intervals, each person sends a HeartBeat signal to the server automatically. The server
understands that this person is still at the given bunker and notes to its database.
When someone wants to learn if a desired person is at one of the bunkers, it sends a lookup
request to the server. When the server receives this lookup request, it checks its database. If the
server finds a valid record in its database for the desired person, it sends back a positive response
with the desired person’s location. Otherwise, it sends a negative response. When the user gets a
positive response from the server, it saves all the information to its address record for future use.
Users can also try to send a text message to another user. When someone tries to send a message,
it first looks at its address book. If the user has the information about the receiver in its address
book, it directly sends the message to the given IP address in the address book. Otherwise, it sends
a lookup request to the server to learn if the receiver is at one of the bunkers. If the user receives a
positive response from the server, it saves the information to its address book and sends the text
message to the given IP address.
