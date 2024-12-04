#include "../includes/Server.hpp"

Server::Server() : _port(0), _server_fd(0) {}

Server::Server( Server const &src ) : _port(src._port), _password(src._password), _server_fd(src._server_fd) {}

Server::Server( std::string name, int port, std::string password ) : _name(name), _port(port), _password(password), _server_fd(0) {}

Server::~Server() {}


Server &Server::operator=( Server const &src ) {
	_port = src._port;
	_password = src._password;
	_server_fd = src._server_fd;
	return *this;
}


int Server::getPort() {
	return _port;
}

std::string Server::getPassword() {
	return _password;
}

int Server::getSocket() {
	return _server_fd;
}

struct sockaddr_in &Server::getAddress() {
	return _address;
}

User &Server::getUser( int i ) {
	return _user[i];
}

int Server::getUserAmt() {
	return _user.size();
}

int Server::getUserFromSocket( int socket ) {
	for (size_t i = 0; i < _user.size(); i++) {
		if (_user[i].getSocket() == socket)
			return i;
	}
	return 0;
}

int Server::getSocketFromNickname( std::string nickname ) {
	std::vector<User>::iterator it;
	for (it = _user.begin(); it != _user.end(); it++) {
		if (it->getNickname() == nickname)
			return it->getSocket();
	}
	return 0;
}

Channel *Server::getChannel( std::string name ) {
	if (_channels.empty())
		return NULL;

	for (std::vector<Channel>::iterator it = _channels.begin(); it != _channels.end(); it++) {
		if (it->getName() == name)
			return &(*it);
	}
	return NULL;
}

bool Server::isRunning() {
	return _running;
}


int Server::start() {
	// Create socket (fd)
	_server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (!_server_fd) {
		std::cerr << "[SERVER] Socket failed!" << std::endl;
		return 1;
	}
	// Set its options
	int opt = 1;
	if (setsockopt(_server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
		std::cerr << "[SERVER] Setsockopt failed!" << std::endl;
		close(_server_fd);
		return 1;
	}
	_address.sin_family = AF_INET;
	_address.sin_addr.s_addr = INADDR_ANY;
	_address.sin_port = htons(_port);
	// Associate socket to address and port
	if (bind(_server_fd, (struct sockaddr *)&_address, sizeof(_address)) < 0) {
		std::cerr << "[SERVER] Bind failed!" << std::endl;
		close(_server_fd);
		return 1;
	}
	// Listen socket
	if (listen(_server_fd, 3) < 0) {
		std::cerr << "[SERVER] Listen failed!" << std::endl;
		close(_server_fd);
		return 1;
	}
	_running = true;
	std::cout << "[SERVER] Server \"" << _name << "\" running on port " << _port << "." << std::endl;
	return 0;
}

void Server::stop() {
	_running = false;
}

void Server::connectUser( std::vector<pollfd> &new_pollfds ) {
	int addrlen = sizeof(_address);
	int new_socket = accept(_server_fd, (struct sockaddr *)&_address, (socklen_t*)&addrlen);
	if (new_socket < 0) {
		std::cerr << "[SERVER] Client listen failed" << std::endl;
		return ;
	}
	_user.push_back(User(new_socket));
	std::cout << "[SERVER] Negotiating with client #" << _user.size() << ", fd=" << new_socket << std::endl;

	pollfd new_client = {new_socket, POLLIN, 0};
	new_pollfds.push_back(new_client);
}

void Server::manageUser( std::vector<pollfd> &pollfds, std::vector<pollfd>::iterator &it ) {
	size_t buffer_size = 4096;
	char buff[buffer_size];

	while (_buffer[it->fd].find('\n') == _buffer[it->fd].npos) {
		int rc = recv(it->fd, buff, buffer_size - 1, 0);
		if (rc < 0) {
			User &curr = _user[getUserFromSocket(it->fd)];
			std::cerr << "[" << it->fd << "<" << curr.getNickname() << ">] recv Error, Client disconnected" << std::endl;
			it = deleteUser(pollfds, it);
			_buffer[it->fd] = "";
			return ;
		}
		buff[rc] = '\0';
		_buffer[it->fd] += buff;
		if (_buffer[it->fd].find('\n') != _buffer[it->fd].npos)
			break;
		if (rc == 0) {
			User &curr = _user[getUserFromSocket(it->fd)];
			std::cout << "[" << it->fd << "<" << curr.getNickname() << ">] Client disconnected" << std::endl;
			it = deleteUser(pollfds, it);
			_buffer[it->fd] = "";
			return ;
		}
	}
	std::string msg = _buffer[it->fd];
	_buffer[it->fd] = "";
	User &curr = _user[getUserFromSocket(it->fd)];

	std::cout << "[" << it->fd << "<" << curr.getNickname() << ">] " << msg;
	if (curr.isConnected())
		parseMessage(it, pollfds, msg);
	else
		treatRequests(msg, curr);
}

std::vector<pollfd>::iterator Server::deleteUser( std::vector<pollfd> &pollfds, std::vector<pollfd>::iterator &it ) {
	if (!_channels.empty()) {
		for (std::vector<Channel>::iterator itc = _channels.begin(); itc != _channels.end(); itc++) {
			itc->removeUser(_user[getUserFromSocket(it->fd)].getNickname());
			if (!itc->getUserAmt()) {
				std::vector<Channel>::iterator tmp = itc - 1;
				_channels.erase(itc);
				itc = tmp;
				continue;
			}
		}
	}
	_user.erase(_user.begin() + getUserFromSocket(it->fd));
	close(it->fd);
	std::vector<pollfd>::iterator its = it - 1;
	pollfds.erase(it);
	return its;
}

void Server::joinChannelClient( std::vector<pollfd>::iterator &it, std::string name ) {
	if (!getChannel(name))
		return ;
	
	std::string msg;
	User &curr = _user[getUserFromSocket(it->fd)];

	msg = ":" + curr.getNickname() + "!~" + curr.getNickname() + "@localhost JOIN " + name + "\r\n";
	sendAll(msg);
	msg = ":" + _name + " 353 " + curr.getNickname()+ " = " + name + " :" + getChannel(name)->getUserList() + "\r\n";
	sendAll(msg);

	if (getChannel(name)->getTopic() != "") {
		msg = ":" + getChannel(name)->getTopicNick() + " TOPIC " + name + " :" + getChannel(name)->getTopic() + "\r\n";
		send(it->fd, msg.c_str(), msg.size(), 0);
	}
	
}


void Server::sendAll( std::string buff, int ignore ) {
	for (size_t i = 0; i < _user.size(); i++) {
		if (_user[i].getSocket() != ignore)
			send(_user[i].getSocket(), buff.c_str(), buff.size(), 0);
	}
}


const char *Server::ReqInvalidPass::what() const throw() {
	return "Invalid password.";
}

const char *Server::ReqNickSpace::what() const throw() {
	return "Nickname cannot contain spaces.";
}

const char *Server::ReqNickExists::what() const throw() {
	return "Nickname already in use, try a different one.";
}