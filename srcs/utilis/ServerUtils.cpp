/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerUtils.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: efittant <efittant@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/25 17:19:21 by dbogovic          #+#    #+#             */
/*   Updated: 2026/02/28 13:01:06 by efittant         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/Server.hpp"

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h>
#include <unistd.h>
#include <vector>
#include <cerrno>
#include <fcntl.h>

Server::Server(int port, const std::string& password) : _port(port), _password(password){}
Server::Server(const Server& other) {(void)other;}
Server& Server::operator=(const Server& other) {(void)other; return *this;}
Server::~Server() {}

bool Server::isBuffEmpty(int i)
{
	if (_clients[_pollfds[i].fd].getWriteBuffer().empty())
		return true;
	return false;
}

void Server::updatePollEvents()
{
	for (size_t i = 0; i < _pollfds.size(); ++i)
	{
		if (_clients[_pollfds[i].fd].getState() == DISCONNECTING)
		{
			_pollfds[i].events = POLLOUT;

		} else
		{

			_pollfds[i].events = POLLIN;
			if (!isBuffEmpty(i))
				_pollfds[i].events |= POLLOUT;
		}
	}
}

ServerEnum Server::clientReadRdy(int i)
{
	if (_pollfds[i].revents & POLLOUT)
		return (READY);
	return (NOT_READY);
}

bool Server::currentClienthasData(int i)
{
	if (_pollfds[i].revents == 0)
		return false;
	return true;
}

ServerEnum Server::readClientsData(int i, CommandHandler& cmd_h)
{
	char buf[1024];
	ssize_t b = recv(_pollfds[i].fd, buf, sizeof(buf), MSG_DONTWAIT);
	if (b > 0)
	{
		_clients[_pollfds[i].fd].appendToReadBuffer(std::string(buf, b));
		return READ_POSSIBLE;
	}
	if (b == 0)
	{
		DisconnectClient(_pollfds[i].fd, cmd_h);
		return CONNECTION_CLOSED;
	}
	if (b == -1)
	{
		if (errno == EINTR)
			return RETRY_READ;
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return NO_DATA;
		std::cerr << "Error: recv() fatal; closing client\n";
		DisconnectClient(_pollfds[i].fd, cmd_h);
		return CONNECTION_CLOSED;
	}
	return NO_DATA;
}

void Server::AddNewClient()
{
	sockaddr_in client_addr;
	socklen_t client_addr_len = sizeof(client_addr);

	if (_pollfds.size() >= MAX_CLIENTS)
	{
		int temp_fd = accept(_server_fd, (struct sockaddr *)&client_addr,
															&client_addr_len);
		close(temp_fd);
		std::cerr << "Server full, rejecting connection." << std::endl;
		return;
	}
	int new_fd = accept(_server_fd, (struct sockaddr *)&client_addr,
															&client_addr_len);
	if (new_fd == -1)
	{
		if (errno == EINTR)
			return;
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return;
		std::cerr << "Error: accept() fatal\n";
		return ;
	}
	if (fcntl(new_fd, F_SETFL, O_NONBLOCK) == -1)
	{
		close(new_fd);
		std::cerr << "Error: fcntl(); cannot set non-blocking mode for client.\n";
	}
	pollfd pfd;
	pfd.fd = new_fd;
	pfd.events = POLLIN;
	pfd.revents = 0;
	_pollfds.push_back(pfd);
	_clients[new_fd] = Client(new_fd);
	std::cout << "New client attempting to connect!\n";
}

void Server::DisconnectClient(int fd, CommandHandler& cmd_h)
{
	cmd_h.updateGroup(fd);
	for (size_t i = 0; i < _pollfds.size(); ++i)
	{
		if (_pollfds[i].fd == fd)
		{
			close(fd);
			_clients.erase(fd); // Erase using the FD *before* you lose it
			_pollfds.erase(_pollfds.begin() + i);
			std::cout << "Client " << fd << " disconnected!" << std::endl;
			return;
		}
	}
}

ServerEnum Server::writeToClient(Client &client, CommandHandler& cmd_h)
{
	std::string &buffer = client.getWriteBuffer();
	if (buffer.empty()) return NO_DATA;


	ssize_t s = send(client.getFd(), buffer.data(), buffer.size(), MSG_NOSIGNAL | MSG_DONTWAIT);
	if (s > 0)
	{
		buffer.erase(0, s);
		if (buffer.empty() && client.getState() == DISCONNECTING)
		{
			DisconnectClient(client.getFd(), cmd_h);
			return CONNECTION_CLOSED;
		}
		return OK;
	}
	else if (s == -1)
	{
		if (errno == EINTR)
			return ERROR;
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return ERROR;
		std::cerr << "Error: send(); closing client\n" << std::endl;
		DisconnectClient(client.getFd(), cmd_h);
		return CONNECTION_CLOSED;
	}
	return OK;
}

ServerEnum Server::newData(int i)
{
	if (_pollfds[i].revents & POLLIN)
		return (ARRIVED);
	return (NOT_ARRIVED);
}

ServerEnum Server::newDataIs(int i)
{
	if (_pollfds[i].fd == _pollfds[0].fd)
		return (NEW_CLIENT);
	return (EXISTING_CLIENT);
}

bool Server::errorOccured(int i)
{
	if (_pollfds[i].revents & (POLLERR | POLLHUP))
		return true;
	return false;
}
