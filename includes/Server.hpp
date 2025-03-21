/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cviegas <cviegas@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/20 22:28:37 by cviegas           #+#    #+#             */
/*   Updated: 2025/03/21 02:30:08 by cviegas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <vector>
#include <map>
#include <cstddef> // for size_t
#include <poll.h>  // for pollfd

// Forward declare any classes from Webserv.hpp that might be needed
// class Webserv;

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

	pollfd fd;
	std::string host;
	int port;
	std::vector<std::string> serverNames;
	bool isDefault;
	std::map<int, std::string> errorPages;
	size_t maxBodySize;
	std::map<std::string, t_Route> routes;
};

#endif // SERVER_HPP