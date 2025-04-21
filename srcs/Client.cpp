/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ple-guya <ple-guya@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/19 12:32:35 by ple-guya          #+#    #+#             */
/*   Updated: 2025/04/14 13:31:15 by ple-guya         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"
#include "Server.hpp" // Include Server header

Client::Client() : fd(-1), _server(NULL)
	{ memset(&address, 0, sizeof(address)); }

// Updated constructor
Client::Client(int client_fd, Server* server) : fd(client_fd), _server(server)
{
    memset(&address, 0, sizeof(address));
    if (fd >= 0)
    {
        int flags = fcntl(client_fd, F_GETFL, 0);
        if (flags != -1)
            fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);
    }
}

Client::Client(const Client &cpy) : fd(cpy.fd), _server(cpy._server) {}

Client &Client::operator=(const Client &rhs)
{
    if (&rhs != this)
    {
        fd = rhs.fd;
        address = rhs.address;
        _server = rhs._server;
    }
    return (*this);
}

Client::~Client()
{
    if (fd >= 0)
        close(fd);
}

Server* Client::getServer() const
	{ return _server; }


