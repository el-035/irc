//ADD header file!
//ADD header file!
//ADD header file!
//ADD header file!
//ADD header file!
//ADD header file!


#include "../include/Server.hpp"
#include "../include/CommandHandler.hpp"
#include "../include/main.hpp"

#include <csignal>
#include <iostream>
#include <sys/signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>
#include <vector>

volatile sig_atomic_t g_server_stop = 0;

Server::Server(int port, const std::string& password) : _port(port), _password(password)
{
}   

void handle_signal (int sig)
{
	(void)sig;
	g_server_stop = 1;
}

void signalSetup(void)
{
	struct sigaction sa;
	sa.sa_handler = handle_signal;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	if (sigaction(SIGINT, &sa, NULL) == -1)
		return ;
}

void Server::ServerStart()
{
	signalSetup();// ! handle possibilty of sigaction failure!
	CommandHandler cmdHandler(_clients, _password);

	while (g_server_stop == 0)
	{
		updatePollEvents();
		if (poll(&_pollfds[0], _pollfds.size(), -1) == -1) break;

		for (size_t i = 0; i < _pollfds.size(); ++i)
		{
			if (currentClienthasData(i) == false) continue;
			if (errorOccured(i))
			{
				DisconnectClient(i);
				--i;
				continue;
			}
			if (newData(i) == ARRIVED)
			{
			
				if (newDataIs(i) == NEW_CLIENT) {
					AddNewClient();
				} else if (newDataIs(i) == EXISTING_CLIENT)
				{
					if (readClientsData(i) == EMPTY_READ)
					{
						DisconnectClient(i);
						--i;
						continue;
					}
					
					cmdHandler.processNewData(_clients[_pollfds[i].fd]);
	
					if (_clients[_pollfds[i].fd].isAuthenticated() == false)
					{
						DisconnectClient(i);
						--i;
						continue;
					}
				}
			}
			if (clientReadRdy(i) == READY)
			{
				writeToClient(_clients[_pollfds[i].fd]);
			}
		}
	}
}



void Server::Initialize()
{
	_server_fd = socket(AF_INET, SOCK_STREAM, 0);//* server FD - to establish connections
	if (_server_fd == -1)
		throw(std::runtime_error("Error: socket()- Creating server failed!"));
	//* SOCK_STREAM - means we will be using TCP
	//* following code is to make sure that we can use port again if we crash witouth waiting 2 min
	/*
		Note: when server closes, TCP protocl enters TIME_WAIT state where it waits few mins
		to make sure no stray data packets from prec connections arrive late and confuse new program			SO_REUSEADDR - tells kernel- I dont care about risk, give me port now!
	*/
	int opt = 1;
	if (setsockopt(_server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
	{
		close(_server_fd);
		throw(std::runtime_error("Error: setsockopt(): Creating server failed!"));
	}

	//*
	if (fcntl(_server_fd, F_SETFL, O_NONBLOCK) == -1)//* change setting of fd file to Nonblocking
	{
		close(_server_fd);
		throw(std::runtime_error("Error: fcntl(): Creating server failed!"));
	}
	//* main goal of following code is to give this fd reciever adress
	sockaddr_in adress;//* create object "adress" of type sockaddr_in
	adress.sin_family = AF_INET; //* rule 1 - use ipV4
	adress.sin_addr.s_addr = INADDR_ANY;//* rule 2 -Listen any network interface
	adress.sin_port = htons(_port);//* rule 3 - use this specific port
	//* htons() transforms port from Little endian to Big endian (just digit order)

	if (bind(_server_fd, (struct sockaddr *)&adress, sizeof(adress)) == -1)//* "glue" - connects fd with adress/port
	{
		close(_server_fd);
		throw(std::runtime_error("Error: bind(): Creating server failed!"));
	}
	//* bind() also claims port to this specific IP adress
	//* we also have to cast it into generic sockaddr in order for "old" C func bind() func to accept it
	if (listen(_server_fd, 10) == -1)//* this "flips" socket to listen mode (Pasive) - no longer socket is looking for
	{
		close(_server_fd);
		throw(std::runtime_error("Error: listen(): Creating server failed!"));
	}
	//* connections, now it waits!
	//* 10 - waiting room size;"" - if 20 ppl want to connect at same time - 10 would go to waiting,
	//* and 10 would be refused ("Connection refused!")
	//*
	//*list of fds of clients in vector (easy to insert or delete if someone disconnects)
	pollfd server_pfd = {_server_fd, POLLIN, 0};//*first entry - SERVER ITSELF!;
	//* POLLIN - tells poll() to wake me up only if there is incomming data (POLLIN-flag)
	//! for server listener "incomming data" usually means a new connection waiting for accept!
	//*0 - initializes revents (returned eveents) to zero - Kernel will rewrite this to tell you what happened
	_pollfds.push_back(server_pfd);
	//! Rule: For almost all IRC server implementations, fds[0] is usually reserver for server listener
	std::cout << "Server waiting on port: " << _port << std::endl;//* console log

}
