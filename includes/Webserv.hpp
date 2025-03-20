/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ple-guya <ple-guya@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/05 19:04:35 by ple-guya          #+#    #+#             */
/*   Updated: 2025/03/20 21:26:06 by ple-guya         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERVER_HPP
#define WEBSERVER_HPP

#include "Client.hpp"
#include <iostream>
#include <string>
#include <fstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <map>
#include <vector>
#include <poll.h>

#define DEFAULT_PORT 8080

struct t_serveur
{
    pollfd fd;
};

class Webserv
{
    private:
        int                     sockfd;
        sockaddr_in             adress;
        socklen_t               adrLen;
        std::vector<pollfd>     fds;
        std::map<int, t_serveur>  serveurs;
        std::map<int, Client>   clients;

    public:
        Webserv();
        Webserv(char * &config_filename);
        Webserv(const Webserv &cpy);
        Webserv &operator=(const Webserv &rhs);
        ~Webserv();

        void    acceptNewClient();
};

#endif