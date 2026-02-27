/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CommandHandler.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dbogovic <dbogovic@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/25 17:24:38 by dbogovic          #+#    #+#             */
/*   Updated: 2026/02/25 19:10:05 by dbogovic         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Client.hpp"
#include "../include/CommandHandler.hpp"

#include <iostream>
#include <sys/socket.h>
#include <iostream>
#include <netinet/in.h>
#include <poll.h>
#include <string>
#include <unistd.h>

void CommandHandler::sendWelcome(Client &client)
{
	std::string nick = client.getNickname();
	std::string source = "localhost"; // Or your server name

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
	std::string oldNick = client.getNickname(); // Save current nick BEFORE changing

	// 1. Check for collisions
	int existingFd = findUsingName(newNick);
	if (existingFd != -1 && existingFd != client.getFd())
	{
		client.appendToWriteBuffer("433 * " + newNick + " :Nickname is already in use\r\n");
		return;
	}

	// 2. Perform the change
	client.setNickname(newNick);

	// 3. Logic Gate
	if (!client.getRegistered())
	{
		// Still in registration phase
		if (!client.getUsername().empty() && (client.isAuthenticated() || _password.empty()))
		{
			client.setRegistered(true);
			std::cout << "REGISTRATION COMPLETE!" << std::endl;
			sendWelcome(client);
		}
	}
	else
	{
		// IMPORTANT: The prefix MUST be the OLD nickname
		// Format: :OldNick!User@Host NICK :NewNick
		std::string msg = ":" + oldNick + "!" + client.getUsername() + "@localhost NICK :" + newNick + "\r\n";

		// You must send this to the client THEMSELVES so their UI updates
		client.appendToWriteBuffer(msg);

		// In the future, you will also broadcast this msg to all channels they are in
		std::cout << "Nick changed: " << oldNick << " -> " << newNick << std::endl;
	}
}

void CommandHandler::caseNOTICE(Client &sender, std::vector<std::string> &cmdTokens)
{
	if (cmdTokens.size() < 3)
		return;

	std::string target = cmdTokens[1];
	std::string msg = cmdTokens[2];

	std::string fullMsg = ":" + sender.getNickname() + " NOTICE " + target + " :" + msg + "\r\n";

	int targetFd = findUsingName(target);
	if (targetFd != -1)
	{
		_clients[targetFd].appendToWriteBuffer(fullMsg);
		return;
	}

	std::map<std::string, Channel>::iterator chanIt = _channels.find(target);
	if (chanIt != _channels.end())
	{
		Channel &chan = chanIt->second;

		for (std::map<int, bool>::iterator it = chan.clients.begin(); it != chan.clients.end(); ++it)
		{
			int fd = it->first;
			if (fd == sender.getFd())
				continue;

			_clients[fd].appendToWriteBuffer(fullMsg);
		}
		return;
	}
}

void CommandHandler::caseUSER(Client &client, std::vector<std::string> &cmdTokens)
{
	// USER command structure: USER <username> <mode> <unused> :<realname>
	if (cmdTokens.size() < 5) return;

	std::string username = cmdTokens[1];
	std::string realname = cmdTokens[4];

	client.setUsername(username);
	client.setRealname(realname);

	// CHECK FOR COMPLETION:
	// If we have a Nickname, a Username, and (optionally) a Password -> Register
	if (!client.getNickname().empty() && !client.getUsername().empty())
	{
		if (client.isAuthenticated() || _password.empty())
		{
			client.setRegistered(true); // You need a flag in your client
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
		client.setAuthenticated(false);
		std::string msg = ":ft_irc 464 * :Password incorrect\r\n";
		client.appendToWriteBuffer(msg);
		return;
	} else
	{
		client.setAuthenticated(true);
	}
}

bool isChannel(const std::string& target)
{
	if (target.empty())
		return false;
	std::string prefixes = "#&!+";

	if (prefixes.find(target[0]) != std::string::npos)
	{
		return true;
	}
	return false;
}

void CommandHandler::casePRIVMSG(Client &client, std::vector<std::string> &cmdTokens)
{
	if (cmdTokens.size() < 3) return;
	std::string targetName = cmdTokens[1];
	std::string content = cmdTokens[2];
	std::vector<int> fds;

	if (isChannel(targetName))
		fds = fetchChannelMembers(targetName);
	else
		fds.push_back(findUsingName(targetName));

	if (fds.empty() || fds[0] == -1)
	{
		std::cout << "Cannot find user!!!\n\n" << std::endl;
		client.appendToWriteBuffer(":localhost 401 " + client.getNickname() + " " + targetName + " :No such nick/channel\r\n");
		return;
	}
	for (std::vector<int>::iterator it = fds.begin(); it != fds.end(); ++it)
	{
		std::string relay = ":" + client.getNickname() + "!" + client.getUsername()
							+ "@localhost PRIVMSG " + targetName + " :" + content + "\r\n";
		_clients[*it].appendToWriteBuffer(relay);
	}
}

void CommandHandler::casePING(Client &client, std::vector<std::string> &cmdTokens)
{
	if (cmdTokens.size() > 1)
	{
		std::string token = cmdTokens[1];
		std::string pong = ":ircserv PONG ircserv :" + token + "\r\n";	//i changed here but made no difference i guess
		client.appendToWriteBuffer(pong);
	}
}

void CommandHandler::caseWHOIS(Client &requester, std::vector<std::string> &cmdTokens)
{
	if (cmdTokens.size() < 2)
		return;

	std::string targetNick = cmdTokens[1];
	int targetFd = findUsingName(targetNick);

	if (targetFd == -1)
	{
		std::string msg = ":ft_irc 401 " + requester.getNickname() + " " + targetNick + " :No such nick\r\n";
		requester.appendToWriteBuffer(msg);
		return;
	}

	Client &target = _clients[targetFd];

	std::string msg311 = ":ft_irc 311 " + requester.getNickname() + " "
						+ target.getNickname() + " " + target.getUsername() + " "
						+ "localhost" + " * :" + target.getRealname() + "\r\n";
	requester.appendToWriteBuffer(msg311);

	std::string msg312 = ":ft_irc 312 " + requester.getNickname() + " "
						+ target.getNickname() + " ft_irc :ft_irc server\r\n";
	requester.appendToWriteBuffer(msg312);

	std::string channels;
	for (std::map<std::string, Channel>::iterator it = _channels.begin(); it != _channels.end(); ++it)
	{
		if (it->second.clients.find(targetFd) != it->second.clients.end())
		{
			if (it->second.clients[targetFd])
				channels += "@" + it->first + " ";
			else
				channels += it->first + " ";
		}
	}

	if (!channels.empty())
	{
		std::string msg319 = ":ft_irc 319 " + requester.getNickname() + " "
							+ target.getNickname() + " :" + channels + "\r\n";
		requester.appendToWriteBuffer(msg319);
	}

	std::string msg318 = ":ft_irc 318 " + requester.getNickname() + " "
						+ target.getNickname() + " :End of WHOIS list\r\n";
	requester.appendToWriteBuffer(msg318);
}

void CommandHandler::caseUNKNOWN(Client &client, std::vector<std::string> &cmdTokens)
{
	std::cout << "Unknown command from client " << client.getFd() << ": " << cmdTokens[0] << std::endl;
	client.appendToWriteBuffer("ERROR :Unknown command\r\n");
}

void  CommandHandler::clientRegister(Client &client, std::vector<std::string> &cmdTokens)
{
	switch (cmdType(cmdTokens[0]))
	{
		case CAP:
			caseCAP(client, cmdTokens);
			break;
		case NICK:
			caseNICK(client, cmdTokens);
			break;
		case USER:
			caseUSER(client, cmdTokens);
			break;
		case PASS:
			casePASS(client, cmdTokens);
			break;
		default:
			return;
	}
}

void CommandHandler::chatCommands(std::vector<std::string> &cmdTokens, Client &client)
{
	switch (cmdType(cmdTokens[0]))
	{
		case PING:
			casePING(client, cmdTokens);
			break;
		case WHOIS:
			caseWHOIS(client, cmdTokens);
			break;
		case NOTICE:
			caseNOTICE(client, cmdTokens);
			break;
		case NICK:
			caseNICK(client, cmdTokens);
			break;
		case USER:
			caseUSER(client, cmdTokens);
			break;
		case PASS:
			return;
		case OPER:
			// Not implemented yet
			break;
		case JOIN:
			caseJOIN(client, cmdTokens);
			break;
		case KICK:
			caseKICK(client, cmdTokens);
			break;
		case INVITE:
			caseINVITE(client, cmdTokens);
			break;
		case TOPIC:
			caseTOPIC(client, cmdTokens);
			break;
		case WHO:
			caseWHO(client, cmdTokens);
			break;
		case MODE:
			if (cmdTokens.size() > 1 && cmdTokens[1][0] != '&' && cmdTokens[1][0] != '#'){
				break;
			}
			caseMODE(client, cmdTokens);
			break;
		case PRIVMSG://* FOR ALL MESSAGES -> both channel or private goes through here!
			casePRIVMSG(client, cmdTokens);
			break;
		case UNKNOWN:
			caseUNKNOWN(client, cmdTokens);
			break;
		default:
			break;
	}
}


void CommandHandler::processNewData(Client &client)
{
	while (commandComplete(client.getReadBuffer()))
	{
		std::vector<std::string> cmdTokens = extractCommand(client.getReadBuffer());
		if (cmdTokens.empty()) continue;

		//std::cout << "CMD: " << cmdTokens[0] << std::endl;//* remove later, testing only

		if (!client.getRegistered())
			clientRegister(client, cmdTokens);
		else
			chatCommands(cmdTokens, client);
	}
}

CommandHandler::CommandHandler(const CommandHandler& other)
		: _clients(other._clients),
			_password(other._password),
			_channels(other._channels)
{}

CommandHandler& CommandHandler::operator=(const CommandHandler& other)
{
		if (this != &other)
		{
			_password = other._password;
			_channels = other._channels;
		}
		return *this;
	}

CommandHandler::~CommandHandler() {}
