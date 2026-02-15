#ifndef COMMANDHANDLER_HPP
#define COMMANDHANDLER_HPP

#include "Client.hpp"
#include "ChannelControl.hpp"
#include <map>
#include <vector>
#include <string>

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
    MODE,
    OPER,
    UNKNOWN
};

class CommandHandler
{
    private:
        std::map<int, Client>&      _clients; 
        std::string                 _password;
        ChannelControl              _channelControl;

        void sendWelcome(Client &client);
        void casePRIVMSG(Client &client, std::vector<std::string> &cmdTokens);
        void caseNICK(Client &client, std::vector<std::string> &cmdTokens);
        void caseUSER(Client &client, std::vector<std::string> &cmdTokens);
        void casePASS(Client &client, std::vector<std::string> &cmdTokens);
        void  clientRegister(Client &client, std::vector<std::string> &cmdTokens);
        void casePING(Client &client, std::vector<std::string> &cmdTokens);
        void caseUNKNOWN(Client &client, std::vector<std::string> &cmdTokens);
        void chatCommands(std::vector<std::string> &cmdTokens, Client &client);
        void caseCAP(Client &client, std::vector<std::string> &cmdTokens);
        std::vector<std::string> extractCommand(std::string& buffer);
        bool commandComplete(const std::string& buffer);
        int cmdType(const std::string& cmd);
    public:
        CommandHandler(std::map<int, Client> &clients, const std::string& password) : _clients(clients), _password(password), _channelControl() {}
        void processNewData(Client &client);
        int findUsingName(std::string name);
		//ortodox canonical form?
};

#endif