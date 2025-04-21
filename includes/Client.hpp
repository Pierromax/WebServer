/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ple-guya <ple-guya@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/19 12:27:22 by ple-guya          #+#    #+#             */
/*   Updated: 2025/04/14 13:31:55 by ple-guya         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "Request.hpp"
#include "Response.hpp"
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>

// Forward declaration
class Server;

class Client
{
    private:
        int         fd;
        sockaddr_in address;
        Server*     _server; // Pointer to the server that accepted this client

    public:
        Client();
        Client(int clientFD, Server* server); // Updated constructor
        Client(const Client &cpy);
        Client &operator=(const Client &rhs);
        ~Client();

        Server* getServer() const; // Getter for the server
        
};

#endif