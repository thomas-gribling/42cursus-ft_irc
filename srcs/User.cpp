#include "../includes/User.hpp"

User::User() : _user_fd(0), _pass_req(false), _nick_req(false), _user_req(false) {}

User::User( User const &src ) {
	_username = src._username;
	_nickname = src._nickname;
	_user_fd = src._user_fd;
	_pass_req = src._pass_req;
	_nick_req = src._nick_req;
	_user_req = src._user_req;
}

User::User( int user_fd ) : _user_fd(user_fd), _pass_req(false), _nick_req(false), _user_req(false) {}

User::~User() {}


int	User::getSocket() {
	return (_user_fd);
}

std::string	User::getUsername() {
	return (_username);
}

std::string	User::getNickname() {
	return (_nickname);
}

void User::setNickname( std::string nickname ) {
	_nickname = nickname;
}

void User::setUsername( std::string username ) {
	_username = username;
}

void User::setSocket( int socket ) {
	_user_fd = socket;
}

bool User::isConnected() {
	return _nick_req && _user_req;
}

bool User::isPassSet() {
	return _pass_req;
}

void User::setReqState( char req ) {
	if (req == 'p')
		_pass_req = true;
	if (req == 'n')
		_nick_req = true;
	if (req == 'u')
		_user_req = true;
}


bool User::joinChannel( std::vector<Channel> &channels, std::string name ) {
	std::vector<Channel>::iterator it;
	for (it = channels.begin(); it != channels.end(); it++) {
		if (it->getName() == name && !it->isUserInChannel(_nickname)) {
			if (!it->isInviteOnly() || (it->isInviteOnly() && it->isInvited(_nickname))) {
				it->popInvited(_nickname);
				it->addUser(_nickname);
				return true;
			}
			else if (it->isInviteOnly() && !it->isInvited(_nickname))
				return false;
		}
	}
	// Channel was not found
	if (it == channels.end()) {
		channels.push_back(Channel(name, _nickname));
		return true;
	}
	return false;
}

void User::quitChannel( std::vector<Channel> &channels, std::string name ) {
	std::vector<Channel>::iterator it;
	for (it = channels.begin(); it != channels.end(); it++) {
		if (it->getName() == name) {
			it->removeUser(_nickname);
			if (!it->getUserAmt()) {
				std::vector<Channel>::iterator tmp = it - 1;
				channels.erase(it);
				it = tmp;
			}
		}
	}
}


User &User::operator=( User const &src ) {
	_username = src._username;
	_nickname = src._nickname;
	_user_fd = src._user_fd;
	_pass_req = src._pass_req;
	_nick_req = src._nick_req;
	_user_req = src._user_req;
	return *this;
}
