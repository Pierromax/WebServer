/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cviegas <cviegas@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/20 22:28:37 by cviegas           #+#    #+#             */
/*   Updated: 2025/06/23 16:57:30 by cviegas          ###   ########.fr       */
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
#include <ctime>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdexcept>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <sstream>

// Forward declaration
struct ConfigNode;

struct sessionData
{
    std::string 	userName;
	unsigned int 	max_age;
    time_t      	createdAt;
    time_t      	expiresAt;
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
		size_t 		maxBodySize;
		ConfigNode* getConfigNode() const;
		bool matchesServerName(const std::string& hostHeader) const;
		void addServerName(const std::string& name); // Getter for the config node

		std::pair<std::string, sessionData> createSession(std::string username);
		bool	isActiveSession(std::string const &id);
		void	deleteSession(std::string const &id);
		bool	isLoginRequired(){return _authRequired;};
	
	private:
		int fd;
		sockaddr_in adress;
		socklen_t adrLen;
		ConfigNode* _configNode;
		bool _authRequired;
		std::map<std::string, sessionData> activeSessions;
};
std::string intToString(int value);

#endif // SERVER_HPP