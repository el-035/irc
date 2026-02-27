/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CommandHandler.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dbogovic <dbogovic@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/25 17:24:38 by dbogovic          #+#    #+#             */
/*   Updated: 2026/02/27 18:15:06 by dbogovic         ###   ########.fr       */
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
		fds = fetchChannelMembers(targetName, client.getFd());
	else
		fds.push_back(findUsingName(targetName));

	if (!fds.empty() && fds[0] == -1)
	{
		client.appendToWriteBuffer(":localhost 401 " + client.getNickname() + " " + targetName + " :No such nick/channel\r\n");
		return;
	}
	for (std::vector<int>::iterator it = fds.begin(); it != fds.end(); ++it)
	{
		if (*it == -1)
			continue;
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
		std::string pong = "PONG " + token + "\r\n";
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

void CommandHandler::runCommands(std::vector<std::string> &cmdTokens, Client &client)
{
	switch (cmdType(cmdTokens[0]))
	{
		case CAP:
			caseCAP(client, cmdTokens);
			break;
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
			casePASS(client, cmdTokens);
			break;
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
		case PART:
			casePART(client, cmdTokens);
			break;
		case MODE:
			if (cmdTokens.size() > 1 && cmdTokens[1][0] != '&' && cmdTokens[1][0] != '#')
				break;
			caseMODE(client, cmdTokens);
			break;
		case PRIVMSG:
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
	std::vector<std::string> cmdTokens;
	while (commandComplete(client.getReadBuffer()))
	{
		cmdTokens = extractCommand(client.getReadBuffer());
		if (cmdTokens.empty()) continue;
		if (cmdTokens[0] == "2")
		{
			std::string msg = ":server 417 " + client.getNickname() + " :Input line too long\r\n";
			client.appendToWriteBuffer(msg);
		}

		if (client.getState() == CONNECTED)
		{
			if (cmdTokens[0] != "PASS" && cmdTokens[0] != "USER" &&
				cmdTokens[0] != "NICK" && cmdTokens[0] != "CAP")
				{
					client.changeState(DISCONNECTING);
				}
			else
				runCommands(cmdTokens, client);
		}
		else
			runCommands(cmdTokens, client);
	}
	if (client.getState() != AUTHENTICATED && client.getState() != REGISTERED && cmdTokens[0] != "CAP")
	{
		std::cout << "2" << std::endl;
		client.changeState(DISCONNECTING);
		client.clearWriteBuffer();
		std::string msg = ":ft_irc 464 * :Password incorrect\r\n";
		client.appendToWriteBuffer(msg);
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
