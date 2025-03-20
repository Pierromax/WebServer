/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ple-guya <ple-guya@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/05 19:04:33 by ple-guya          #+#    #+#             */
/*   Updated: 2025/03/19 14:07:52 by ple-guya         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"

Request::Request(int client_fd) : fd(client_fd)
{
    std::string buffer[1024];
    recv(client_fd, buffer, sizeof(buffer), NULL);
}


Request::Request(const Request &cpy) : headers(cpy.headers)
{}

Request &Request::operator=(const Request &rhs)
{
    if (&rhs != this)
        return (*this);
    return (*this);
}

Request::~Request()
{}