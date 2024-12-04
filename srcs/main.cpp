#include "../includes/main.hpp"

int main( int ac, char **av ) {
	if (ac != 3) {
		std::cerr << "Usage:  ./ircserv <port> <password>" << std::endl;
		return 1;
	}

	for (int i = 0; av[1][i]; i++){
		if (!isdigit(av[1][i])) {
			std::cerr << "Error: invalid port." << std::endl;
			return 1;
		}
	}
	int port = atoi(av[1]);
	if (port < 1)
		std::cerr << "Error: port number too small: " << port << std::endl;
	if (port > 65535)
		std::cerr << "Error: port number too large: " << port << std::endl;
	if (port < 1 || port > 65535)
		return 1;

	Server server("PortHub", port, av[2]);
	if (server.start())
		return 1;

	int error = 0;

	std::vector<pollfd> pollfds;
	pollfd tmp = {server.getSocket(), POLLIN, 0};
	pollfds.push_back(tmp);

	std::string input;
	while (server.isRunning()) { /* === MAIN LOOP === */
		std::vector<pollfd> new_pollfds;

		if (poll(pollfds.data(), pollfds.size(), -1) < 0) {
			error = 1;
			server.stop();
			break;
		}

		std::vector<pollfd>::iterator it;
		for (it = pollfds.begin(); it != pollfds.end(); it++) {
			if (it->revents & POLLIN) {
				if (it->fd == server.getSocket())
					server.connectUser(new_pollfds);
				else {
					try {
						server.manageUser(pollfds, it);
					}
					catch (std::exception &e) {
						std::cerr << "[" << it->fd << "] Error: " << e.what() << std::endl;
						std::string msg = e.what();
						msg = "error Unable to connect to server: " + msg + "\r\n";
						send(it->fd, msg.c_str(), msg.size(), 0);
						it = server.deleteUser(pollfds, it);
						continue;
					}
				}
			}
			else if (it->revents & POLLERR) {}
		}
		pollfds.insert(pollfds.end(), new_pollfds.begin(), new_pollfds.end());
	}

	for (int i = 0; i < server.getUserAmt(); i++) {
		close(server.getUser(i).getSocket());
	}
	close(server.getSocket());

	return error;
}