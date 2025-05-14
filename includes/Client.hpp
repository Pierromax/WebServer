/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ple-guya <ple-guya@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/19 12:27:22 by ple-guya          #+#    #+#             */
/*   Updated: 2025/05/14 21:07:00 by ple-guya         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
#define CLIENT_HPP


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
class Response;
class Request;

class Client
{
    private:
        int         fd;
        sockaddr_in address;
        Server*     _server; // Pointer to the server that accepted this client
        bool        isReady;
        
    public:
        Request     *request;
        Response    *response;
        
        // Client();
        // Client(const Client &cpy);
        // Client &operator=(const Client &rhs);
        Client(int clientFD, Server* server); // Updated constructor
        ~Client();
        
        Server* getServer() const;
        void    prepareRequest();
        void    prepareResponse();
        bool    isRequestValid() const;
        void    sendResponse() const;

};


#endif