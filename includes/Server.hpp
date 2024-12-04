#pragma once

#include "main.hpp"

class User;
class Channel;

class Server {
	private:
		std::string _name;
		int _port;
		std::string _password;
		int _server_fd;
		struct sockaddr_in _address;
		std::vector<User> _user;
		std::vector<Channel> _channels;
		bool _running;
		std::map<int, std::string> _buffer;
	
	public:
		Server();
		Server( Server const &src );
		Server( std::string name, int port, std::string password );
		~Server();

		Server &operator=( Server const &src );

		int getPort();
		std::string getPassword();
		int getSocket();
		struct sockaddr_in &getAddress();
		User &getUser( int i );
		int getUserAmt();
		int getUserFromSocket( int socket );
		int getSocketFromNickname( std::string nickname );
		Channel *getChannel( std::string name );
		bool isRunning();

		int start();
		void stop();
		void connectUser( std::vector<pollfd> &new_pollfds );
		void manageUser( std::vector<pollfd> &pollfds, std::vector<pollfd>::iterator &it );
		std::vector<pollfd>::iterator deleteUser( std::vector<pollfd> &pollfds, std::vector<pollfd>::iterator &it );
		void joinChannelClient( std::vector<pollfd>::iterator &it, std::string name );
		void treatRequests( std::string msg, User &curr );
		void parseMessage( std::vector<pollfd>::iterator &it, std::vector<pollfd> &pollfds, std::string msg );

		void sendAll( std::string buff, int ignore = 0 );

		void botardAnswer( std::string channel, std::string msg );
		void botarate( std::string channel );

		class ReqInvalidPass : public std::exception {
			virtual const char *what() const throw();
		};
		class ReqNickSpace : public std::exception {
			virtual const char *what() const throw();
		};
		class ReqNickExists : public std::exception {
			virtual const char *what() const throw();
		};
};