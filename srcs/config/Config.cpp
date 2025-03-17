/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cviegas <cviegas@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/03/17 23:00:00 by cviegas           #+#    #+#             */
/*   Updated: 2025/03/17 23:23:44 by cviegas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/Config.hpp"
#include <fstream>
#include <iostream>

Config::Config() {}

Config::Config(const Config &other)
{
	*this = other;
}

Config &Config::operator=(const Config &other)
{
	if (this != &other)
	{
		_servers = other._servers;
	}
	return *this;
}

Config::~Config() {}

bool Config::loadFromFile(const std::string &filename)
{
	std::ifstream file(filename.c_str());
	if (!file.is_open())
	{
		std::cerr << "Error: Cannot open config file: " << filename << std::endl;
		return false;
	}

	// The actual parsing will be implemented later
	// For now, just return true to indicate success

	file.close();
	return true;
}

const std::vector<Server> &Config::getServers() const
{
	return _servers;
}
