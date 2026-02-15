//ADD HEADER LATER
//ADD HEADER LATER
//ADD HEADER LATER

#include "../include/Client.hpp"


Client::Client() : fd(-1), authenticated(false), registered(false) {}
Client::Client(int f) : fd(f), authenticated(false), registered(false) {}

int Client::getFd() const { return fd; }
std::string Client::getNickname() const { return nickname; }
std::string Client::getUsername() const { return username; }
std::string Client::getRealname() const { return realname; }
bool Client::getRegistered() const { return registered; }
void Client::setRegistered(bool reg) { registered = reg; }
bool Client::isAuthenticated() const { return authenticated; }
std::string& Client::getWriteBuffer() { return write_buffer; }
std::string& Client::getReadBuffer()  { return read_buffer; } 
void Client::setNickname(const std::string& name) { nickname = name; }
void Client::setUsername(const std::string& name) { username = name; }
void Client::setRealname(const std::string& name) { realname = name; }
void Client::setAuthenticated(bool auth) { authenticated = auth; }
void Client::appendToWriteBuffer(const std::string& data) { write_buffer += data; }
void Client::appendToReadBuffer(const std::string& data) { read_buffer += data; }
void Client::clearReadBuffer() { read_buffer.clear(); }
void Client::clearWriteBuffer() { write_buffer.clear(); }