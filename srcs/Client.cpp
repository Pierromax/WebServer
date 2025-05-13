/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ple-guya <ple-guya@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/19 12:32:35 by ple-guya          #+#    #+#             */
/*   Updated: 2025/05/13 18:38:57 by ple-guya         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"
#include "Server.hpp" // Include Server header
#include "Request.hpp"
#include "Response.hpp"

// Client::Client() : fd(-1), _server(NULL), request(), 
//     {
//          memset(&address, 0, sizeof(address)); }

// Updated constructor
Client::Client(int client_fd, Server* server) : fd(client_fd)
                                                , _server(server)
                                                , request(new Request(client_fd))
                                                , response()
                                                ,isReady(false)
{
    memset(&address, 0, sizeof(address));
    if (fd >= 0)
    {
        int flags = fcntl(client_fd, F_GETFL, 0);
        if (flags != -1)
            fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);
    }
}

Client::~Client()
{
    if (fd >= 0)
        close(fd);
    delete request;
    if (response)
        delete response;
}

Server* Client::getServer() const
{ return _server; }

void    Client::PrepareResponse()
{
    this->response = new Response(*request, getServer());
    this->isReady = true;
}

bool    Client::isRequestValid() const
{
    ssize_t bytesRead = request->getBytesRead();
            
    if (bytesRead <= 0)
    {
        if (bytesRead == 0)
        {
            std::cout << "Client fd = " << fd << " disconnected." << std::endl;
           return false;
        }
        else
        {
            if (errno != EAGAIN && errno != EWOULDBLOCK)
            {
                std::cerr << "Recv error on client fd = " << fd << ": " << strerror(errno) << std::endl;
               return false;
            }
        }
    }
    return true;
}

