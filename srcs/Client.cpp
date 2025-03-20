/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ple-guya <ple-guya@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/19 12:32:35 by ple-guya          #+#    #+#             */
/*   Updated: 2025/03/19 16:55:38 by ple-guya         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

Client::Client(int &client_fd) : fd(client_fd)
{
    if (fcntl(client_fd, F_SETFL, O_NONBLOCK) < 0)
        std::cerr << "failed setting nonblocking socket";
}

Client::Client(const Client &cpy) : fd(cpy.fd) {}

Client &Client::operator=(const Client &rhs)
{
    if (&rhs != this)
        return (*this);
    return (*this);
}

Client::~Client()
{}