/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cezou <cezou@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/20 22:43:31 by cviegas           #+#    #+#             */
/*   Updated: 2025/04/07 17:02:52 by cezou            ###   ########.fr       */
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
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
		throw std::runtime_error("failed to set socket options");

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
		routes = other.routes;
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