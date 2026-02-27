/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: efittant <efittant@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/25 17:15:57 by dbogovic          #+#    #+#             */
/*   Updated: 2026/02/27 15:42:05 by efittant         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <sys/socket.h>
#include <list>

class Client
{
	protected:
		int				fd;
		std::string		nickname;
		std::string		username;
		std::string		realname;
		bool			authenticated;
		bool			registered;
		std::string		write_buffer;
		std::string		read_buffer;
		
	public:
		Client(const Client& other);
		Client& operator=(const Client& other);
		Client();
		Client(int f);
		~Client();

		std::list<std::string> channelsJoined;
		int getFd() const;
		std::string getNickname() const;
		std::string getUsername() const;
		std::string getRealname() const;
		bool getRegistered() const;
		void setRegistered(bool reg);
		bool isAuthenticated() const;
		std::string& getWriteBuffer();
		std::string& getReadBuffer();
		void setNickname(const std::string& name);
		void setUsername(const std::string& name);
		void setRealname(const std::string& name);
		void setAuthenticated(bool auth);
		void appendToWriteBuffer(const std::string& data);
		void appendToReadBuffer(const std::string& data);
		void clearReadBuffer();
};

#endif
