// ADD HEADER LATER
// ADD HEADER LATER
// ADD HEADER LATER
#include "../include/ChannelControl.hpp"

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
	//* This function will remove the client with the given file descriptor from the specified channel.
	// You will need to check if the channel exists, if the client is part of the channel, and then remove them accordingly.

	//	(void)channelName; (void)clientFd;

};

// INVITE
void ChannelControl::inviteClientToChannel(std::string channelName, int clientFd) {
	//(void)channelName; (void)clientFd;
	//* This function will handle inviting a client to a channel. You will need to check if the channel exists,
	// if the client is already in the channel, and then add them to the channel's client list.
	// You may also want to send an invitation message to the client.
};

//TOPIC
void ChannelControl::setChannelTopic(std::string channelName, std::string topic) {
	//(void)channelName; (void)topic;
	//* This function will set the topic for a specified channel. You will need to check if the channel exists and
	// then update its topic field. You may also want to broadcast the new topic to all clients in the channel.
};

//MODE --> caseMODE
void ChannelControl::setChannelMode(std::string channelName, std::string mode) {
	//(void)channelName; (void)mode;
	//* This function will set the mode for a specified channel. You will need to check if the channel exists and then update
	// its mode field. Channel modes can include things like +m (moderated), +i (invite-only), etc.
	// You may also want to broadcast the new mode to all clients in the channel.
};

//JOIN
void ChannelControl::channelJoin(std::string channelName, int clientFd) {
	//(void)channelName; (void)clientFd;
	//creates channel if it doesnt exist
	//* This function will handle a client joining a channel. You will need to check if the channel exists,
	// if the client is already in the channel, and then add them to the channel's client list.
	// You may also want to send a message to all clients in the channel announcing the new member
};

std::vector<int> ChannelControl::fetchChannelMembers(std::string channelName) {
	//* returns filled up vector list with all fd-s associated with a channel! if Channel doesnt exits, return list with only 1 member and its value is -1}
};

//* note: channel name starts with # or & - treat both as valid, if neither is at start, return error (-1 value struct)
//* u can also throw error if youd like just tell me