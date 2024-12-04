#include "../includes/main.hpp"

void Server::botardAnswer( std::string channel, std::string msg ) {
	for (size_t i = 0; i < msg.size(); i++) {
		msg[i] = tolower(msg[i]);
	}

	std::string word = getLastWord(msg);
	if (word.substr(0, 4) == "pour" && word.size() > 5) {
			word = word.substr(5);
	}
	if (word[word.size() - 1] == '\n')
		word = word.substr(0, word.size() - 1);
	if (word[0] == ':')
		word = word.substr(1);

	if (word == "quoi" || word == "koi" || word == "kwa" || word == "qoi" || word == "qwa" || word == "keuwa" || word == "qeuwa" || word == "queuwa")
		msg = "feur";
	else if (word == "oui" || word == "ui")
		msg = "stiti";
	else if (word == "ouai" || word == "ouais" || word == "oue" || word == "ue")
		msg = "stern";
	else if (word == "non" || word == "nn")
		msg = "bril";
	else if (word == "si")
		msg = "tron";
	else
		return ;
	msg = ":botard PRIVMSG " + channel + " :" + msg + "\n";
	sendAll(msg);
}

void Server::botarate( std::string channel ) {
	std::string msg;
	
	std::string one[] = {"ccadoret", "Xavier Niel", "un mec", "acasanov", "le pingouin linux", "le correcteur"};
	std::string two[] = {"un bar", "42 perpignan", "le quartier Saint-Jacques", "la gare de Perpignan", "dans les chiottes"};
	std::string three[] = {"tgriblin", "son pere", "une pute", "tmoirand", "un handicape", "gakarbouch"};
	std::string four[] = {"defouraille", "enleve son fut", "se desape", "sort l'attirail", "devoile le conduit"};
	std::string five[] = {"l'encule", "lui pete le cul", "le defonce", "lui rearrange les tripes"};

	msg = "C'est " + one[rand() % 6] + " qui entre dans " + two[rand() % 5] + ", il voit " + three[rand() % 6] + ", il " + four[rand() % 5] + " et il " + five[rand() % 4];
	msg = ":botard PRIVMSG " + channel + " :" + msg + "\n";
	sendAll(msg);
}