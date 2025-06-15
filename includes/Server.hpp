/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cviegas <cviegas@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/20 22:28:37 by cviegas           #+#    #+#             */
<<<<<<<<< Temporary merge branch 1
/*   Updated: 2025/06/04 09:55:18 by cezou            ###   ########.fr       */
=========
/*   Updated: 2025/06/09 15:03:59 by ple-guya         ###   ########.fr       */
>>>>>>>>> Temporary merge branch 2
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
	ConfigNode* getConfigNode() const; // Getter for the config node

private:
	int fd;
	sockaddr_in adress;
	socklen_t adrLen;
	ConfigNode* _configNode; // Store the config node used to create this server
};

#endif // SERVER_HPP