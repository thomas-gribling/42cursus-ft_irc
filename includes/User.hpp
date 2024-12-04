#pragma once

#include "main.hpp"

class Channel;

class User {
	private:
		int _user_fd;
		std::string _username;
		std::string _nickname;
		bool _pass_req;
		bool _nick_req;
		bool _user_req;
	
	public:
		User();
		User( User const &src );
		User( int user_fd );
		~User();

		int	getSocket();
		std::string	getUsername();
		std::string	getNickname();
		void setNickname( std::string nickname );
		void setUsername( std::string username );
		void setSocket( int socket );
		bool isConnected();
		bool isPassSet();
		void setReqState( char req );

		bool joinChannel( std::vector<Channel> &channels, std::string name );
		void quitChannel( std::vector<Channel> &channels, std::string name );

		User &operator=( User const &src );
};