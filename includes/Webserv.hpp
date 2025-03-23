/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ple-guya <ple-guya@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/05 19:04:35 by ple-guya          #+#    #+#             */
/*   Updated: 2025/03/23 20:37:49 by ple-guya         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERVER_HPP
#define WEBSERVER_HPP

#include "Client.hpp"
#include "Serveur.hpp"
#include <iostream>
#include <string>
#include <fstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <map>
#include <vector>
#include <poll.h>
#include <cstring>

class Webserv
{
    private:
        std::vector<pollfd>         fds;
        std::map<int, Serveur*>     serveurs;
        std::map<int, Client*>      clients;

    public:
        Webserv();
       // Webserv(char * &config_filename);
        //Webserv(const Webserv &cpy);
        Webserv &operator=(const Webserv &rhs);
        ~Webserv();

        void    acceptNewClient(const Serveur &server);
        void    run();
};


#endif