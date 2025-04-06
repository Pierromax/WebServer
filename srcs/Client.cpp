/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cezou <cezou@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/19 12:32:35 by ple-guya          #+#    #+#             */
/*   Updated: 2025/04/06 23:55:14 by cezou            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

Client::Client() : fd(-1)
{
    memset(&address, 0, sizeof(address));
}

Client::Client(int &client_fd) : fd(client_fd)
{
    int flags = fcntl(client_fd, F_SETFL, O_NONBLOCK) < 0;
    fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);

}

Client::Client(const Client &cpy) : fd(cpy.fd) {}

Client &Client::operator=(const Client &rhs)
{
    if (&rhs != this)
        return (*this);
    return (*this);
}

Client::~Client()
{
    close(fd);
}
