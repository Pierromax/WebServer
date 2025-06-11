/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ple-guya <ple-guya@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/19 12:32:35 by ple-guya          #+#    #+#             */
/*   Updated: 2025/06/11 15:55:39 by ple-guya         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"
#include "Server.hpp"
#include "Request.hpp"
#include "Response.hpp"

Client::Client(int client_fd, Server* server) : fd(client_fd)
                                                , _server(server)
                                                , request(NULL)
                                                , response(NULL)
{
    this->lastActivity = time(NULL);
    memset(&address, 0, sizeof(address));
    if (fd >= 0)
    {
        int flags = fcntl(client_fd, F_GETFL, 0);
        if (flags != -1)
            fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);
    }
    state = READING;
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

time_t  Client::getLastActivity() const {return this->lastActivity;}

ClientState     Client::getState() const {return state;}


bool Client::isTimeout() const
{
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    
    // Configuration adaptative selon l'état
    time_t timeout_value;
    if (!request || !request->isComplete())
        timeout_value = CONNECTION_ESTABLISHMENT_TIMEOUT; // ~30s
    else if (response)
        timeout_value = RESPONSE_SENDING_TIMEOUT; // ~60s
    else
        timeout_value = KEEPALIVE_TIMEOUT; // ~120s
    
    return (now.tv_sec - lastActivity) > timeout_value;
}

Server* Client::getServer() const { return _server; }

void    Client::prepareResponse()
{
    if (this->response)
        delete(this->response);
    this->response = new Response(*request, getServer());
}

void    Client::prepareRequest()
{
    if (!this->request)
        this->request = new Request(fd);
    if (state == READING)
    {
        this->request->ReadFromSocket();
        if (request->isComplete())
            state = WRITING;
    }
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
    if (getState() == READING)
        return;

    std::string resp = this->response->build();
    ssize_t     total_sent = 0;
    ssize_t     resp_len = resp.size();
    
    while (total_sent < resp_len) 
    {
        ssize_t sent = send(this->fd, resp.c_str() + total_sent, resp_len - total_sent, 0);
        if (sent <= 0) 
            break;
        total_sent += sent;
    }
}
