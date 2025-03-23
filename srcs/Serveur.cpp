/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Serveur.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ple-guya <ple-guya@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/20 23:48:15 by ple-guya          #+#    #+#             */
/*   Updated: 2025/03/23 18:17:04 by ple-guya         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Serveur.hpp"
#include <cstring>

Serveur::Serveur()
{
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
        throw (std::runtime_error("failed create socket"));
        
    int opt = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0)
        throw (std::runtime_error("failed to set socket reuse port option"));
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
        throw (std::runtime_error("failed to set socket options"));
        
    adress.sin_family = AF_INET;
    adress.sin_addr.s_addr = INADDR_ANY;
    adress.sin_port = htons(DEFAULT_PORT);
    adrLen = sizeof(adress);
    
    if (bind(fd, (sockaddr *)&adress, adrLen) < 0)
        throw (std::runtime_error(std::string("failed bind server: ") + strerror(errno)));
    if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1)
        throw (std::runtime_error("failed to get nonBlock socket"));
    if (listen(fd, 10) < 0)
        throw(std::runtime_error("can't listen serveur"));
    std::cout << "Serveur en écoute sur le port " << DEFAULT_PORT << std::endl;
}

// Serveur::Serveur(const Serveur &src) : fd(src.fd), adress(src.adress)
// {    
//     fd = socket(AF_INET, SOCK_STREAM, 0);
//     if (fd < 0)
//         throw (std::runtime_error("failed create socket"));
//     int opt = 1;
//     if (setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0)
//         throw (std::runtime_error("failed to set socket reuse port option"));
//     if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
//         throw (std::runtime_error("failed to set socket options"));
        
//     adress.sin_family = AF_INET;
//     adress.sin_addr.s_addr = INADDR_ANY;
//     adress.sin_port = htons(DEFAULT_PORT);
//     adrLen = sizeof(adress);
//     if (bind(fd, (sockaddr *)&adress, adrLen) < 0)
//         throw (std::runtime_error(std::string("failed bind server: ") + strerror(errno)));
//     if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1)
//         throw (std::runtime_error("failed to get nonBlock socket"));
//     if (listen(fd, 10) < 0)
//         throw(std::runtime_error("can't listen"));
//     std::cout << "Serveur en écoute sur le port " << DEFAULT_PORT << std::endl;
// }

Serveur::~Serveur() 
{
    close(fd);  
}

int     Serveur::getfd() const
{
    return (this->fd);
}

sockaddr_in   Serveur::getAddress()const
{
    return(this->adress);
}