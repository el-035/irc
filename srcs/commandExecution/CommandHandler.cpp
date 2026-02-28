/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CommandHandler.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dbogovic <dbogovic@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/25 17:24:38 by dbogovic          #+#    #+#             */
/*   Updated: 2026/02/28 09:55:38 by dbogovic         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/Client.hpp"
#include "../../include/CommandHandler.hpp"

#include <iostream>
#include <sys/socket.h>
#include <iostream>
#include <netinet/in.h>
#include <poll.h>
#include <string>
#include <unistd.h>

void CommandHandler::executeCommand(ClientCommand type, std::vector<std::string> &cmdTokens, Client &client)
{
	switch (type)
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
			if (cmdTokens.size() > 1 && (cmdTokens[1][0] == '&' || cmdTokens[1][0] == '#'))
				caseMODE(client, cmdTokens);
			break;
		case PRIVMSG:
			casePRIVMSG(client, cmdTokens);
			break;
		case OPER:
			break;
		case UNKNOWN:
			caseUNKNOWN(client, cmdTokens);
			break;
		default:
			break;
	}
}

void CommandHandler::runCommands(std::vector<std::string> &cmdTokens, Client &client)
{
	if (cmdTokens.empty())
		return;

	ClientCommand type = cmdType(cmdTokens[0]);

	if (type == PASS || type == NICK || type == USER || type == CAP || type == PING)
	{
		executeCommand(type, cmdTokens, client);
		return;
	}

	if (client.getState() != REGISTERED)
	{
		std::string msg = ":ft_irc 451 * :You have not registered\r\n";
		client.appendToWriteBuffer(msg);
		return;
	}

	executeCommand(type, cmdTokens, client);
}

void CommandHandler::processNewData(Client &client)
{
	while (commandComplete(client.getReadBuffer()))
	{
		std::vector<std::string> cmdTokens = extractCommand(client.getReadBuffer());
		if (cmdTokens.empty())
			continue;
		if (cmdTokens[0] == "2")
		{
			std::string msg = ":ft_irc 417 " + client.getNickname() + " :Input line too long\r\n";
			client.appendToWriteBuffer(msg);
			continue;
		}
		runCommands(cmdTokens, client);
		if (client.getState() == DISCONNECTING)
			break;
	}
}
