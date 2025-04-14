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

Client::Client() : fd(-1)
{
    memset(&address, 0, sizeof(address));
}

Client::Client(int &client_fd) : fd(client_fd)
{
    memset(&address, 0, sizeof(address));
    
    // Vérifier que le descripteur est valide
    if (fd >= 0) {
        int flags = fcntl(client_fd, F_GETFL, 0);
        if (flags != -1) {
            fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);
        }
    }
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


