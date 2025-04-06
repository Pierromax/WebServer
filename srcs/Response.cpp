/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cezou <cezou@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/05 19:04:28 by ple-guya          #+#    #+#             */
/*   Updated: 2025/04/06 19:10:10 by cezou            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"

Response::Response() : status_code("200 OK"), content_type("text/html"), content("")
{
}

Response::Response(const std::string &status) : status_code(status), content_type("text/html"), content("")
{
}

Response::Response(const Response &cpy) : status_code(cpy.status_code),
                                         content_type(cpy.content_type),
                                         content(cpy.content)
{
}

Response &Response::operator=(const Response &rhs)
{
    if (this != &rhs)
    {
        status_code = rhs.status_code;
        content_type = rhs.content_type;
        content = rhs.content;
    }
    return (*this);
}

Response::~Response()
{
}

void Response::setStatusCode(const std::string &status)
{
    status_code = status;
}

void Response::setContentType(const std::string &type)
{
    content_type = type;
}

void Response::setContent(const std::string &body)
{
    content = body;
}

std::string Response::build() const
{
    std::stringstream response;
    
    response << "HTTP/1.1 " << status_code << "\r\n";
    response << "Content-Type: " << content_type << "\r\n";
    response << "Content-Length: " << content.size() << "\r\n";
    response << "Connection: close\r\n";
    response << "\r\n";
    response << content;
    
    return response.str();
}

void Response::loadErrorPage(const std::string &file_path)
{
    std::ifstream file(file_path.c_str());
    if (file.is_open())
    {
        std::stringstream buffer;
        buffer << file.rdbuf();
        setContent(buffer.str());
        file.close();
    }
    else
    {
        setContent("<html><body><h1>Error " + status_code + "</h1><p>Page not found</p></body></html>");
    }
}

