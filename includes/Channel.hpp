#pragma once

#include "main.hpp"

class Channel {
	private:
		std::string _name;
		std::map<std::string, bool> _user;
		std::vector<std::string> _invites;
		std::string _topic;
		std::string _topic_nick;
		bool _topic_restrict;
		bool _invite_only;
		std::string _pass;
		int _limit;

	public:
		Channel();
		Channel( Channel const &src );
		Channel( std::string name, std::string owner );
		~Channel();

		std::string getName();
		void addUser( std::string toAdd );
		void removeUser( std::string toRemove );
		void addOp( std::string toAdd );
		void removeOp( std::string toRemove );
		int getUserAmt();
		std::string getUserList();
		std::map<std::string, bool> &getUsers();
		void setTopic( std::string topic, std::string nick );
		std::string getTopic();
		std::string getTopicNick();
		bool isTopicRestricted();
		void setTopicRestricted( bool state );
		bool isInviteOnly();
		void setInviteOnly( bool state );
		void addInvited( std::string nickname );
		void popInvited( std::string nickname );
		bool isInvited( std::string nickname );
		int getLimit();
		void setLimit( int limit );
		std::string getPass();
		void setPass( std::string pass );

		bool isUserInChannel( std::string nickname );
		bool isUserOp( std::string nickname );

		Channel &operator=( Channel const &src );
};