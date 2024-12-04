#include "../includes/Channel.hpp"

Channel::Channel() {}

Channel::Channel( Channel const &src ) {
	_name = src._name;
	_user = src._user;
	_invites = src._invites;
	_topic = src._topic;
	_topic_nick = src._topic_nick;
	_topic_restrict = src._topic_restrict;
	_invite_only = src._invite_only;
	_pass = src._pass;
	_limit = src._limit;
}

Channel::Channel( std::string name, std::string owner ) : _name(name) {
	_user[owner] = true;
	_topic_restrict = true;
	_invite_only = false;
	_limit = 0;
}

Channel::~Channel() {}


std::string Channel::getName() {
	return _name;
}

void Channel::addUser( std::string toAdd ) {
	if (_user.find(toAdd) == _user.end())
		_user[toAdd] = false;
}

void Channel::removeUser( std::string toRemove ) {
	if (_user.find(toRemove) != _user.end())
		_user.erase(_user.find(toRemove));
}

void Channel::addOp( std::string toAdd ) {
	if (_user.empty())
		return ;
	
	if (_user.find(toAdd) != _user.end())
		_user[toAdd] = true;
}

void Channel::removeOp( std::string toRemove ) {
	if (_user.empty())
		return ;
	
	if (_user.find(toRemove) != _user.end())
		_user[toRemove] = false;
}

int Channel::getUserAmt() {
	return _user.size();
}

std::string Channel::getUserList() {
	if (_user.empty())
		return "";

	std::string out;
	for (std::map<std::string, bool>::iterator it = _user.begin(); it != _user.end(); it++) {
		if (it->second)
			out += "@";
		out += it->first;
		out += " ";
	}
	return out.substr(0, out.size() - 1);
}

std::map<std::string, bool> &Channel::getUsers() {
	return _user;
}

void Channel::setTopic( std::string topic, std::string nick )  {
	_topic = topic;
	_topic_nick = nick;
}

std::string Channel::getTopic() {
	return _topic;
}

std::string Channel::getTopicNick() {
	return _topic_nick;
}

bool Channel::isTopicRestricted() {
	return _topic_restrict;
}

void Channel::setTopicRestricted( bool state ) {
	_topic_restrict = state;
}

bool Channel::isInviteOnly() {
	return _invite_only;
}

void Channel::setInviteOnly( bool state ) {
	_invite_only = state;
}

void Channel::addInvited( std::string nickname ) {
	_invites.push_back(nickname);
}

void Channel::popInvited( std::string nickname ) {
	for (std::vector<std::string>::iterator it = _invites.begin(); it != _invites.end(); it++) {
		if (*it == nickname) {
			_invites.erase(it);
			if (isNickInVector(_invites, nickname))
				popInvited(nickname);
			return ;
		}
	}
}

bool Channel::isInvited( std::string nickname ) {
	return isNickInVector(_invites, nickname);
}

int Channel::getLimit() {
	return _limit;
}

void Channel::setLimit( int limit ) {
	_limit = limit;
}

std::string Channel::getPass() {
	return _pass;
}

void Channel::setPass( std::string pass ) {
	_pass = pass;
}


bool Channel::isUserInChannel( std::string nickname ) {
	if (_user.find(nickname) == _user.end())
		return false;
	return true;
}

bool Channel::isUserOp( std::string nickname ) {
	if (_user.empty())
		return false;

	if (_user.find(nickname) != _user.end())
		return _user[nickname];

	return false;
}


Channel &Channel::operator=( Channel const &src ) {
	_name = src._name;
	_user = src._user;
	_topic = src._topic;
	_topic_nick = src._topic_nick;
	_topic_restrict = src._topic_restrict;
	_invite_only = src._invite_only;
	_pass = src._pass;
	_limit = src._limit;
	return *this;
}