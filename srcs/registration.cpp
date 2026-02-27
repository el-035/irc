/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   registration.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: efittant <efittant@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/26 15:42:38 by dbogovic          #+#    #+#             */
/*   Updated: 2026/02/27 17:39:22 by efittant         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Client.hpp"
#include "../include/CommandHandler.hpp"
#include<algorithm>
#include <iostream>


void CommandHandler::sendWelcome(Client &client)
{
	std::string nick = client.getNickname();
	std::string source = "localhost";

	// RPL_WELCOME (001)
	client.appendToWriteBuffer(":" + source + " 001 " + nick + " :Welcome to the IRC Network " + nick + "\r\n");

	// RPL_YOURHOST (002)
	client.appendToWriteBuffer(":" + source + " 002 " + nick + " :Your host is " + source + ", running version 1.0\r\n");

	// RPL_CREATED (003)
	client.appendToWriteBuffer(":" + source + " 003 " + nick + " :This server was created now\r\n");

	// RPL_MYINFO (004)
	client.appendToWriteBuffer(":" + source + " 004 " + nick + " " + source + " 1.0 io io\r\n");
}

void CommandHandler::caseCAP(Client &client, std::vector<std::string> &cmdTokens)
{
	if (cmdTokens.size() > 1)
	{
		if (cmdTokens[1] == "LS")
		{
			client.appendToWriteBuffer("CAP * LS :multi-prefix\r\n");
		}
		else if (cmdTokens[1] == "REQ")
		{
			// Irssi is asking for 'multi-prefix'
			// We must respond with: CAP * ACK :capability
			std::string feature = (cmdTokens.size() > 2) ? cmdTokens[2] : "";
			client.appendToWriteBuffer("CAP * ACK :" + feature + "\r\n");
			std::cout << "Acknowledged CAP REQ: " << feature << std::endl;
		}
		else if (cmdTokens[1] == "END")
		{
			std::cout << "CAP negotiation finished." << std::endl;
			// Now Irssi will automatically send NICK and USER
		}
	}
}

void CommandHandler::caseNICK(Client &client, std::vector<std::string> &cmdTokens)
{
	if (cmdTokens.size() < 2) return;

	std::string newNick = cmdTokens[1];
	std::string oldNick = client.getNickname();
	int existingFd = findUsingName(newNick);

	if (existingFd != -1 && existingFd != client.getFd())
	{
		client.appendToWriteBuffer("433 * " + newNick + " :Nickname is already in use\r\n");
		return;
	}
	client.setNickname(newNick);
	if (client.getState() != REGISTERED)
	{
		if (!client.getUsername().empty() && (client.getState() == AUTHENTICATED || _password.empty()))
		{
			client.changeState(REGISTERED);
			std::cout << "REGISTRATION COMPLETE!" << std::endl;
			sendWelcome(client);
		}
	}
	else
	{
		//* Format: :OldNick!User@Host NICK :NewNick
		std::string msg = ":" + oldNick + "!" + client.getUsername() + "@localhost NICK :" + newNick + "\r\n";
		client.appendToWriteBuffer(msg);
		std::cout << "Nick changed: " << oldNick << " -> " << newNick << std::endl;
		
		//broadcast message to all clients with common channel
		std::list<int> alreadySent;
		for (std::list<std::string>::iterator chanName = client.channelsJoined.begin(); chanName != client.channelsJoined.end(); ++chanName){
			std::map<std::string, Channel>::iterator curChan = _channels.find(*chanName);
			if (curChan == _channels.end())
				continue;
			for (std::map<int, bool>::iterator curClient = curChan->second.clients.begin(); curClient != curChan->second.clients.end(); ++curClient){
				if (curClient->first == client.getFd())
					continue;
				if (std::find(alreadySent.begin(), alreadySent.end(), curClient->first) != alreadySent.end())
					continue;
				std::map<int, Client>::iterator target = _clients.find(curClient->first);
				if (target != _clients.end()){
					target->second.appendToWriteBuffer(msg);
					alreadySent.push_back(target->first);
				}
			}
		}
	}
}

void CommandHandler::caseUSER(Client &client, std::vector<std::string> &cmdTokens)
{
	//* USER command structure: USER <username> <mode> <unused> :<realname>
	if (cmdTokens.size() < 5) return;

	std::string username = cmdTokens[1];
	std::string realname = cmdTokens[4];

	client.setUsername(username);
	client.setRealname(realname);

	if (!client.getNickname().empty() && !client.getUsername().empty())
	{
		if (client.getState() == AUTHENTICATED || _password.empty())
		{
			client.changeState(REGISTERED);
			std::cout << "REGISTRATION COMPLETE!" << std::endl;
			sendWelcome(client);
		}
	}
}

void CommandHandler::casePASS(Client &client, std::vector<std::string> &cmdTokens)
{
	std::string pass = cmdTokens[1];
	if (pass != _password)
	{
		client.changeState(DISCONNECTING);
		return;
	} else
	{
		client.changeState(AUTHENTICATED);
	}
}
