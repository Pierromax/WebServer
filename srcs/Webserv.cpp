/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ple-guya <ple-guya@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/05 19:04:30 by ple-guya          #+#    #+#             */
/*   Updated: 2025/03/23 18:53:32 by ple-guya         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Webserv.hpp"
#include <sstream>

Webserv::Webserv() 
{
    Serveur *newserv = new Serveur();
    
    pollfd newfd;
    newfd.fd = newserv->getfd();
    newfd.events = POLLIN;

    fds.push_back(newfd);
    serveurs[newfd.fd] = newserv;
    std::cout << "Serveur ajouté à la map avec fd = " << newfd.fd << std::endl;
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
{
    
    for (std::map<int, Serveur*>::iterator it = serveurs.begin(); it != serveurs.end(); ++it)
        delete it->second;
    for (std::map<int, Client*>::iterator it = clients.begin(); it != clients.end(); ++it)
        delete it->second;
    for (std::vector<pollfd>::iterator it = fds.begin(); it != fds.end(); ++it)
        close(it->fd);
}

void    Webserv::acceptNewClient(const Serveur &connect)
{
    std::cout << "Tentative d'accepter un nouveau client depuis le serveur fd = " << connect.getfd() << std::endl;

    sockaddr_in serveuraddr = connect.getAddress();
    socklen_t addrlen = sizeof(serveuraddr);
    int     client_fd = accept(connect.getfd(), (sockaddr *)&serveuraddr, &addrlen);
    if (client_fd < 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            std::cout << "Aucune connexion client en attente" << std::endl;
            return;  // Simplement retourner sans erreur
        }
        // Pour les autres erreurs, lancer une exception
        std::cerr << "Erreur accept(): " << strerror(errno) << std::endl;
        throw (std::runtime_error("Error: accept client failed"));
    }
    
    std::cout << "Nouveau client accepté avec fd = " << client_fd << std::endl;

    Client *newclient = new Client(client_fd);

    pollfd newfd;
    newfd.fd = client_fd;
    newfd.events = POLLIN;

    fds.push_back(newfd);
    clients[client_fd] = newclient;
    std::cout << "client ajouter avec fd = " << client_fd << std::endl ;
}

//run all server
void    Webserv::run()
{
    while (true)
    {
        int ret = poll(fds.data(), fds.size(), -1);
        if (ret < 0)
            throw(std::runtime_error("poll failed"));

        
        for (std::vector<pollfd>::iterator it = fds.begin(); it != fds.end(); it++)
        {
            fds.size();
            if (serveurs.count(it->fd))
            {
                if (it->revents & POLLIN)
                    acceptNewClient(*serveurs[it->fd]);
            }
            if (clients.count(it->fd) && it->revents & POLLIN)
            {
                Request req(it->fd);
                
            } 
        }
    }
}
