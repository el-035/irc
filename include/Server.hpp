/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dbogovic <dbogovic@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/25 17:17:23 by dbogovic          #+#    #+#             */
/*   Updated: 2026/02/27 11:57:41 by dbogovic         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include "Client.hpp"
#include <string>
#include <poll.h>
#include <vector>
#include <map>

enum ServerEnum
{
	ARRIVED,
	NOT_ARRIVED,
	NEW_CLIENT,
	EXISTING_CLIENT,
	READY,
	NOT_READY,
	ERROR,
	EMPTY_READ,
	CONNECTION_CLOSED,
	RETRY_READ,
	NO_DATA,
	OK,
	READ_POSSIBLE
};

#define MAX_CLIENTS 1000

class Server
{

	public:
	Server(int port, const std::string& password);
	~Server();
	void	Initialize();
	void	ServerStart();

	private:
		std::map<int, Client>		_clients;
		std::vector<pollfd>			_pollfds;// List of pollfd structures for monitoring server and client sockets
		int							_port;
		int							_server_fd;
		std::string					_password;

		void		AddNewClient();
		void		DisconnectClient(int fd);
		ServerEnum	writeToClient(Client &client);
		void		closeAllFds();
		void		signalSetup(void);
		bool		currentClienthasData(int i);
		ServerEnum	newData(int i);
		ServerEnum	newDataIs(int i);
		ServerEnum	clientReadRdy(int i);
		bool		isBuffEmpty(int i);
		bool		errorOccured(int i);
		ServerEnum	readClientsData(int i);
		void		updatePollEvents();

		Server(const Server& other);
		Server& operator=(const Server& other);
};

#endif
