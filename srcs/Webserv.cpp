/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ple-guya <ple-guya@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/05 19:04:30 by ple-guya          #+#    #+#             */
/*   Updated: 2025/03/20 21:23:58 by ple-guya         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Webserv.hpp"

Webserv::Webserv() 
{
    sockfd = socket(AF_INET, SOCK_STREAM, NULL);
    if (sockfd < 0);
        throw (std::runtime_error("failed create socket"));
    adress.sin_family = AF_INET;
    adress.sin_addr.s_addr = INADDR_ANY;
    adress.sin_port = htons(DEFAULT_PORT);
    adrLen = sizeof(adress);
    if (bind(sockfd, (sockaddr *)&adress, adrLen) < 1)
        throw (std::runtime_error("failed bind server"));
    if (listen(sockfd, 10) < 0)
        throw(std::runtime_error("can't listen"));
    if (fcntl(sockfd, F_SETFL, O_NONBLOCK) == -1)
        throw (std::runtime_error("failed to get nonBlock socket"));
}

// Webserv::Webserv(char * &config_filename)
// {
//     // std::ifstream config_file(config_filename);
    
//     // if (!config_file.is_open())
//     //     throw (std::ifstream::failure("file don't exist or can't access"));
    
// }

Webserv &Webserv::operator=(const Webserv &rhs)
{
    if (&rhs != this)
        return (*this);
    return (*this);
}

Webserv::~Webserv()
{}

void    Webserv::acceptNewClient()
{
    int     client_fd = accept(sockfd, NULL, NULL);
    if (client_fd < 0)
        throw (std::runtime_error("Error : accept client failed"));
    
    Client newclient(client_fd);

    pollfd newfd;
    newfd.fd = client_fd;
    newfd.events = POLLIN;

    newfd.push_back(newfd);
    clients[client_fd] = newclient;
}
