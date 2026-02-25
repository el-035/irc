/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CommandHandlerChannelControl.cpp                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dbogovic <dbogovic@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/25 17:24:43 by dbogovic          #+#    #+#             */
/*   Updated: 2026/02/25 19:03:37 by dbogovic         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Client.hpp"
#include "../include/CommandHandler.hpp"
#include <iostream>
#include <sstream>
#include <algorithm>

int CommandHandler::getClientFdFromNick(std::string& Nickname){
	for(std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it){
		if (it->second.getNickname() == Nickname)
			return it->first;
	}
	return -1;
};


std::string errMsg(int errCode,const std::string &ClientNick,const std::string &token,const std::string &msg, const std::string& extra){
	std::stringstream ss;
    ss << errCode;

	std::string final = ":ircserv " + ss.str() + " " + ClientNick + " " + token + extra + msg;
	return final;
}

//KICK channel name [reason]
void CommandHandler::caseKICK(Client &client, std::vector<std::string> &cmdTokens){
	try{
		//check tokens
		if (cmdTokens.size() < 3)
			throw(errMsg(ERR_NEEDMOREPARAMS, client.getNickname(), cmdTokens[0], MSG_NEEDMOREPARAMS , ""));

		//check if channel exists
		std::map<std::string, Channel>::iterator curChan = _channels.find(cmdTokens[1]);
		if (curChan == _channels.end())
			throw(errMsg(ERR_NOSUCHCHANNEL, client.getNickname(), cmdTokens[1], MSG_NOSUCHCHANNEL, ""));

		//check if client is part of channel
		std::map<int, bool>::iterator curClien = curChan->second.clients.find(client.getFd());
		if (curClien == curChan->second.clients.end())
			throw(errMsg(ERR_NOTONCHANNEL, client.getNickname(), cmdTokens[1], MSG_NOTONCHANNEL, ""));

		//check if client is operator
		if (curClien->second == false)
			throw(errMsg(ERR_CHANOPRIVSNEEDED, client.getNickname(), cmdTokens[1], MSG_CHANOPRIVSNEEDED, ""));

		//check if user exists
		int userFd = getClientFdFromNick(cmdTokens[2]);
		if (userFd == -1)
			throw(errMsg(ERR_NOSUCHNICK, client.getNickname(), cmdTokens[2], MSG_NOSUCHNICK, ""));

		//check if user is in channel
		curClien = curChan->second.clients.find(userFd);
		if (curClien == curChan->second.clients.end()){
			std::string extra = " " + cmdTokens[1];
			throw(errMsg(ERR_USERNOTINCHANNEL, client.getNickname(), cmdTokens[2], MSG_USERNOTINCHANNEL, extra));
		}

		//send msg to all clients
		std::string reason;
		if (cmdTokens.size() > 3){
			reason = cmdTokens[3];
			for (size_t i = 4; i < cmdTokens.size(); i++)
				reason += " " + cmdTokens[i];
			reason = " :" + reason;
		}
		std::string ejectMsg = ":" + client.getNickname() + "!" + client.getUsername() + "@localhost KICK " + cmdTokens[1] + " " + cmdTokens[2] + reason + "\r\n";
		for (std::map<int, bool>::iterator it = curChan->second.clients.begin(); it != curChan->second.clients.end(); ++it){
			int fd = it->first;
			std::map<int, Client>::iterator cli = _clients.find(fd);
			if (cli != _clients.end())
        		cli->second.appendToWriteBuffer(ejectMsg);
		}

		//delete user from channel client list
		curChan->second.clients.erase(userFd);
	}
	catch(const std::string& msg){
		client.appendToWriteBuffer(msg);
	}
}


//(INVITE, targetNick, channel).
void CommandHandler::caseINVITE(Client &client, std::vector<std::string> &cmdTokens){
	try{
		//chek tokens
		if (cmdTokens.size() != 3)
			throw(errMsg(ERR_NEEDMOREPARAMS, client.getNickname(), cmdTokens[0], MSG_NEEDMOREPARAMS , ""));

		//check if channel exists
		std::map<std::string, Channel>::iterator curChan = _channels.find(cmdTokens[2]);
		if (curChan == _channels.end())
			throw(errMsg(ERR_NOSUCHCHANNEL, client.getNickname(), cmdTokens[2], MSG_NOSUCHCHANNEL, ""));

		//check if client is part of channel
		std::map<int, bool>::iterator curClien = curChan->second.clients.find(client.getFd());
		if (curClien == curChan->second.clients.end())
			throw(errMsg(ERR_NOTONCHANNEL, client.getNickname(), cmdTokens[2], MSG_NOTONCHANNEL, ""));

		//if on mode +i, client needs to be operator
		if (curChan->second.mode[INV] == true && curClien->second == false)
			throw(errMsg(ERR_CHANOPRIVSNEEDED, client.getNickname(), cmdTokens[2], MSG_CHANOPRIVSNEEDED, ""));

		//check if invited user exists
		int userFd = getClientFdFromNick(cmdTokens[1]);
		if (userFd == -1)
			throw(errMsg(ERR_NOSUCHNICK, client.getNickname(), cmdTokens[2], MSG_NOSUCHNICK, ""));

		//check if client is already on channel
		curClien = curChan->second.clients.find(userFd);
		if (curClien != curChan->second.clients.end()){
			std::string extra = " " + cmdTokens[2];			//DOUBLE CHECK
			throw(errMsg(ERR_USERONCHANNEL, client.getNickname(), cmdTokens[1], MSG_USERONCHANNEL, extra));
		}

		//send invite to user
		std::string invite = ":" + client.getNickname() + "!" + client.getUsername() + "@localhost INVITE " + cmdTokens[1] + " :" + cmdTokens[2] + "\r\n";
		std::map<int, Client>::iterator it = _clients.find(userFd);
		it->second.appendToWriteBuffer(invite);

		//send confirmation to inviter
		std::string confirm = ":ircserv 341 " + client.getNickname() + " " + cmdTokens[1] + " " + cmdTokens[2] + "\r\n";
		client.appendToWriteBuffer(confirm);

		//add user to list of invited (if not already there)
		std::list<int>::iterator inv = std::find(curChan->second.invited.begin(), curChan->second.invited.end(), userFd);
		if (inv == curChan->second.invited.end())
			curChan->second.invited.push_back(userFd);
	}
	catch(const std::string& msg){
		client.appendToWriteBuffer(msg);
	}
}

//TOPIC channel [topic]
void CommandHandler::caseTOPIC(Client &client, std::vector<std::string> &cmdTokens){
	try{
		//check min 2 tokens
		if (cmdTokens.size() < 2)
			throw(errMsg(ERR_NEEDMOREPARAMS, client.getNickname(), cmdTokens[0], MSG_NEEDMOREPARAMS , ""));

		//check if channel exists
		std::map<std::string, Channel>::iterator curChan = _channels.find(cmdTokens[1]);
		if (curChan == _channels.end())
			throw(errMsg(ERR_NOSUCHCHANNEL, client.getNickname(), cmdTokens[1], MSG_NOSUCHCHANNEL, ""));

		//check if client is part of channel
		std::map<int, bool>::iterator curClien = curChan->second.clients.find(client.getFd());
		if (curClien == curChan->second.clients.end())
			throw(errMsg(ERR_NOTONCHANNEL, client.getNickname(), cmdTokens[1], MSG_NOTONCHANNEL, ""));

		// if 2 tokens -_>show topic
		if (cmdTokens.size() == 2){

			if (!curChan->second.topic.empty()){
				std::string reply = ":ircserv 332 " + client.getNickname() + " " + cmdTokens[1] + " :" + curChan->second.topic + "\r\n";
				client.appendToWriteBuffer(reply);
			}
			else{
				std::string reply = ":ircserv 331 " + client.getNickname() + " " + cmdTokens[1] + " :No topic is set\r\n";
				client.appendToWriteBuffer(reply);
			}
			return ;
		}

		//if more check operator permission for changing topic
		else if (cmdTokens.size() >= 3){
			if (curChan->second.mode[TOP] == true){
				if (curClien->second == false)
					throw(errMsg(ERR_CHANOPRIVSNEEDED, client.getNickname(), cmdTokens[1], MSG_CHANOPRIVSNEEDED, ""));
			}
		}

		//change topic
		std::string topic = cmdTokens[2];
		for (size_t i = 3; i < cmdTokens.size(); i++)
			topic += " " + cmdTokens[i];
		curChan->second.topic = topic;
		topic = " :" + topic + "\r\n";

		//notify all channel members
		std::string topMsg = ":" + client.getNickname() + "!" + client.getUsername() + "@localhost TOPIC " + cmdTokens[1] + topic;
		for (std::map<int, bool>::iterator it = curChan->second.clients.begin(); it != curChan->second.clients.end(); ++it){
			std::map<int, Client>::iterator fdIt = _clients.find(it->first);
			if (fdIt != _clients.end())
				fdIt->second.appendToWriteBuffer(topMsg);
		}
	}
	catch(const std::string& msg){
		client.appendToWriteBuffer(msg);
	}
}

bool CommandHandler::channelSyntax(const std::string& name){
	if (name[0] != '#' && name[0] != '&')
		return false;
	for(std::string::const_iterator it = name.begin(); it != name.end(); ++it){
		if (*it == ',' || *it == ' ' || *it == '\a')
			return false;
	}
	return true;
}

//JOIN channel key
void CommandHandler::caseJOIN(Client &client, std::vector<std::string> &cmdTokens){
	try{
		//check params
		if (cmdTokens.size() < 2)
			throw(errMsg(ERR_NEEDMOREPARAMS, client.getNickname(), cmdTokens[0], MSG_NEEDMOREPARAMS , ""));

		//channel syntax rules
		if (!channelSyntax(cmdTokens[1])) //to be tested still with nc
			throw(errMsg(ERR_BADCHANMASK, client.getNickname(), cmdTokens[1], MSG_BADCHANMASK , ""));

		//check if channel exists
		std::map<std::string, Channel>::iterator curChan = _channels.find(cmdTokens[1]);
		bool chanCreated = false;
		if (curChan == _channels.end())	{
			//create channel		//SET EVERYTIHNF BITTE
			_channels[cmdTokens[1]];
			chanCreated = true;
			curChan = _channels.find(cmdTokens[1]);
			curChan->second.name = cmdTokens[1];
			curChan->second.topic = "";
			curChan->second.key = "";
			curChan->second.limit = 200;
			curChan->second.clients[client.getFd()] = true;
			//return ;
		}

		//check if client is already part of channel
		if (!chanCreated && curChan->second.clients.find(client.getFd()) != curChan->second.clients.end())
			return;

		//check modes
		if (curChan->second.mode[LIM] == true){
			//if l check max limit
			if (curChan->second.clients.size() >= curChan->second.limit)
				throw(errMsg(ERR_CHANNELISFULL, client.getNickname(), cmdTokens[1], MSG_CHANNELISFULL, ""));

		}
		if (curChan->second.mode[KEY] == true){
			//if k --> needs password
			if (cmdTokens.size() < 3 || cmdTokens[2] != curChan->second.key)
				throw(errMsg(ERR_BADCHANNELKEY, client.getNickname(), cmdTokens[1], MSG_BADCHANNELKEY, ""));
		}
		if (curChan->second.mode[INV] == true){
			//if +i --> check if client is invited
			bool join = false;
			std::list<int>::iterator it;
			for (it = curChan->second.invited.begin(); it != curChan->second.invited.end(); ++it){
				if (*it == client.getFd()){
					join = true;
					break ;
				}
			}
			if (join == false)
				throw(errMsg(ERR_INVITEONLYCHAN, client.getNickname(), cmdTokens[1], MSG_INVITEONLYCHAN, ""));

			//remove from invited list
			curChan->second.invited.erase(it);
		}

		//user joins channel
		if (!chanCreated)
			curChan->second.clients[client.getFd()] = false;

		//confirmation of joining EVERYONE ON CHANNEL
		std::string joined = ":" + client.getNickname() + "!" + client.getUsername() + "@localhost JOIN :" + cmdTokens[1] + "\r\n";
		for (std::map<int, bool>::iterator it = curChan->second.clients.begin(); it != curChan->second.clients.end(); ++it){
			std::map<int, Client>::iterator c = _clients.find(it->first);
			if (c != _clients.end())
				c->second.appendToWriteBuffer(joined);
		}

		// print topic
		if (!curChan->second.topic.empty()){
			std::string topic = ":ircserv 332 " + client.getNickname() + " " + cmdTokens[1] + " :" + curChan->second.topic + "\r\n";
			client.appendToWriteBuffer(topic);
		}
		else{
			std::string topic = ":ircserv 331 " + client.getNickname() + " " + cmdTokens[1] + " :No topic is set\r\n";
			client.appendToWriteBuffer(topic);
		}

		//print list of users
		std::string namesList = "";
		for (std::map<int, bool>::iterator it = curChan->second.clients.begin(); it != curChan->second.clients.end(); ++it){
			std::map<int, Client>::iterator cur = _clients.find(it->first);
			if (cur == _clients.end())
				continue;
			std::string entry;
    		if (it->second)
				entry += "@";
    		entry += cur->second.getNickname();
    		if (!namesList.empty())
    		    namesList += " ";
    		namesList += entry;
		}
		std::string s = ":ircserv 353 " + client.getNickname() + " = " + cmdTokens[1] + " :" + namesList + "\r\n";
		client.appendToWriteBuffer(s);
		s = ":ircserv 366 " + client.getNickname() + " " + cmdTokens[1] + " :End of /NAMES list\r\n";
		client.appendToWriteBuffer(s);
	}
	catch(const std::string& msg){
		client.appendToWriteBuffer(msg);
	}
}

std::vector<int> CommandHandler::fetchChannelMembers(std::string channelName){
	std::vector<int> fds;

	std::map<std::string, Channel>::iterator curChan = _channels.find(channelName);
		if (curChan == _channels.end())	{
			fds.push_back(-1);
			return fds;
		}
		for (std::map<int, bool>::iterator it = curChan->second.clients.begin(); it != curChan->second.clients.end(); ++it){
			fds.push_back(it->first);
		}
		return fds;
}

bool CommandHandler::validateModeToken(std::vector<std::string>& cmdTokens){
	if (cmdTokens.size() < 2)
		return false;
	if (cmdTokens.size() > 2){
		if (cmdTokens[2][0] != '+' && cmdTokens[2][0] != '-')
			return false;
		if (cmdTokens[2][1] != 'l' && cmdTokens[2][1] != 'k' && cmdTokens[2][1] != 'i' && cmdTokens[2][1] != 't' && cmdTokens[2][1] != 'o')
			return false;
		if (cmdTokens[2] == "+l"){
			if (cmdTokens.size() < 4)
				return false;
			std::stringstream ss(cmdTokens[3]);
			size_t lim;
			ss >> lim;
			if (ss.fail() || !ss.eof())
    			return false;
		}
		if (cmdTokens[2] == "+k" && cmdTokens.size() < 4)
			return false;
		if ((cmdTokens[2] == "+o" || cmdTokens[2] == "-o") && cmdTokens.size() < 4)
			return false;
	}
	return true;
}

void	CommandHandler::sendModes(Client& client, std::string& chanName, Channel& curChan){
	std::string modes;
	std::string extra;
	if (curChan.mode[INV])
		modes += "i";
	if (curChan.mode[TOP])
		modes += "t";
	if (curChan.mode[KEY]){
		modes += "k";
		extra += " " + curChan.key;
	}
	if (curChan.mode[LIM]){
		std::stringstream ss;
		ss << curChan.limit;
		modes += "l";
		extra += " " + ss.str();
	}
	if (!modes.empty())
		modes = " +" + modes;
	extra += "\r\n";
	std::string str = ":ircserv 324 " + client.getNickname() + " " + chanName + modes + extra;
	client.appendToWriteBuffer(str);
}

std::string CommandHandler::changeModes(std::vector<std::string>& cmdTokens, Channel& curChan){
	std::string update;
	char sign = cmdTokens[2][0];
	char mode = cmdTokens[2][1];

	switch (mode){
	case 'i':
		if (curChan.mode[INV] && sign == '-'){
			curChan.mode[INV] = false;
			update = "-i\r\n";
		}
		else if (!curChan.mode[INV] && sign == '+'){
			curChan.mode[INV] = true;
			update = "+i\r\n";
		}
		break;
	case 't':
		if (curChan.mode[TOP] && sign == '-'){
			curChan.mode[TOP] = false;
			update = "-t\r\n";
		}
		else if (!curChan.mode[TOP] && sign == '+'){
			curChan.mode[TOP] = true;
			update = "+t\r\n";
		}
		break;
	case 'k':
		if (curChan.mode[KEY] && sign == '-'){
			curChan.mode[KEY] = false;
			curChan.key.clear();
			update = "-k\r\n";
		}
		else if (!curChan.mode[KEY] && sign == '+'){
			curChan.mode[KEY] = true;
			curChan.key = cmdTokens[3];
			update = "+k " + curChan.key + "\r\n";
		}
		break;
	case 'o': {
		std::map<int, bool>::iterator it = curChan.clients.find(getClientFdFromNick(cmdTokens[3]));
		if (it == curChan.clients.end())
			break;
		if (sign == '+' && it->second == false){
			it->second = true;
			update = "+o " + cmdTokens[3] + "\r\n";
		}
		else if (sign == '-' && it->second == true){
			it->second = false;
			update = "-o " + cmdTokens[3] + "\r\n";
		}
		break;
	}
	case 'l':
		if (curChan.mode[LIM] && sign == '-'){
			curChan.mode[LIM] = false;
			curChan.limit = 200;
			update = "-l\r\n";
		}
		else if (sign == '+'){
			curChan.mode[LIM] = true;
			std::stringstream ss(cmdTokens[3]);
			ss >> curChan.limit;
			update = "+l " + cmdTokens[3] + "\r\n";
		}
		break;
	default:
		break;
	}
	return update;
}

//MODE channel letter [key]
void CommandHandler::caseMODE(Client& client, std::vector<std::string>& cmdTokens){
	try{
		if (!validateModeToken(cmdTokens))
			throw(errMsg(ERR_NEEDMOREPARAMS, client.getNickname(), cmdTokens[0], MSG_NEEDMOREPARAMS , ""));

		//check if channel exists
		std::map<std::string, Channel>::iterator curChan = _channels.find(cmdTokens[1]);
		if (curChan == _channels.end())
			throw(errMsg(ERR_NOSUCHCHANNEL, client.getNickname(), cmdTokens[1], MSG_NOSUCHCHANNEL, ""));

		//check if client is part of channel
		std::map<int, bool>::iterator curClien = curChan->second.clients.find(client.getFd());
		if (curClien == curChan->second.clients.end())
			throw(errMsg(ERR_NOTONCHANNEL, client.getNickname(), cmdTokens[1], MSG_NOTONCHANNEL, ""));

		//if no modes, print them
		if (cmdTokens.size() == 2)
			return (sendModes(client, cmdTokens[1], curChan->second));

		//check if client is operator
		if (curClien->second == false)
			throw(errMsg(ERR_CHANOPRIVSNEEDED, client.getNickname(), cmdTokens[2], MSG_CHANOPRIVSNEEDED, ""));

		//save mode in struct
		std::string update = changeModes(cmdTokens, curChan->second);
		if (!update.empty()){
			std::string msg = ":" + client.getNickname() + " MODE " + cmdTokens[1] + update;
			//TO ALL CLIENTS
			for (std::map<int, bool>::iterator it = curChan->second.clients.begin(); it != curChan->second.clients.end(); ++it){
				std::map<int, Client>::iterator clie = _clients.find(it->first);
				clie->second.appendToWriteBuffer(msg);
			}
		}
	}
	catch(const std::string& msg){
		client.appendToWriteBuffer(msg);
	}
}
