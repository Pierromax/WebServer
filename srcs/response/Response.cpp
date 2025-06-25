/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ple-guya <ple-guya@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/05 19:04:28 by ple-guya          #+#    #+#             */
/*   Updated: 2025/06/25 22:14:12 by ple-guya         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"
#include "Request.hpp"
#include "Response.hpp"


Response::Response() : status_code("200 OK"), content_type("text/html"), body("")
{
    _server = NULL;
}

bool Response::isLoginRequired(std::string req_path)
{
    ConfigNode *location = findBestLocation(req_path, _server);
    return (location && location->isAuthRequired);
}

Response::Response(Server* server) : _server(server)
{}

Response::Response(const Request &req, Server* server) : _server(server)
{
    std::string method = req.getMethod();
    fd = req.getfd();
    status_code = req.getStatusCode();
    setConnectionType(req.getHeader("Connection"));
        
    if (!checkLogin(req) && isLoginRequired(req.getPath()))
        redirectTo("/login");
    else if (method == "GET")
        handleGetRequest(req);
    else if (method == "POST")
        handlePostRequest(req);
    else if (method == "DELETE")
        handleDeleteRequest(req);
}

Response::Response(const Response &cpy) : status_code(cpy.status_code),
                                         content_type(cpy.content_type),
                                         body(cpy.body),
                                         _server(cpy._server)
{
}

Response &Response::operator=(const Response &rhs)
{
    if (this != &rhs)
    {
        status_code = rhs.status_code;
        content_type = rhs.content_type;
        body = rhs.body;
        _server = rhs._server;
    }
    return (*this);
}

Response::~Response()
{
}

std::string Response::getbody() const
{
    return (body);
}

std::string Response::getConnectionType() const
{
    return (connection_type);
}

void Response::setStatusCode(const std::string &status)
{
    status_code = status;
}

void Response::setHeaders(const std::string &headKey, const std::string &headValue)
{
    headers[headKey] = headValue;
}

void Response::setContentType(const std::string &type)
{
    content_type = type;
}

void Response::setConnectionType(const std::string &type)
{
    connection_type = type;
}

void Response::setBody(const std::string &body)
{
    this->body = body;
}

std::string Response::build() const
{
    std::stringstream response;

    response << "HTTP/1.1 " << status_code << "\r\n";
    response << "Content-Type: " << content_type << "\r\n";
    response << "Content-Length: " << body.size() << "\r\n";
    for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it)
        response << it->first << ": " << it->second << "\r\n";
    for (std::map<std::string, std::string>::const_iterator it = Cookies.begin(); it != Cookies.end(); ++it)
    {
        response << "Set-Cookie: " << it->first << "=" << it->second;
        if (it->second.empty())
            response << "; Max-Age=0\r\n";
        else
            response << "; Max-Age=3600\r\n";
    }
    response << "Connection: " << getConnectionType() << "\r\n"; 
    response << "\r\n";
    response << body;

    return response.str();
}
