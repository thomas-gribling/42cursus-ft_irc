#include "../includes/main.hpp"

void Server::treatRequests( std::string msg, User &curr ) {
	std::string content;

	if (msg.substr(0, 6) == "PASS :") {
		content = msg.substr(msg.find(":") + 1);
		if (content[content.size() - 1] == '\n')
			content = content.substr(0, content.size() - 1);
	
		if (content != _password)
			throw ReqInvalidPass();

		curr.setReqState('p');
	}
	if (msg.substr(0, 5) == "NICK ") {
		content = msg.substr(msg.find(" ") + 1);
		if (content[content.size() - 1] == '\n')
			content = content.substr(0, content.size() - 1);

		if (content.find(" ") != std::string::npos)
			throw ReqNickSpace();
		for (std::vector<User>::iterator it = _user.begin(); it != _user.end(); it++) {
			if (it->getNickname() == content)
				throw ReqNickExists();
		}

		curr.setNickname(content);
		curr.setReqState('n');
	}
	if (msg.substr(0, 5) == "USER ") {
		if (msg.find(":") == std::string::npos)
			return ;
		
		content = msg.substr(msg.find(":") + 1);
		if (content[content.size() - 1] == '\n')
			content = content.substr(0, content.size() - 1);

		curr.setUsername(content);
		curr.setReqState('u');
	}
	if (curr.isConnected()) {
		if (!curr.isPassSet())
			throw ReqInvalidPass();

		msg = "Welcome Successfully connected to " + _name + " !\r\n";
		send(curr.getSocket(), msg.c_str(), msg.size(), 0);
		std::cout << "[SERVER] User successfully connected: " << curr.getNickname() << ", " << curr.getUsername() << ", " << curr.getSocket() << std::endl;
	}
}

void Server::parseMessage( std::vector<pollfd>::iterator &it, std::vector<pollfd> &pollfds, std::string msg ) {
	User &curr = _user[getUserFromSocket(it->fd)];
	std::string curr_user = curr.getNickname();
	std::string channel_name, target;
	std::vector<std::string> args;

	if (msg.substr(0, 5) == "JOIN ") {
		args = splitStr(msg, ' ');
		channel_name = args[1];

		if (channel_name[channel_name.size() - 1] == '\n')
			channel_name = channel_name.substr(0, channel_name.size() - 1);
		if (args.size() > 2) {
			target = args[2];
			if (target[target.size() - 1] == '\n')
				target = target.substr(0, target.size() - 1);
		}
		if (getChannel(channel_name) && getChannel(channel_name)->getPass() != "" && (args.size() < 3 || getChannel(channel_name)->getPass() != target)) {
			msg = "Error Cannot join " + channel_name  + ": The channel is password-protected and either a wrong or no password was given\r\n";
			send(it->fd, msg.c_str(), msg.size(), 0);
			return ;
		}
		else if (getChannel(channel_name) && getChannel(channel_name)->getLimit() && getChannel(channel_name)->getUserAmt() >= getChannel(channel_name)->getLimit()) {
			msg = "471 " + curr_user + " " + channel_name + " Cannot join channel (+l) - channel is full, try again later\r\n";
			send(it->fd, msg.c_str(), msg.size(), 0);
			return ;
		}
		if (curr.joinChannel(_channels, channel_name))
			joinChannelClient(it, channel_name);
		else {
			msg = "473 " + curr_user + " " + channel_name + " Cannot join channel (+i) - you must be invited\r\n";
			send(it->fd, msg.c_str(), msg.size(), 0);
		}
	}

	else if (msg.substr(0, 6) == "QUIT :") { // Leaving server
		it = deleteUser(pollfds, it);
		msg = ":" + curr_user + "!~" + curr_user + "@localhost " + msg;
		sendAll(msg);
	}

	else if (msg.substr(0, 6) == "PART #") { // Leaving a channel
		args = splitStr(msg, ' ');
		channel_name = args[1];

		if (channel_name[channel_name.size() - 1] == '\n')
			channel_name = channel_name.substr(0, channel_name.size() - 1);
		if (!getChannel(channel_name))
			return ;
		curr.quitChannel(_channels, channel_name);
		msg = ":" + curr_user + "!~" + curr_user + "@localhost " + msg;
		sendAll(msg);
	}

	else if (msg.substr(0, 8) == "PRIVMSG ") {
		args = splitStr(msg, ' ');
		channel_name = args[1];

		if (!getChannel(channel_name) || !getChannel(channel_name)->isUserInChannel(curr_user)) {
			if (getChannel(channel_name) && getChannel(channel_name)->getPass() != "") {
				msg = "Error Cannot join " + channel_name  + ": The channel is password-protected and either a wrong or no password was given\r\n";
				send(it->fd, msg.c_str(), msg.size(), 0);
				return ;
			}
			if (getChannel(channel_name) && getChannel(channel_name)->getLimit() && getChannel(channel_name)->getUserAmt() >= getChannel(channel_name)->getLimit()) {
				msg = "471 " + curr_user + " " + channel_name + " Cannot join channel (+l) - channel is full, try again later\r\n";
				send(it->fd, msg.c_str(), msg.size(), 0);
				return ;
			}
			if (curr.joinChannel(_channels, channel_name)) {
				joinChannelClient(it, channel_name);
			}
			else {
				msg = "404 " + curr_user + " " + channel_name + " Cannot send to nick/channel\r\n";
				send(it->fd, msg.c_str(), msg.size(), 0);
				return ;
			}
		}
		msg = ":" + curr_user + " " + msg;
		sendAll(msg, it->fd);
		botardAnswer(channel_name, msg);

	}

	else if (msg.substr(0, 6) == "KICK #") {
		args = splitStr(msg, ' ');
		channel_name = args[1];
		target = args[2];

		if (!getChannel(channel_name))
			return ;

		if (!getChannel(channel_name)->isUserInChannel(target)) {
			msg = "Error " + target + ": No such nick/channel.\r\n";
			send(it->fd, msg.c_str(), msg.size(), 0);
		}
		else if (getChannel(channel_name)->isUserOp(curr_user)) {
			User &target_user = _user[getUserFromSocket(getSocketFromNickname(target))];
			target_user.quitChannel(_channels, channel_name);
			msg = ":" + curr_user + "!~" + curr_user + "@localhost " + msg;
			sendAll(msg);
		}
		else {
			msg = "Error You need to be a channel operator in " + channel_name + " to do that.\r\n";
			send(it->fd, msg.c_str(), msg.size(), 0);
		}

	}

	else if (msg.substr(0, 7) == "TOPIC #") {
		args = splitStr(msg, ' ');
		channel_name = args[1];
		target = "";
		if (args.size() > 2) {
			for (std::vector<std::string>::iterator its = args.begin() + 2; its != args.end(); its++) {
				target += *its;
				target += " ";
			}
		}
		if (target != "" && (target[target.size() - 1] == ' ' || target[target.size() - 1] == '\n'))
			target = target.substr(0, target.size() - 1);
		if (target != "" && (target[target.size() - 1] == ' ' || target[target.size() - 1] == '\n'))
			target = target.substr(0, target.size() - 1);
		if (target != "" && target[0] == ':')
			target = target.substr(1);
		if (channel_name[channel_name.size() - 1] == '\n')
			channel_name = channel_name.substr(0, channel_name.size() - 1);
		
		if (!getChannel(channel_name))
			return ;

		if (args.size() < 3) {
			if (getChannel(channel_name)->getTopic() != "")
				msg = ":" + getChannel(channel_name)->getTopicNick() + " 332 " + curr_user + " " + channel_name + " :" + getChannel(channel_name)->getTopic() + "\r\n";
			else
				msg = "TOPIC The channel " + channel_name + " has no topic set.\r\n";
			send(it->fd, msg.c_str(), msg.size(), 0);
		}
		else {
			if (getChannel(channel_name)->isUserOp(curr_user) || !getChannel(channel_name)->isTopicRestricted()) {
				getChannel(channel_name)->setTopic(target, curr_user + "!~" + curr_user + "@localhost");
				msg = ":" + curr_user + "!~" + curr_user + "@localhost " + msg;
				sendAll(msg);
			}
			else {
				msg = "Error You need to be a channel operator in " + channel_name + " to do that.\r\n";
				send(it->fd, msg.c_str(), msg.size(), 0);
			}
		}	
	}
	
	else if (msg.substr(0, 7) == "INVITE ") {
		args = splitStr(msg, ' ');
		target = args[1];
		channel_name = args[2];
		if (channel_name[channel_name.size() - 1] == '\n')
			channel_name = channel_name.substr(0, channel_name.size() - 1);
		if (!getSocketFromNickname(target)) {
			msg = "Error " + target + ": No such nick/channel.\r\n";
			send(it->fd, msg.c_str(), msg.size(), 0);
		}
		else if (!getChannel(channel_name)) {
			msg = "Error " + channel_name + ": No such nick/channel.\r\n";
			send(it->fd, msg.c_str(), msg.size(), 0);
		}
		else {
			if (getChannel(channel_name)->isUserInChannel(target)) {
				msg = "443 " + curr_user + " " + target + " " + channel_name + " is already on channel\r\n";
				send(it->fd, msg.c_str(), msg.size(), 0);
			}
			else {
				getChannel(channel_name)->addInvited(target);
				msg = ":" + curr_user + "!~" + curr_user + "@localhost " + msg;
				send(getSocketFromNickname(target), msg.c_str(), msg.size(), 0);
			}
		}
	}

	else if (toLowerStr(msg.substr(0, 5)) == "mode ") {
		bool positive = true;
		args = splitStr(msg, ' ');

		if (args.size() == 1) {
			msg = "Error MODE: This command requires more parameters.\r\n";
			send(it->fd, msg.c_str(), msg.size(), 0);
			return ;
		}

		channel_name = args[1];
		if (channel_name[channel_name.size() - 1] == '\n')
			channel_name = channel_name.substr(0, channel_name.size() - 1);

		if (!getChannel(channel_name)) {
			msg = "Error " + channel_name + ": No such channel.\r\n";
			send(it->fd, msg.c_str(), msg.size(), 0);
		}
		else if (args.size() < 3) {
			std::string modes = "+n";
			std::string modes_args = "";
			if (getChannel(channel_name)->isInviteOnly())
				modes += "i";
			if (getChannel(channel_name)->isTopicRestricted())
				modes += "t";
			if (getChannel(channel_name)->getPass() != "")
				modes += "k";
			if (getChannel(channel_name)->getLimit()) {
				modes += "l ";
				modes += getChannel(channel_name)->getLimit();
			}
			msg = ":" + _name + " 324 " + curr.getNickname()+ " " + channel_name + " " + modes + "\r\n";
			send(it->fd, msg.c_str(), msg.size(), 0);
		}
		else {
			if (!getChannel(channel_name)->isUserOp(curr_user)) {
				msg = "Error You need to be a channel operator in " + channel_name + " to do that.\r\n";
				send(it->fd, msg.c_str(), msg.size(), 0);
				return ;
			}
			std::string flags = args[2];
			if (flags[flags.size() - 1] == '\n')
			flags = flags.substr(0, flags.size() - 1);

			size_t i = 0;
			while (flags[i] == '+' || flags[i] == '-') {
				if (flags[i] == '+')
					positive = true;
				else
					positive = false;
				i++;
			}
			size_t curr_arg = 3;
			std::string tmp_arg;
			while (i < flags.size()) {
				if (flags[i] != 'i' && flags[i] != 't' && flags[i] != 'k' && flags[i] != 'o' && flags[i] != 'l' && flags[i] != 'b') {
					msg = "472 " + curr_user + " ";
					msg += flags[i];
					msg += " is an unknown mode char to me\r\n";
					send(it->fd, msg.c_str(), msg.size(), 0);
					i++;
					continue;
				}
				if (flags[i] == 'b') {
					msg = ":" + _name + " 368 " + curr.getNickname()+ " " + channel_name + " :End of ban list\r\n";
					send(it->fd, msg.c_str(), msg.size(), 0);
				}
				if (flags[i] == 'i') {
					if (positive && !getChannel(channel_name)->isInviteOnly()) {
						msg = ":" + curr_user + "!~" + curr_user + "@localhost MODE " + channel_name + " +o " + tmp_arg + "\r\n";
						sendAll(msg);
					}
					if (!positive && getChannel(channel_name)->isInviteOnly()) {
						msg = ":" + curr_user + "!~" + curr_user + "@localhost MODE " + channel_name + " -o " + tmp_arg + "\r\n";
						sendAll(msg);
					}
					getChannel(channel_name)->setInviteOnly(positive);
				}
				if (flags[i] == 't') {
					if (positive && !getChannel(channel_name)->isTopicRestricted()) {
						msg = ":" + curr_user + "!~" + curr_user + "@localhost MODE " + channel_name + " +t " + tmp_arg + "\r\n";
						sendAll(msg);
					}
					if (!positive && getChannel(channel_name)->isTopicRestricted()) {
						msg = ":" + curr_user + "!~" + curr_user + "@localhost MODE " + channel_name + " -t " + tmp_arg + "\r\n";
						sendAll(msg);
					}
					getChannel(channel_name)->setTopicRestricted(positive);
				}
				if (flags[i] == 'k') {
					if (args.size() > curr_arg) {
						tmp_arg = args[curr_arg++];
						if (tmp_arg[tmp_arg.size() - 1] == '\n')
							tmp_arg = tmp_arg.substr(0, tmp_arg.size() - 1);
						if (getChannel(channel_name)->getPass() == "" && positive) {
							msg = ":" + curr_user + "!~" + curr_user + "@localhost MODE " + channel_name + " +k " + tmp_arg + "\r\n";
							sendAll(msg);
							getChannel(channel_name)->setPass(tmp_arg);
						}
						if (getChannel(channel_name)->getPass() != "" && !positive) {
							msg = ":" + curr_user + "!~" + curr_user + "@localhost MODE " + channel_name + " -k " + tmp_arg + "\r\n";
							sendAll(msg);
							getChannel(channel_name)->setPass(tmp_arg);
						}
					}
				}
				if (flags[i] == 'o') {
					if (args.size() > curr_arg) {
						tmp_arg = args[curr_arg++];
						if (tmp_arg[tmp_arg.size() - 1] == '\n')
							tmp_arg = tmp_arg.substr(0, tmp_arg.size() - 1);
						if (!getChannel(channel_name)->isUserInChannel(tmp_arg)) {
							msg = "441 " + curr_user + " " + tmp_arg + " " + channel_name + " They aren't on that channel\r\n";
							send(it->fd, msg.c_str(), msg.size(), 0);
						}
						else {
							if (positive) {
								if (!getChannel(channel_name)->isUserOp(tmp_arg)) {
									msg = ":" + curr_user + "!~" + curr_user + "@localhost MODE " + channel_name + " +o " + tmp_arg + "\r\n";
									sendAll(msg);
								}
								getChannel(channel_name)->addOp(tmp_arg);
							}
							else {
								if (getChannel(channel_name)->isUserOp(tmp_arg)) {
									msg = ":" + curr_user + "!~" + curr_user + "@localhost MODE " + channel_name + " -o " + tmp_arg + "\r\n";
									sendAll(msg);
								}
								getChannel(channel_name)->removeOp(tmp_arg);
							}
						}

					}
				}
				if (flags[i] == 'l') {
					if (!positive && getChannel(channel_name)->getLimit()) {
						getChannel(channel_name)->setLimit(0);
						msg = ":" + curr_user + "!~" + curr_user + "@localhost MODE " + channel_name + " -l\r\n";
						sendAll(msg);
					}
					if (args.size() <= curr_arg && positive) {
						msg = "Error MODE: This command requires more parameters.\r\n";
						send(it->fd, msg.c_str(), msg.size(), 0);
					}
					else if (args.size() > curr_arg && positive) {
						tmp_arg = args[curr_arg++];
						if (tmp_arg[tmp_arg.size() - 1] == '\n')
							tmp_arg = tmp_arg.substr(0, tmp_arg.size() - 1);
						bool tmp = false;
						for (size_t j = 0; j < tmp_arg.size(); j++) {
							if (!isdigit(tmp_arg[j]))
								tmp = true;
						}
						int limit = atoi(tmp_arg.c_str());
						if (limit >= 1 && limit <= std::numeric_limits<int>::max()) {
							getChannel(channel_name)->setLimit(limit);
							msg = ":" + curr_user + "!~" + curr_user + "@localhost MODE " + channel_name + " +l " + tmp_arg + "\r\n";
							sendAll(msg);
						}
					}
					else
						curr_arg++;
				}
				i++;
			}
		}
	}
	else if (msg.substr(0, 9) == "shutdown ") {
		if (curr_user == _user[0].getNickname())
			stop();
		else {
			msg = "Error You must be the original user to use this command.\n";
			send(it->fd, msg.c_str(), msg.size(), 0);
		}
	}
	else if (msg.substr(0, 9) == "botarate ") {
		args = splitStr(msg, ' ');
		channel_name = args[1];
		if (channel_name[channel_name.size() - 1] == '\n')
		channel_name = channel_name.substr(0, channel_name.size() - 1);

		if (args.size() < 2) {
			msg = "Error MODE: This command requires more parameters.\r\n";
			send(it->fd, msg.c_str(), msg.size(), 0);
		}
		else if (!getChannel(channel_name)) {
			msg = "Error " + channel_name + ": No such channel.\r\n";
			send(it->fd, msg.c_str(), msg.size(), 0);
		}
		else
			botarate(channel_name);
	}
}