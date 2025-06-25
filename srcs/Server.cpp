/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cviegas <cviegas@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/20 22:43:31 by cviegas           #+#    #+#             */
/*   Updated: 2025/06/23 20:15:36 by cviegas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "Webserv.hpp"

Server::Server() : port(DEFAULT_PORT), isDefault(false), maxBodySize(15000000), _configNode(NULL), _ownsConfigNode(false)
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
	d_cout << "Serveur en écoute sur le port " << DEFAULT_PORT << std::endl;
}

Server::Server(const Server &other)
	{ *this = other; }

Server &Server::operator=(const Server &other)
{
	if (this != &other)
	{
		if (_ownsConfigNode && _configNode)
			delete _configNode;
		fd = other.fd;
		host = other.host;
		port = other.port;
		serverNames = other.serverNames;
		isDefault = other.isDefault;
		errorPages = other.errorPages;
		maxBodySize = other.maxBodySize;
		_configNode = other._configNode;
		_ownsConfigNode = false; // Copy doesn't own the config
	}
	return *this;
}

Server::Server(ConfigNode *configNode) : isDefault(false), maxBodySize(15000000), _ownsConfigNode(true)
{
	fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0)
		throw std::runtime_error("failed create socket");

	int opt = 1;
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0)
		throw std::runtime_error("failed to set socket reuse port option");
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
		throw std::runtime_error("failed to set socket options");

	_configNode = configNode;
	port = DEFAULT_PORT;
	host = "0.0.0.0";
	
	if (!configNode->listenPairs.empty())
	{
		host = configNode->listenPairs[0].first;
		port = configNode->listenPairs[0].second;
		if (host.empty())
			host = "0.0.0.0";
	}

	std::map<std::string, std::vector<std::string> >::const_iterator serverNameIt = 
		configNode->directives.find("server_name");
	if (serverNameIt != configNode->directives.end())
	{
		for (size_t i = 0; i < serverNameIt->second.size(); ++i)
			serverNames.push_back(serverNameIt->second[i]);
	}

	adress.sin_family = AF_INET;
	if (host == "0.0.0.0" || host.empty())
		adress.sin_addr.s_addr = INADDR_ANY;
	else
		adress.sin_addr.s_addr = inet_addr(host.c_str());
	adress.sin_port = htons(port);

	adrLen = sizeof(adress);

	if (bind(fd, (sockaddr *)&adress, adrLen) < 0)
		throw std::runtime_error(std::string("failed bind server: ") + strerror(errno));
	if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1)
		throw std::runtime_error("failed to get nonBlock socket");
	if (listen(fd, 10) < 0)
		throw std::runtime_error("can't listen serveur");
	d_cout << "Serveur en écoute sur " << host << ":" << port << std::endl;
}

int Server::getfd() const
	{ return this->fd; }

sockaddr_in Server::getAddress() const
	{ return this->adress; }

ConfigNode* Server::getConfigNode() const
	{ return this->_configNode; }

Server::~Server()
{
	close(fd);
	delete _configNode;
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

/**
 * @brief Checks if the server matches the given Host header
 * @param hostHeader The Host header from the HTTP request
 * @return true if this server should handle the request
 */
bool Server::matchesServerName(const std::string& hostHeader) const
{
	if (serverNames.empty())
		return true;
	
	std::string hostname = hostHeader;
	size_t colonPos = hostHeader.find(':');
	if (colonPos != std::string::npos)
		hostname = hostHeader.substr(0, colonPos);
	
	for (size_t i = 0; i < serverNames.size(); ++i)
	{
		if (serverNames[i] == hostname)
			return true;
	}
	return false;
}

/**
 * @brief Adds a server name to this server
 * @param name Server name to add
 */
void Server::addServerName(const std::string& name)
{
	serverNames.push_back(name);
}
