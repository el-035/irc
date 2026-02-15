//ADD HEADER LATER
//ADD HEADER LATER
//ADD HEADER LATER
//ADD HEADER LATER


#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <sys/socket.h>

class Client
{
    private:
       // Client& operator=(const Client& other); //! this cannot stay pricate! somebody uses it!
    protected:
        int             fd;
        std::string     nickname;
        std::string     username;
        std::string     realname;
        bool            authenticated;
        bool            registered;
        std::string     write_buffer; // Data waiting to be sent (POLLOUT)
        std::string     read_buffer;  // Data received but not yet a full command (\n)

    public:
        Client();
        Client(int f);
        // Getters and setters for the member variables
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
        void clearWriteBuffer();
};

#endif