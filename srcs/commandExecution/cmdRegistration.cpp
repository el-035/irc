/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cmdRegistration.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dbogovic <dbogovic@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/26 15:42:38 by dbogovic          #+#    #+#             */
/*   Updated: 2026/02/28 09:50:59 by dbogovic         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/Client.hpp"
#include "../../include/CommandHandler.hpp"
#include <iostream>
#include <set>

void CommandHandler::attemptRegistration(Client &client)
{
	if (client.getState() == REGISTERED)
		return;

	if (client.getState() != AUTHENTICATED)
	{
		client.appendToWriteBuffer(":ft_irc 464 * :Password incorrect\r\n");
		client.changeState(DISCONNECTING);
		return ;
	}
	bool hasNick = !client.getNickname().empty();
	bool hasUser = !client.getUsername().empty();
	bool passOk = (_password.empty() || client.getState() == AUTHENTICATED);

	if (hasNick && hasUser && passOk)
	{
		client.changeState(REGISTERED);
		std::cout << "Registration complete for: " << client.getNickname() << std::endl;
		sendWelcome(client);
	}
}

void CommandHandler::sendWelcome(Client &client)
{
	std::string nick = client.getNickname();
	std::string srv = "ft_irc.localhost";

	client.appendToWriteBuffer(":" + srv + " 001 " + nick + " :Welcome to the IRC Network " + nick + "\r\n");
	client.appendToWriteBuffer(":" + srv + " 002 " + nick + " :Your host is " + srv + ", running version 1.0\r\n");
	client.appendToWriteBuffer(":" + srv + " 003 " + nick + " :This server was created recently\r\n");
	client.appendToWriteBuffer(":" + srv + " 004 " + nick + " " + srv + " 1.0 i o\r\n");
}

void CommandHandler::caseCAP(Client &client, std::vector<std::string> &cmdTokens)
{
	if (cmdTokens.size() < 2)
		return;

	if (cmdTokens[1] == "LS")
		client.appendToWriteBuffer("CAP * LS :multi-prefix\r\n");
	else if (cmdTokens[1] == "REQ")
	{
		std::string feature = (cmdTokens.size() > 2) ? cmdTokens[2] : "";
		client.appendToWriteBuffer("CAP * ACK :" + feature + "\r\n");
	}
	else if (cmdTokens[1] == "END")
		std::cout << "CAP negotiation finished for FD: " << client.getFd() << std::endl;
}

void CommandHandler::casePASS(Client &client, std::vector<std::string> &cmdTokens)
{
	if (client.getState() == REGISTERED)
	{
		client.appendToWriteBuffer(":ft_irc 462 " + client.getNickname() + " :Unauthorized command (already registered)\r\n");
		return;
	}
	if (cmdTokens.size() < 2)
	{
		client.appendToWriteBuffer(":ft_irc 461 * PASS :Not enough parameters\r\n");
		return;
	}

	if (cmdTokens[1] == _password)
	{
		client.changeState(AUTHENTICATED);
		attemptRegistration(client);
	}
	else
	{
		client.appendToWriteBuffer(":ft_irc 464 * :Password incorrect\r\n");
		client.changeState(DISCONNECTING);
	}
}

void CommandHandler::caseNICK(Client &client, std::vector<std::string> &cmdTokens)
{
	if (cmdTokens.size() < 2)
	{
		client.appendToWriteBuffer(":ft_irc 431 * :No nickname given\r\n");
		return;
	}

	std::string newNick = cmdTokens[1];
	int existingFd = findUsingName(newNick);

	if (existingFd != -1 && existingFd != client.getFd())
	{
		client.appendToWriteBuffer(":ft_irc 433 * " + newNick + " :Nickname is already in use\r\n");
		return;
	}

	if (client.getState() == REGISTERED)
	{
		std::string oldNick = client.getNickname();
		std::string msg = ":" + oldNick + "!" + client.getUsername() + "@localhost NICK :" + newNick + "\r\n";
		client.setNickname(newNick);
		broadcastToNickChange(client, msg);
		client.appendToWriteBuffer(msg);
	}
	else
	{
		client.setNickname(newNick);
		attemptRegistration(client);
	}
}

void CommandHandler::caseUSER(Client &client, std::vector<std::string> &cmdTokens)
{
	if (client.getState() == REGISTERED)
	{
		client.appendToWriteBuffer(":ft_irc 462 " + client.getNickname() + " :Unauthorized command\r\n");
		return;
	}
	if (cmdTokens.size() < 5)
	{
		client.appendToWriteBuffer(":ft_irc 461 * USER :Not enough parameters\r\n");
		return;
	}

	client.setUsername(cmdTokens[1]);
	client.setRealname(cmdTokens[4]);
	attemptRegistration(client);
}

void CommandHandler::broadcastToNickChange(Client &client, const std::string &msg)
{
	std::set<int> notifiedFds;
	for (std::list<std::string>::iterator it = client.channelsJoined.begin(); it != client.channelsJoined.end(); ++it)
	{
		Channel &chan = _channels[*it];
		for (std::map<int, bool>::iterator cit = chan.clients.begin(); cit != chan.clients.end(); ++cit)
		{
			int targetFd = cit->first;
			if (targetFd != client.getFd() && notifiedFds.find(targetFd) == notifiedFds.end())
			{
				_clients[targetFd].appendToWriteBuffer(msg);
				notifiedFds.insert(targetFd);
			}
		}
	}
}
