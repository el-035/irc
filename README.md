*This project has been created as part of the 42 curriculum by \<efittant\> & \<dbogovic\>.*

# ft_irc

## Description

**ft_irc** is a simplified implementation of an Internet Relay Chat (IRC) server written in C++ and optimised to work on LinuxOS.

The goal of this project is to understand and implement a real network protocol using TCP sockets, while handling multiple clients simultaneously in a non-blocking way. The server follows the IRC protocol (RFC 1459/2812) and allows clients to connect using an IRC client (e.g. irssi).

Through this project, we explore:

- Socket programming
- Network protocols (IRC)
- Multiplexing (poll)
- Client authentication
- Channel management
- Message broadcasting
- C++ object-oriented design

This project is developed following the C++98 standard.


### Features

- TCP server using IPv4
- Non-blocking I/O
- Multiple simultaneous clients
- Password authentication
- Nickname and username handling
- Channel creation and management
- Operator privileges
- Private messages
- Basic IRC commands:
  - PASS
  - NICK
  - USER
  - JOIN
  - PRIVMSG
  - KICK
  - INVITE
  - TOPIC
  - MODE


### How It Works

The server:

1. Creates a TCP socket
2. Binds to a given port
3. Listens for incoming connections
4. Uses poll() (or select/epoll depending on your implementation) to manage multiple clients
5. Parses IRC commands
6. Maintains internal state:
   - Connected clients
   - Channels
   - Operators
   - Modes

---

## Instructions

### Requirements

- C++ compiler (clang++ / g++)
- Make
- Unix-based system (Linux / macOS)
  - (please note that this version is not yet fully optimized to work on macOS, implement fcntl(fd, F_SETFL, O_NONBLOCK); in Server.cpp init())

### Compile and run

Compile the project using `make`:
 - after generating the executable run it/start server using ./ircserv \<port\> \<password\>
 - password is restricted to 32 characters maximum, whitespace not allowed
 - port choice is restricted: 1024 - 65535 (to avoid needing to have root access or to avoid using privileged ports)

### Example

- To connect IRC client irssi to server first make sure you have it installed on your device. *irssi --help* to check.
- Run it from a different terminal using *"irssi"* which will open its interface. Once there you can type **/connect localhost \<port\> \<pass\>**
- for more features and or commands of irrsi please reffer to [irssi website](https://irssi.org/)

## Resources

Following are resources used in creation of this 42 school project:

- [irssi manual](https://irssi.org/documentation/manual/)
- [C++ reference](https://cppreference.com/)https://cppreference.com/
- linux man pages (eg. man irssi)
- [ChatGPT](https://chatgpt.com/) - used as explanatory tool and in limited function as debugging tool
