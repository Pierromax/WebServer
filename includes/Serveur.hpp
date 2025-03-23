/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Serveur.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ple-guya <ple-guya@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/20 23:48:12 by ple-guya          #+#    #+#             */
/*   Updated: 2025/03/23 18:08:30 by ple-guya         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVEUR_HPP
#define SERVEUR_HPP

#include <unistd.h>
#include <iostream>
#include <string>
#include <fstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <map>
#include <vector>
#include <poll.h>
#include <fcntl.h>

#define DEFAULT_PORT 8080

class Serveur
{
    private:
        int				fd;
        sockaddr_in		adress;
        socklen_t		adrLen;
        
    public:
        Serveur();
        ~Serveur();
        Serveur(const Serveur &src); 
        
        int 		getfd() const;
        sockaddr_in getAddress() const;
};

#endif