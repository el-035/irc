/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CommandHandlerUtils.cpp                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: efittant <efittant@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/25 17:24:15 by dbogovic          #+#    #+#             */
/*   Updated: 2026/02/27 15:24:52 by efittant         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/CommandHandler.hpp"
#include<iostream>

int CommandHandler::cmdType(const std::string& cmd)
{
	if (cmd == "NICK") return NICK;
	if (cmd == "CAP") return CAP;
	if (cmd == "USER") return USER;
	if (cmd == "PASS") return PASS;
	if (cmd == "JOIN") return JOIN;
	if (cmd == "INVITE") return INVITE;
	if (cmd == "KICK") return KICK;
	if (cmd == "TOPIC") return TOPIC;
	if (cmd == "MODE") return MODE;
	if (cmd == "WHO") return WHO;
	if (cmd == "WHOIS") return WHOIS;
	if (cmd == "PRIVMSG") return PRIVMSG;
	if (cmd == "PING") return PING;
	if (cmd == "NOTICE") return NOTICE;
	if (cmd == "PART") return PART;
	
	return UNKNOWN;
}

int CommandHandler::findUsingName(std::string name)
{
	for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		if (it->second.getNickname() == name)
			return it->first;
	}
	return -1;
}

bool CommandHandler::commandComplete(const std::string& buffer)
{
	if (buffer.find('\n') != std::string::npos)
		return true; // At least one complete command is present
	return false;
}


std::vector<std::string> CommandHandler::extractCommand(std::string& buffer)
{
	std::cout << "SERVER 1: Entering extractCommand. Buffer size: " << buffer.size() << std::endl;
	std::vector<std::string> commandTokens;
	size_t pos = buffer.find('\n');

	if (pos == std::string::npos) return commandTokens;

	std::string line = buffer.substr(0, pos);

	if (!line.empty() && line[line.size() - 1] == '\r')
		line.erase(line.size() - 1);

	size_t i = 0;
	while (i < line.length())
	{
		while (i < line.length() && line[i] == ' ') i++;
		if (i >= line.length()) break;

		if (line[i] == ':')
		{
			commandTokens.push_back(line.substr(i + 1));
			break;
		}

		size_t nextSpace = line.find(' ', i);
        // CRASH WATCH: Check if this substr calculation is valid
		std::string token = line.substr(i, nextSpace - i);
		commandTokens.push_back(token);

		std::cout << "SERVER 3: Extracted token: [" << token << "]" << std::endl;

		if (nextSpace == std::string::npos) break;
		i = nextSpace;
	}
	buffer.erase(0, pos + 1);
	return commandTokens;
}

CommandHandler::CommandHandler(std::map<int, Client> &clients, const std::string& password) : _clients(clients), _password(password)
	{}
