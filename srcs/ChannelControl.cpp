// ADD HEADER LATER
// ADD HEADER LATER
// ADD HEADER LATER
#include "../include/ChannelControl.hpp"
#include <iostream>

ChannelControl::ChannelControl() {} //* constructor will be with no parameters, and will initialize the _channels map as empty

ChannelControl::ChannelControl(const ChannelControl &other) : _channels(other._channels) {};
ChannelControl &ChannelControl::operator=(const ChannelControl &other) {
	if (this != &other)
		_channels = other._channels;
	return (*this);
};
ChannelControl::~ChannelControl() {};


// KICK
void ChannelControl::ejectClientFromChannel(std::string channelName, int clientFd) {
	//check if channel exists
	std::map<std::string, Channel>::iterator curChan = _channels.find(channelName);
	if (curChan == _channels.end()){
		std::cout << "channel does not exist: cannot kick user" << std::endl;
		return ;
	}

	//check if client is part of channel
	std::map<int, bool>::iterator curClien = curChan->second.clients.find(clientFd);
	if (curClien == curChan->second.clients.end()){
		std::cout << "user not part of channel: cannot kick user" << std::endl;
		return ;
	}

	//delete client from channel
	curChan->second.clients.erase(clientFd);

};


// INVITE
void ChannelControl::inviteClientToChannel(std::string channelName, int clientFd) {
	//check if channel exists
	std::map<std::string, Channel>::iterator curChan = _channels.find(channelName);
	if (curChan == _channels.end()){
		std::cout << "channel does not exist: cannot invite user" << std::endl;
		return ;
	}
	
	//check if client is part of channel
	std::map<int, bool>::iterator curClien = curChan->second.clients.find(clientFd);
	if (curClien != curChan->second.clients.end()){
		std::cout << "user already in channel: cannot invite user" << std::endl;
		return ;
	}
	
	//add user to chanel
	curChan->second.clients[clientFd] = false; //double check
	//send message to client -->    "You have been invited to join " << _channels.first << std::endl;

};

//TOPIC
void ChannelControl::setChannelTopic(std::string channelName, std::string topic) {
	(void)channelName; (void)topic;
	//* This function will set the topic for a specified channel. You will need to check if the channel exists and
	// then update its topic field. You may also want to broadcast the new topic to all clients in the channel.
};

//MODE --> caseMODE
void ChannelControl::setChannelMode(std::string channelName, std::string mode) {
	(void)channelName; (void)mode;
	//* This function will set the mode for a specified channel. You will need to check if the channel exists and then update
	// its mode field. Channel modes can include things like +m (moderated), +i (invite-only), etc.
	// You may also want to broadcast the new mode to all clients in the channel.
};

//JOIN
void ChannelControl::channelJoin(std::string channelName, int clientFd) {
	(void)channelName; (void)clientFd;
	//creates channel if it doesnt exist
	//* This function will handle a client joining a channel. You will need to check if the channel exists,
	// if the client is already in the channel, and then add them to the channel's client list.
	// You may also want to send a message to all clients in the channel announcing the new member
};

std::vector<int> ChannelControl::fetchChannelMembers(std::string channelName) {
	(void) channelName;
	std::vector<int> hh;
	return hh;
	//return vector with - if channel does not exist
	//* returns filled up vector list with all fd-s associated with a channel! if Channel doesnt exits, return list with only 1 member and its value is -1}
};

//* note: channel name starts with # or & - treat both as valid, if neither is at start, return error (-1 value struct)
//* u can also throw error if youd like just tell me