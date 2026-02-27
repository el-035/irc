/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dbogovic <dbogovic@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/25 17:16:09 by dbogovic          #+#    #+#             */
/*   Updated: 2026/02/26 16:31:46 by dbogovic         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Client.hpp"
#include <unistd.h>

Client::Client() : fd(-1), state(CONNECTED) {}
Client::Client(int f) : fd(f), state(CONNECTED) {}

int Client::getFd() const { return fd; }
std::string Client::getNickname() const { return nickname; }
std::string Client::getUsername() const { return username; }
std::string Client::getRealname() const { return realname; }
std::string& Client::getWriteBuffer() { return write_buffer; }
std::string& Client::getReadBuffer()  { return read_buffer; }
void Client::setNickname(const std::string& name) { nickname = name; }
void Client::setUsername(const std::string& name) { username = name; }
void Client::setRealname(const std::string& name) { realname = name; }
void Client::appendToWriteBuffer(const std::string& data) { write_buffer += data; }
void Client::appendToReadBuffer(const std::string& data) { read_buffer += data; }
void Client::clearReadBuffer() {read_buffer.clear(); }
ClientState Client::getState() const {return state;}
void Client::changeState(ClientState status) {state = status;}
void Client::clearWriteBuffer(void) {write_buffer.erase();}


Client::Client(const Client& other)
		: fd(other.fd),
			nickname(other.nickname),
			username(other.username),
			realname(other.realname),
			state(other.state),
			write_buffer(other.write_buffer),
			read_buffer(other.read_buffer)
{}

Client& Client::operator=(const Client& other)
{
		if (this != &other)
		{
			fd = other.fd;
			nickname = other.nickname;
			username = other.username;
			realname = other.realname;
			state = other.state;
			write_buffer = other.write_buffer;
			read_buffer = other.read_buffer;
		}
		return *this;
}

Client::~Client()
{}
