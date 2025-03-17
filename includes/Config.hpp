/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cviegas <cviegas@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/03/17 23:00:00 by cviegas           #+#    #+#             */
/*   Updated: 2025/03/17 23:01:40 by cviegas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>
#include <vector>
#include <map>

struct Route
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

struct Server
{
	std::string host;
	int port;
	std::vector<std::string> serverNames;
	bool isDefault;
	std::map<int, std::string> errorPages;
	size_t maxBodySize;
	std::map<std::string, Route> routes;
};

class Config
{
public:
	Config();
	Config(const Config &other);
	Config &operator=(const Config &other);
	~Config();

	bool loadFromFile(const std::string &filename);

	// Getters
	const std::vector<Server> &getServers() const;

private:
	std::vector<Server> _servers;
};

#endif // CONFIG_HPP
