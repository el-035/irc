/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: efittant <efittant@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/25 17:19:00 by dbogovic          #+#    #+#             */
/*   Updated: 2026/02/28 12:28:43 by efittant         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Server.hpp"
#include "../include/CommandHandler.hpp"

#include <csignal>
#include <iostream>
#include <stdexcept>
#include <sys/signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h>
#include <unistd.h>
#include <vector>
#include <cerrno>
#include <fcntl.h>

volatile sig_atomic_t g_server_stop = 0;

void handle_signal (int sig)
{
	(void)sig;
	g_server_stop = 1;
}

int Server::signalSetup(void)
{
	struct sigaction sa;
	sa.sa_handler = handle_signal;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	if (sigaction(SIGINT, &sa, NULL) == -1)
		return -1;
	return 0;
}

void Server::closeAllFds(void)
{
	for (size_t i = 0; i < _pollfds.size(); ++i)
	{
		if (_pollfds[i].fd >= 0)
		{
			close(_pollfds[i].fd);
			_pollfds[i].fd = -1;
		}
	}
}

void Server::ServerStart()
{
	if (signalSetup() == -1)
	{
		closeAllFds();
		throw(std::runtime_error("Error: sigaction(); fatal failure"));
	}
	CommandHandler cmdHandler(_clients, _password);

	while (g_server_stop == 0)
	{
		updatePollEvents();
		if (poll(&_pollfds[0], _pollfds.size(), -1) == -1)
		{
			if (g_server_stop == 1)
				break;
			if (errno == EINTR)
				continue;
			throw(std::runtime_error("Error: poll() failed!"));
		}

		for (size_t i = 0; i < _pollfds.size(); ++i)
		{
			int fd = _pollfds[i].fd;
			Client &cli = _clients[fd];
			if (errorOccured(i))
			{
				cmdHandler.updateGroup(fd);
				DisconnectClient(fd, cmdHandler);
				--i;
				continue;
			}
			if (_pollfds[i].revents & POLLOUT)
			{
				if (writeToClient(cli, cmdHandler) == CONNECTION_CLOSED)
				{
					cmdHandler.updateGroup(fd);
					--i;
					continue;
				}
			}
			if (_pollfds.size() <= i || _pollfds[i].fd != fd) { --i; continue; }

			if (_pollfds[i].revents & POLLIN)
			{
				if (fd == _server_fd)
					AddNewClient();
				else
				{
					if (readClientsData(i, cmdHandler) == CONNECTION_CLOSED)
					{
						cmdHandler.updateGroup(fd);
						--i;
						continue;
					}
					if (_pollfds.size() > i && _pollfds[i].fd == fd)
						cmdHandler.processNewData(cli);
				}
			}
		}
	}
	closeAllFds();
}

void Server::Initialize()
{
	_server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (_server_fd == -1)
		throw(std::runtime_error("Error: socket()- Creating server failed!"));
	int opt = 1;
	if (setsockopt(_server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
	{
		close(_server_fd);
		throw(std::runtime_error("Error: setsockopt(): Creating server failed!"));
	}
	fcntl(_server_fd, F_SETFL, O_NONBLOCK);
	{
		close(_server_fd);
		throw(std::runtime_error("Error: fcntl(): Could not set non-blocking mode"));
	}

	sockaddr_in adress;
	adress.sin_family = AF_INET;
	adress.sin_addr.s_addr = INADDR_ANY;
	adress.sin_port = htons(_port);

	if (bind(_server_fd, (struct sockaddr *)&adress, sizeof(adress)) == -1)
	{
		close(_server_fd);
		throw(std::runtime_error("Error: bind(): Creating server failed!"));
	}
	if (listen(_server_fd, 10) == -1)//* this "flips" socket to listen mode (Pasive) - no longer socket is looking for
	{
		close(_server_fd);
		throw(std::runtime_error("Error: listen(): Creating server failed!"));
	}
	pollfd server_pfd = {_server_fd, POLLIN, 0};
	_pollfds.push_back(server_pfd);
	std::cout << "Server waiting on port: " << _port << std::endl;
}
