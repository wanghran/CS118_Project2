# CS118 Project 2

## Team
Team Members: Haoran Wang (505029637), Zhiwen Hu (105025900), Yunsheng Bai (005023812).

Haoran Wang and Zhiwen Hu work on server and part of the client, mainly the connection part. Yunsheng Bai work on most of the client. We designed our server and client structures togather.

## Server and Client Design
### Server
Our server implements a simplified TCP design, where the server will only send cumulative acks and buffer all the out of order packets. The server will keep track of all the clients with a client status struct, which contains the client id, the client address, the client's buffer, client's file name, and the last legit ack number. Map is used to keep order of each clients and their status.

### Client
Our client implements similar simplified TCP design. The client will send all the packets within the congestion window and try to receive as many acks as the size of the congestion window. If any packet gets time-out, the client will retransmit that packet and wait for the correct ack from server. The size change of the congestion window follows the spec.

## Problems
The first big problem we met was to serialize our packet object. We tried different approaches, such as putting it into the stringstream and sending the string through the socker. In the end, we had to manual call memcpy to serialize the object into a char array and transfer that char array. And on the server side, we had to do the same thing in reverse order. Maneuvering pointers in C++ was really subtle and hard to debug.

The second big problem we met was the implementation of time-out. We first tried traverse the all the packets in the congestion window to check the timestamp in each one of them. However, we found out that was hard to implement, especially combining with selection function, which, already, carries a timeout input. Therefore, we decided only use select function to check timeout and kept a status in the packet object, which were init, sent and timeout. 

## Extra Library
Besides all the network libraries, we used deque, chrono, time, map.

## Outside Source
http://developerweb.net/viewtopic.php?id=3196
https://stackoverflow.com/questions/42142473/socket-programming-client-server-udp

## Makefile

This provides a couple make targets for things.
By default (all target), it makes the `server` and `client` executables.

It provides a `clean` target, and `tarball` target to create the submission file as well.

You will need to modify the `Makefile` to add your userid for the `.tar.gz` turn-in at the top of the file.

## Provided Files

`server.cpp` and `client.cpp` are the entry points for the server and client part of the project.

## Academic Integrity Note

You are encouraged to host your code in private repositories on [GitHub](https://github.com/), [GitLab](https://gitlab.com), or other places.  At the same time, you are PROHIBITED to make your code for the class project public during the class or any time after the class.  If you do so, you will be violating academic honestly policy that you have signed, as well as the student code of conduct and be subject to serious sanctions.

## Wireshark dissector

For debugging purposes, you can use the wireshark dissector from `tcp.lua`. The dissector requires
at least version 1.12.6 of Wireshark with LUA support enabled.

To enable the dissector for Wireshark session, use `-X` command line option, specifying the full
path to the `tcp.lua` script:

    wireshark -X lua_script:./confundo.lua

To dissect tcpdump-recorded file, you can use `-r <pcapfile>` option. For example:

    wireshark -X lua_script:./confundo.lua -r confundo.pcap


