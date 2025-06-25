/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cviegas <cviegas@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/19 12:32:35 by ple-guya          #+#    #+#             */
/*   Updated: 2025/06/25 22:25:16 by cviegas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"
#include "Server.hpp"
#include "Request.hpp"
#include "Response.hpp"

Client::Client(int client_fd, Server* server, class Webserv* webserv) : fd(client_fd)
                                                , _server(server)
                                                , _webserv(webserv)
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

void            Client::setState(ClientState newState) {state = newState;}

Server* Client::getServer() const { return _server; }

void Client::setServer(Server* server) { _server = server; }

void    Client::prepareResponse()
{
    if (this->response)
        delete(this->response);
    this->response = new Response(*request, getServer());
}

void    Client::prepareRequest()
{
    if (!this->request)
        this->request = new Request(fd, _server);
    if (state == READING)
    {
        if (request->isComplete())
        {
            this->request->reset();
            if (request->getStatusCode() != "200 OK")
            {
                state = WRITING;
                return;
            }
        }
        this->request->ReadFromSocket();
        if (request->isComplete())
        {
            if (_webserv && !request->getHeader("Host").empty())
            {
                std::string hostHeader = request->getHeader("Host");
                int port = ntohs(_server->getAddress().sin_port);
                Server* bestServer = _webserv->findBestServer(port, hostHeader);
                if (bestServer && bestServer != _server)
                    setServer(bestServer);
            }
            state = WRITING;
        }

    }
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
