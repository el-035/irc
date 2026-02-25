/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dbogovic <dbogovic@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/07 13:18:41 by dbogovic          #+#    #+#             */
/*   Updated: 2026/02/25 18:26:46 by dbogovic         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/main.hpp"
#include "../include/Server.hpp"

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h>
#include <unistd.h>


int main(int argc, char **argv)
{
	try
	{
		argcCheck(argc);
		int port = portParse(argv[1]);
		std::string pass = passParse(argv[2]);
		Server server(port, pass);
		server.Initialize();
		server.ServerStart();

	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
		return (1);
	}
	return (0);
}
