/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CommandHandlerUtils.cpp                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dbogovic <dbogovic@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/25 17:24:15 by dbogovic          #+#    #+#             */
/*   Updated: 2026/02/27 18:43:08 by dbogovic         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/CommandHandler.hpp"
#include<iostream>

bool CommandHandler::hasTooLongToken(const std::vector<std::string>& tokens)
{
	if (tokens.size() > MAX_PARAMS)
		return true;

	for (size_t i = 0; i < tokens.size(); ++i)
	{
		const std::string& token = tokens[i];

		if (i == tokens.size() - 1 && !token.empty() && token[0] == ':')
		{
			if (token.length() > MAX_TRAILING_LEN)
				return true;
		}
		else
		{
			if (token.length() > MAX_TOKEN_LEN)
				return true;
		}
	}
	return false;
}

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
	return buffer.find("\r\n") != std::string::npos;
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
		size_t len = (nextSpace == std::string::npos) ? std::string::npos : (nextSpace - i);
		std::string token = line.substr(i, len);
		commandTokens.push_back(token);

		std::cout << "SERVER 3: Extracted token: [" << token << "]" << std::endl;

		if (nextSpace == std::string::npos) break;
		i = nextSpace;
	}
	buffer.erase(0, pos + 1);
	if (hasTooLongToken(commandTokens) == true)
	{
		commandTokens.clear();
		commandTokens.push_back("2");
	}
	return commandTokens;
}

CommandHandler::CommandHandler(std::map<int, Client> &clients, const std::string& password) : _clients(clients), _password(password)
	{}

void CommandHandler::updateGroup(int clientFd)
{
	std::vector<std::string> emptyChannels;

	for (std::map<std::string, Channel>::iterator it = _channels.begin();
		it != _channels.end();
		++it)
	{
		Channel& chan = it->second;

		std::map<int, bool>::iterator cit = chan.clients.find(clientFd);
		if (cit != chan.clients.end())
			chan.clients.erase(cit);
		chan.invited.remove(clientFd);

		if (chan.clients.empty())
			emptyChannels.push_back(it->first);
	}
	for (std::vector<std::string>::iterator eit = emptyChannels.begin();
		eit != emptyChannels.end();
		++eit)
	{
		_channels.erase(*eit);
	}
}
