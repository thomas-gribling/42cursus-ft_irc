#pragma once

// C++ Headers
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <vector>
#include <map>
#include <algorithm>
#include <limits>
// C Headers
#include <unistd.h>     // close()
#include <poll.h>       // poll()
#include <netinet/in.h> // sockaddr_in
// Local headers
#include "Server.hpp"
#include "User.hpp"
#include "Channel.hpp"

std::string toLowerStr( std::string s );
std::vector<std::string> splitStr( std::string s, char c );
bool isNickInVector( std::vector<std::string> v, std::string nick );
std::string getLastWord( std::string msg );