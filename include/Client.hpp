/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dbogovic <dbogovic@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/25 17:15:57 by dbogovic          #+#    #+#             */
/*   Updated: 2026/02/26 16:31:06 by dbogovic         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
#define CLIENT_HPP

enum ClientState
{
	CONNECTED,
	AUTHENTICATED,
	REGISTERED,
	DISCONNECTING
};
#include <string>
#include <sys/socket.h>

class Client
{
	protected:
		int				fd;
		std::string		nickname;
		std::string		username;
		std::string		realname;
		ClientState		state;
		std::string		write_buffer;
		std::string		read_buffer;

	public:
		Client(const Client& other);
		Client& operator=(const Client& other);
		Client();
		Client(int f);
		~Client();

		int getFd() const;
		std::string getNickname() const;
		std::string getUsername() const;
		std::string getRealname() const;
		ClientState getState() const;
		void changeState(ClientState status);
		std::string& getWriteBuffer();
		std::string& getReadBuffer();
		void setNickname(const std::string& name);
		void setUsername(const std::string& name);
		void setRealname(const std::string& name);
		void appendToWriteBuffer(const std::string& data);
		void appendToReadBuffer(const std::string& data);
		void clearReadBuffer();
		void clearWriteBuffer();
};

#endif
