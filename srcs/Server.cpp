/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ple-guya <ple-guya@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/20 22:43:31 by cviegas           #+#    #+#             */
/*   Updated: 2025/06/23 14:34:59 by ple-guya         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "Webserv.hpp"

Server::Server() : port(DEFAULT_PORT), isDefault(false), maxBodySize(15000000), _configNode(NULL)
{
	fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0)
		throw std::runtime_error("failed create socket");

	int opt = 1;
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0)
		throw std::runtime_error("failed to set socket reuse port option");

	adress.sin_family = AF_INET;
	adress.sin_addr.s_addr = INADDR_ANY;
	adress.sin_port = htons(DEFAULT_PORT);
	adrLen = sizeof(adress);

	if (bind(fd, (sockaddr *)&adress, adrLen) < 0)
		throw std::runtime_error(std::string("failed bind server: ") + strerror(errno));
	if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1)
		throw std::runtime_error("failed to get nonBlock socket");
	if (listen(fd, 10) < 0)
		throw std::runtime_error("can't listen serveur");
	std::cout << "Serveur en écoute sur le port " << DEFAULT_PORT << std::endl;
}

Server::Server(const Server &other)
	{ *this = other; }

Server &Server::operator=(const Server &other)
{
	if (this != &other)
	{
		fd = other.fd;
		host = other.host;
		port = other.port;
		serverNames = other.serverNames;
		isDefault = other.isDefault;
		errorPages = other.errorPages;
		maxBodySize = other.maxBodySize;
		_configNode = other._configNode; // Copy config node pointer
	}
	return *this;
}

Server::Server(ConfigNode *configNode) : isDefault(false), maxBodySize(15000000)
{
	fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0)
		throw std::runtime_error("failed create socket");

	int opt = 1;
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0)
		throw std::runtime_error("failed to set socket reuse port option");
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
		throw std::runtime_error("failed to set socket options");

	adress.sin_family = AF_INET;
	adress.sin_addr.s_addr = INADDR_ANY;
	_configNode = configNode; // Store the config node
	port = DEFAULT_PORT;
	std::map<std::string, std::vector<std::string> >::const_iterator it = configNode->directives.find("listen");
	if (it != configNode->directives.end() && !it->second.empty())
	{
		std::stringstream portStream(it->second[0]);
		if (!(portStream >> port))
		{
			std::cout << "Warning: Invalid port value, using default port " << DEFAULT_PORT << std::endl;
			port = DEFAULT_PORT;
		}
	}
	adress.sin_port = htons(port);

	adrLen = sizeof(adress);

	if (bind(fd, (sockaddr *)&adress, adrLen) < 0)
		throw std::runtime_error(std::string("failed bind server: ") + strerror(errno));
	if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1)
		throw std::runtime_error("failed to get nonBlock socket");
	if (listen(fd, 10) < 0)
		throw std::runtime_error("can't listen serveur");
	std::cout << "Serveur en écoute sur le port " << port << std::endl;
}

int Server::getfd() const
	{ return this->fd; }

sockaddr_in Server::getAddress() const
	{ return this->adress; }

ConfigNode* Server::getConfigNode() const
	{ return this->_configNode; }

Server::~Server()
{
}

/* ******************** */
/* Gestion des sessions */
/* ******************** */

std::pair<std::string, sessionData>    Server::createSession(std::string username)
{
    sessionData user;
    std::string sessionID;
    time_t      creationTime = time(NULL);
    
    unsigned int seed = static_cast<unsigned int>(creationTime);
    int random = rand_r(&seed);

    sessionID = "sess_" + intToString(creationTime) + intToString(random);
	
    user.userName = username;
	user.max_age = 3600;
    user.createdAt = creationTime;
    user.expiresAt = creationTime + user.max_age;
	 
    this->activeSessions.insert(std::make_pair(sessionID, user));

	return std::make_pair(sessionID, user);
}

bool	Server::isActiveSession(std::string const &id)
{

	std::cout << "===check activeSessions map======" << std::endl;
	std::map<std::string, sessionData>::iterator itt = activeSessions.begin();
	for (; itt != activeSessions.end(); itt++)
		std::cout << itt->first << " = " << itt->second.userName << std::endl;
		
	std::map<std::string, sessionData>::iterator it = activeSessions.find(id);
	if (it == activeSessions.end())
		return false;

	time_t now = time(NULL);
	if (now > it->second.expiresAt) {
		activeSessions.erase(id);
		return false;
	}
	return true;
}

void	Server::deleteSession(std::string const &id)
{
	std::map<std::string, sessionData>::iterator it = activeSessions.find(id);
	if (it == activeSessions.end())
		return;
	activeSessions.erase(id);
}

std::string intToString(int value)
{
    std::stringstream ss;
    ss << value;
	std::string result = ss.str();
	
    return result;
}
