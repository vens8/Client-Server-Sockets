# Client-Server Sockets
Various client-server programs made in C as part of CN assignment (Socket Programming).

# Repo Hierarchy
```bash
├── Sequential
│   ├── Client.c
│   ├── Server.c
│   ├── Results.txt
│   ├── Makefile
├── Concurrent
│   ├── Client.c
│   ├── forkServer.c
│   ├── pthreadServer.c
│   ├── selectServer.c
│   ├── pollServer.c
│   ├── ePollServer.c
│   ├── resultsThread.txt
│   ├── resultsFork.txt
│   ├── resultsSelect.txt
│   ├── resultsPoll.txt
│   ├── resultsEPoll.txt
│   ├── Makefile
└── README.md
```

## Sequential
The sequential client sends 20 sequential requests to the server, each being a number (1-20).
The sequential server connects to the clients in queue and handles them one by one. As soon as it connects with one client, it reads the number from the client, calculates its factorial and sends the result back to the client. This goes on until all the clients in the queue are disconnected. The makefile compiles the code and runs the server object first and 10 clients at once (one in each terminal).

To run the code, run:
```
make sequential
```
Note: You need to install `xterm` first.

## Concurrent
The concurrent client creates 10 threads where each thread represents an individual client (each one sending 20 requests). Multithreading implemented using `pthread`.
There are 5 types of concurrent servers used: Server using `fork`, server using `pthread`, server using `select`, server using `poll` and server using `epoll`. All these servers handle the concurrent client program (essentially multiple clients in concurrency).
To run the code, run:
```
make fork
```
```
make thread
```
```
make select
```
```
make poll
```
```
make epoll
```
Note: You need to install `xterm` first.
