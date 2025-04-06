/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cezou <cezou@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/20 22:43:31 by cviegas           #+#    #+#             */
/*   Updated: 2025/04/06 20:09:06 by cezou            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/Server.hpp"

Server::Server() : port(80), isDefault(false), maxBodySize(15000000)
{
}

Server::Server(const Server &other)
{
	*this = other;
}

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
	}
	return *this;
}

Server::~Server()
{
}