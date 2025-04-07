/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cezou <cezou@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/20 22:28:37 by cviegas           #+#    #+#             */
/*   Updated: 2025/04/07 18:02:18 by cezou            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <vector>
#include <map>
#include <cstddef> // for size_t
#include <poll.h>  // for pollfd
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdexcept>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <sstream>

// Forward declaration
struct ConfigNode;

struct t_Route
{
	std::vector<std::string> allowedMethods;
	std::string redirect;
	std::string root;
	bool directoryListing;
	std::string defaultFile;
	std::string cgiExtension;
	bool allowUploads;
	std::string uploadDirectory;
};

class Server
{
public:
	Server();
	Server(const Server &other);
	Server &operator=(const Server &other);
	~Server();

	Server(ConfigNode *configNode); // New constructor

	int getfd() const;
	sockaddr_in getAddress() const;

	std::string host;
	int port;
	std::vector<std::string> serverNames;
	bool isDefault;
	std::map<int, std::string> errorPages;
	size_t maxBodySize;
	std::map<std::string, t_Route> routes;

private:
	int fd;
	sockaddr_in adress;
	socklen_t adrLen;
};

#endif // SERVER_HPP