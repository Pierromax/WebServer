/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ple-guya <ple-guya@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/05 19:04:28 by ple-guya          #+#    #+#             */
/*   Updated: 2025/04/14 13:25:01 by ple-guya         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"

/*****************************/
/* Constructor & Destructors */
/*****************************/

Response::Response() : status_code("200 OK"), content_type("text/html"), body("")
{
}

Response::Response(const Request &req)
{
    std::string status = req.getStatusCode();
    std::string method = req.getMethod();
    
    if (status != GOOD_REQUEST) {
        loadErrorPage(status);
    }
    if (method == "GET") {
        handleGetRequest(req);
    } else if (method == "POST") {
        handlePostRequest(req);
    } else if (method == "DELETE") {
        handleDeleteRequest(req);
    }
}

Response::Response(const Response &cpy) : status_code(cpy.status_code),
                                         content_type(cpy.content_type),
                                         body(cpy.body)
{
}

Response &Response::operator=(const Response &rhs)
{
    if (this != &rhs)
    {
        status_code = rhs.status_code;
        content_type = rhs.content_type;
        body = rhs.body;
    }
    return (*this);
}

Response::~Response()
{
}

/*****************************/
/*      Getter & Setter      */
/*****************************/

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

void Response::setBody(const std::string &body)
{
    this->body = body;
}

/*****************************/
/*      Member Functions     */
/*****************************/

void    Response::handleGetRequest(const Request &req)
{
    
}

void    Response::handlePostRequest(const Request &req)
{
    
}

void    Response::handleDeleteRequest(const Request &req)
{
    
}

std::string Response::build() const
{
    std::stringstream response;
    
    response << "HTTP/1.1 " << status_code << "\r\n";
    response << "Content-Type: " << content_type << "\r\n";
    response << "Content-Length: " << body.size() << "\r\n";
    response << "Connection: close\r\n";
    response << "\r\n";
    response << body;
    
    return response.str();
}

void Response::loadErrorPage(const std::string &status)
{
    std::string errorCode;
    std::string filePath;
    
    errorCode = status.substr(0,3);
    filePath = "config/html/" + errorCode + ".html";

    std::ifstream file(filePath.c_str());
    if (file.is_open())
    {
        std::stringstream buffer;
        buffer << file.rdbuf();
        setBody(buffer.str());
        file.close();
    }
}

