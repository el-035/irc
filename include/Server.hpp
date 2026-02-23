//ADD HEADER FILE!
//ADD HEADER FILE!
//ADD HEADER FILE!


#ifndef SERVER_HPP
#define SERVER_HPP

#include "Client.hpp"
#include <string>
#include <poll.h>
#include <vector>
#include <map>

enum ServerEnum
{
	WAITING_FOR_NICK,//not used
	WAITING_FOR_PASS,//not used
	AUTHENTICATED,//not used
	ARRIVED,
	NOT_ARRIVED,
	NEW_CLIENT,
	EXISTING_CLIENT,
	READY,
	NOT_READY,
	ERROR,
	EMPTY_READ,
	READ_POSSIBLE
};


class Server
{
		
	public:
	Server(int port, const std::string& password);
	void Initialize();//Sets up server socket, binds it to port, and starts listening. Also adds server socket to _pollfds for monitoring incoming connections.
	void ServerStart(); // Starts the server loop to monitor and handle client connections and data.
	void HandleClientData(int client_fd); //Handles incoming data from a client, processes it, and prepares responses to be sent back to the client.
	void SendDataToClient(int client_fd); //Sends data from the client's write buffer to the client socket, and handles any errors
	~Server();

	private:
		std::map<int, Client>   _clients;
		std::vector<pollfd>	 _pollfds;// List of pollfd structures for monitoring server and client sockets
		int					 _port;
		int					 _server_fd; 
		std::string			 _password;

		void AddNewClient(); //Accepts a new client connection, sets it to non-blocking mode, and adds it to _pollfds for monitoring.
		void DisconnectClient(size_t i); //Handles client disconnection by closing the socket, removing the client from the clients map, and erasing the corresponding pollfd from _pollfds.
		void catchClientData(int client_fd, char* buf, int b); //Catches and processes data from a specific client. This function will read the data received from the client, process it according to the server's logic (e.g., parsing commands, updating client state), and prepare any necessary responses to be sent back to the client.
		void writeToClient(Client &client); //Sends data from the client's write buffer to the client socket, and handles any errors that may occur during sending. This function will be responsible for actually transmitting data to the client based on what has been prepared in the client's write buffer.
		void processNewData(Client &client); // Process complete commands in the client's read buffer and execute appropriate actions based on the commands received. This function will handle command parsing, authentication checks, and response generation for the client.


		// Helper functions to check client states and events
		bool currentClienthasData(int i); //Checks if the client at index i in _pollfds has data to read (POLLIN event).
		ServerEnum newData(int i); //Checks if there is new data to read from the client at index i in _pollfds.
		ServerEnum newDataIs(int i); //Determines if the new data is from a new client or an existing client.
		ServerEnum clientReadRdy(int i); //Checks if the client at index i in _pollfds is ready for writing (POLLOUT event).
		bool isBuffEmpty(int i); //Checks if the write buffer for the client at index i in _pollfds is empty, which indicates whether there is data to send to the client.
		bool errorOccured(int i); //Checks if there was an error or hang-up event for the client at index		
		ServerEnum readClientsData(int i); //Reads data from the client at index i in _pollfds and processes it. This function will handle the actual reading of data from the client's socket, and then call catchClientData to process the received data.
		void updatePollEvents(); //Updates the events field in _pollfds for each client based on whether there is data to read or write. This function will be called before polling to ensure that the server is monitoring the correct events for each client. 

		//rest of canonical form locked!
		Server(const Server& other);
		Server& operator=(const Server& other);
};

#endif