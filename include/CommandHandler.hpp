/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CommandHandler.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: efittant <efittant@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/25 17:22:58 by dbogovic          #+#    #+#             */
/*   Updated: 2026/02/27 19:28:09 by efittant         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef COMMANDHANDLER_HPP
#define COMMANDHANDLER_HPP

#include "Client.hpp"
//#include "ChannelControl.hpp"
#include <map>
#include <vector>
#include <string>
#include <list>

enum ClientCommand {
	NICK,
	CAP,
	USER,
	PASS,
	JOIN,
	PRIVMSG,
	PING,
	KICK,
	INVITE,
	TOPIC,
    WHO,
	MODE,
	OPER,
	WHOIS,
	NOTICE,
	PART,
	QUIT,
	UNKNOWN
};

enum ChannelModes {
	INV,
	TOP,
	KEY,
	LIM
};

struct Channel
{
	std::string 					name;
	std::string 					topic;
	std::map<ChannelModes, bool> 	mode;
	std::list<int> 					invited; 	//fro invide only mode, we know who has been invited
	size_t							limit;
	std::string 					key;
	std::map<int, bool> 			clients; //int fd and bool isOperator
};

class CommandHandler
{
	private:
		std::map<int, Client>&	  		_clients;
		std::string				 		_password;
		std::map<std::string, Channel> 	_channels; // Map of channel name to Channel struct

		void sendWelcome(Client &client);
		void casePRIVMSG(Client &client, std::vector<std::string> &cmdTokens);
		void caseNICK(Client &client, std::vector<std::string> &cmdTokens);
		void caseUSER(Client &client, std::vector<std::string> &cmdTokens);
		void casePASS(Client &client, std::vector<std::string> &cmdTokens);
		void casePING(Client &client, std::vector<std::string> &cmdTokens);
		void caseUNKNOWN(Client &client, std::vector<std::string> &cmdTokens);
		void runCommands(std::vector<std::string> &cmdTokens, Client &client);
		void caseCAP(Client &client, std::vector<std::string> &cmdTokens);
		std::vector<std::string> extractCommand(std::string& buffer);
		bool commandComplete(const std::string& buffer);
		int cmdType(const std::string& cmd);

		void caseMODE(Client &client, std::vector<std::string> &cmdTokens);
		void caseKICK(Client &client, std::vector<std::string> &cmdTokens);
		void caseINVITE(Client &client, std::vector<std::string> &cmdTokens);
		void caseTOPIC(Client &client, std::vector<std::string> &cmdTokens);
		void caseJOIN(Client &client, std::vector<std::string> &cmdTokens);
		void casePART(Client &client, std::vector<std::string> &cmdTokens);
		void caseWHO(Client &client, std::vector<std::string> &cmdTokens);
		void caseWHOIS(Client &requester, std::vector<std::string> &cmdTokens);
		void caseNOTICE(Client &sender, std::vector<std::string> &cmdTokens);

		void deleteEmptyChannel(std::string channelName);
		bool channelSyntax(const std::string& name);
		bool hasTooLongToken(const std::vector<std::string>& tokens);
		int getClientFdFromNick(std::string& Nickname);
		std::vector <int> fetchChannelMembers(std::string channelName, int fd_sender);//* returns filled up vector list with all fd-s associated with a channel! if Channel doesnt exits, return list with only 1 member and its value is -1
		bool validateModeToken(std::vector<std::string>& cmdTokens);
		void	sendModes(Client& client, std::string& chanName, Channel& curChan);
		std::string changeModes(Client& client, std::vector<std::string>& cmdTokens, Channel& curChan);

	public:
		CommandHandler(std::map<int, Client> &clients, const std::string& password);
		void processNewData(Client &client);
		int findUsingName(std::string name);
		void updateGroup(int clientFd);
		~CommandHandler();
		CommandHandler& operator=(const CommandHandler& other);
		CommandHandler(const CommandHandler& other);
};

#define MAX_TOKEN_LEN			98
#define MAX_PARAMS				15
#define MAX_TRAILING_LEN		400
#define MAX_CLIENT_BUFFER		4096
#define ERR_NEEDMOREPARAMS		461
#define ERR_NOSUCHCHANNEL		403
#define ERR_CHANOPRIVSNEEDED	482
#define ERR_NOSUCHNICK			401
#define ERR_NOTONCHANNEL		442
#define ERR_USERNOTINCHANNEL	441
#define ERR_USERONCHANNEL		443
#define ERR_BADCHANMASK			476
#define ERR_INVITEONLYCHAN		473
#define ERR_BADCHANNELKEY		475
#define ERR_CHANNELISFULL		471

#define MSG_NEEDMOREPARAMS		" :Not enough parameters\r\n"
#define MSG_NOSUCHCHANNEL		" :No such channel\r\n"
#define MSG_CHANOPRIVSNEEDED	" :You're not channel operator\r\n"
#define MSG_NOSUCHNICK			" :No such nick/channel\r\n"
#define MSG_NOTONCHANNEL		" :You're not on that channel\r\n"
#define MSG_USERNOTINCHANNEL	" :They aren't on that channel\r\n"
#define MSG_USERONCHANNEL		" :is already on channel\r\n"
#define MSG_BADCHANMASK			" :Bad Channel Mask\r\n"
#define MSG_INVITEONLYCHAN		" :Cannot join channel (+i)\r\n"
#define MSG_BADCHANNELKEY		" :Cannot join channel (+k)\r\n"
#define MSG_CHANNELISFULL		" :Cannot join channel (+l)\r\n"


#endif
