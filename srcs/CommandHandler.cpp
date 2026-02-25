//HEADER ADD LATER
//HEADER ADD LATER
//HEADER ADD LATER


#include "../include/Client.hpp"
#include "../include/CommandHandler.hpp"
//#include "../include/ChannelControl.hpp"

#include <iostream>
#include <sys/socket.h>
#include <iostream>
#include <netinet/in.h>
#include <fcntl.h>
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

void CommandHandler::caseUSER(Client &client, std::vector<std::string> &cmdTokens)
{
	// USER command structure: USER <username> <mode> <unused> :<realname>
	if (cmdTokens.size() < 5) return; // Strict IRC usually requires 4 params

	std::string username = cmdTokens[1];
	std::string realname = cmdTokens[4]; // assuming parser handles ":"

	client.setUsername(username);
	client.setRealname(realname); // If you have this setter

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
		client.appendToWriteBuffer("ERROR :Invalid password\r\n");
		return; // Stop processing further commands for this client
	} else 
	{
	   // std::cout << "Client " << client.getFd() << " authenticated successfully." << std::endl;
		client.setAuthenticated(true);
	}
}
/*
!Split message into tokens!
!Then fetch vector of ids of persons in interest
!if any reason not to send text -> write error str to buffer! 
!else append to all in vector list of fds message!

*/

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
							+ "@localhost PRIVMSG " + targetName + " :" + content + "\r\n";//! check how targetname deals with channels
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
		std::cout << "Sent PONG to client " << client.getFd() << std::endl;
	}
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
		case NICK:
			caseNICK(client, cmdTokens);
			break;
		case USER:
			caseUSER(client, cmdTokens);
			break;  
		case PASS: 
			return; // These are handled in registration phase
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
			if (cmdTokens.size() > 1 && cmdTokens[1][0] != '&' && cmdTokens[1][0] != '#')
				break;
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

