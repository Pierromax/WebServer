/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ple-guya <ple-guya@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/19 12:32:35 by ple-guya          #+#    #+#             */
/*   Updated: 2025/05/14 22:38:55 by ple-guya         ###   ########.fr       */
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
                                                , request(NULL)
                                                , response(NULL)
                                                , isReady(false)
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
    if (request)
        delete request;
    if (response)
        delete response;
}

Server* Client::getServer() const
{ return _server; }

void    Client::prepareResponse()
{
    if (this->response)
        delete(this->response);
    this->response = new Response(*request, getServer());
}

void    Client::prepareRequest()
{
    if (this->request)
        delete this->request;
    this->request = new Request(fd);
    this->request->ReadFromSocket();
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

void Client::sendResponse() const
{
    std::string resp = this->response->build();
    
    ssize_t total_sent = 0;
    ssize_t resp_len = resp.size();
    while (total_sent < resp_len) 
    {
        ssize_t sent = send(this->fd, resp.c_str() + total_sent, resp_len - total_sent, 0);
        if (sent < 0) 
        {
            if (errno == EWOULDBLOCK || errno == EAGAIN) {
                std::cerr << "Socket not ready for writing, would block" << std::endl;
            } else if (errno == EPIPE || errno == ECONNRESET) {
                std::cerr << "Client disconnected" << std::endl;
            } else {
                std::cerr << "Failed to send response" << std::endl;
            }
            break;
        }
        total_sent += sent;
    }
}
