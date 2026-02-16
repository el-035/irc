//ADD HEADER LATER
//ADD HEADER LATER
//ADD HEADER LATER



#ifndef CHANNELCONTROL_HPP
#define CHANNELCONTROL_HPP

#include <string>
#include <map>
#include <vector>

struct Channel
{
	std::string name; //double??
	std::string topic;
	std::string mode;
	std::map<int, bool> clients; //int fd and bool isOperator
	// You can add more fields like a list of clients in the channel, etc.
};

class ChannelControl
{
	private:
			std::map<std::string, Channel> _channels; // Map of channel name to Channel struct
	public:
		ChannelControl();//* constructor will be with no parameters, and will initialize the _channels map as empty
		ChannelControl(const ChannelControl& other);
		ChannelControl& operator=(const ChannelControl& other);
		~ChannelControl();

		void ejectClientFromChannel(std::string channelName, int clientFd);//* This function will remove the client with the given file descriptor from the specified channel. You will need to check if the channel exists, if the client is part of the channel, and then remove them accordingly.
		void inviteClientToChannel(std::string channelName, int clientFd);//* This function will handle inviting a client to a channel. You will need to check if the channel exists, if the client is already in the channel, and then add them to the channel's client list. You may also want to send an invitation message to the client.
		void setChannelTopic(std::string channelName, std::string topic);//* This function will set the topic for a specified channel. You will need to check if the channel exists and then update its topic field. You may also want to broadcast the new topic to all clients in the channel.
		void setChannelMode(std::string channelName, std::string mode);//* This function will set the mode for a specified channel. You will need to check if the channel exists and then update its mode field. Channel modes can include things like +m (moderated), +i (invite-only), etc. You may also want to broadcast the new mode to all clients in the channel.
		void channelJoin(std::string channelName, int clientFd);//* This function will handle a client joining a channel. You will need to check if the channel exists, if the client is already in the channel, and then add them to the channel's client list. You may also want to send a message to all clients in the channel announcing the new member.

		std::vector <int> fetchChannelMembers(std::string channelName);//* returns filled up vector list with all fd-s associated with a channel! if Channel doesnt exits, return list with only 1 member and its value is -1
		//* note: channel name starts with # or & - treat both as valid, if neither is at start, return error (-1 value struct)
		//* u can also throw error if youd like just tell me
};

#endif