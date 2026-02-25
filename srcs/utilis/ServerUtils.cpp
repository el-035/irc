/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerUtils.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dbogovic <dbogovic@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/25 17:19:21 by dbogovic          #+#    #+#             */
/*   Updated: 2026/02/25 19:17:57 by dbogovic         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/Server.hpp"

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h>
#include <unistd.h>
#include <vector>

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
		_pollfds[i].events = POLLIN;
		if (!isBuffEmpty(i))
			_pollfds[i].events |= POLLOUT;
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

ServerEnum Server::readClientsData(int i)
{
	char buf[1024];
	int b = recv(_pollfds[i].fd, buf, sizeof(buf), 0);

	std::cout << "total number of bytes read : " << b << std::endl;
	for (int j = 0; j < b; j++)
	{
		if (buf[j] == '\r') std::cout << "\\r";
		else if (buf[j] == '\n') std::cout << "\\n";
		else std::cout << buf[j];
	}
	std::cout << std::flush;
	std::cout << std::endl;
	if (b > 0)
	{
		_clients[_pollfds[i].fd].appendToReadBuffer(std::string(buf, b));
		return (READ_POSSIBLE);
	}
	return (EMPTY_READ);
}

void Server::AddNewClient()
{
	struct sockaddr_in client_addr;
	socklen_t client_addr_len = sizeof(client_addr);
	int new_fd = accept(_server_fd, (struct sockaddr *)&client_addr, &client_addr_len);

	if (new_fd == -1)
	{
		std::cerr << "Accept has failed!" << std::endl;
		return; // Handle accept error (e.g., log it, but don't crash the server)
	}
	pollfd pfd;
	pfd.fd = new_fd;
	pfd.events = POLLIN;
	pfd.revents = 0;
	_pollfds.push_back(pfd);
	Client new_client(new_fd);
	new_client.setAuthenticated(true); // Mark new client as authenticated for testing purposes
	_clients[new_fd] = new_client;
	std::cout << "New client connected!" << std::endl;
}

void Server::DisconnectClient(size_t i)
{
	if (_pollfds[i].fd != -1)
		close(_pollfds[i].fd);
	_pollfds[i].fd = -1;
	_clients.erase(_pollfds[i].fd);
	_pollfds.erase(_pollfds.begin() + i);
	std::cout << "Client disconnected!" << std::endl;
}

void Server::writeToClient(Client &client)
{
	std::string &buffer = client.getWriteBuffer();
	if (buffer.empty()) return;

	// Try to send what is in the buffer
	int s = send(client.getFd(), buffer.c_str(), buffer.size(), 0);

	if (s > 0)
	{
		buffer.erase(0, s);
	}
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

