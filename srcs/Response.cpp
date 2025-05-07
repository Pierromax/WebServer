/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ple-guya <ple-guya@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/05 19:04:28 by ple-guya          #+#    #+#             */
/*   Updated: 2025/05/07 14:43:56 by ple-guya         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"

/*****************************/
/* Constructor & Destructors */
/*****************************/

Response::Response(const Request &req) : status_code("200 OK"), content_type("text/html"), body("")
{
    std::string status = req.getStatusCode();
    std::string method = req.getMethod();
    

    std::cout << req.getroot() << std::endl;
    
    if (method == "GET") {
        handleGetRequest(req);
    } else if (method == "POST") {
        handlePostRequest(req);
    } else if (method == "DELETE") {
        handleDeleteRequest(req);
    } else
        loadErrorPage(status);
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

std::string Response::getConnectionType() const
{
    return(connection_type);
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

/*****************************/
/*      Member Functions     */
/*****************************/

void    Response::handleGetRequest(const Request &req)
{
    std::string path = req.getPath();

    std::cout << "path : " << path << std::endl;
    setConnectionType("close");

    if (path == "/favicon.ico") 
    {
        status_code = "204 No Content";
        body = "";
        return;
    }
    if (path == "/")
    {
        try {
            setConnectionType("keep_alive");
            loadHtmlFile("/index.html");
        }
        catch(std::exception &e){
            loadErrorPage(FILE_NOT_FOUND);
        }
    }
    else
    {
        try {
            loadHtmlFile(path);
        }
        catch(std::exception &e){
            loadErrorPage(FILE_NOT_FOUND);
        }
    }
    //std::cout << body << std::endl;
}

void    Response::handlePostRequest(const Request &req)
{
    (void)req;

}

void    Response::handleDeleteRequest(const Request &req)
{
    (void)req;

}

std::string Response::build(const std::string &path) const
{
    std::stringstream response;

    
    response << "HTTP/1.1 " << status_code << "\r\n";
    for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it)
        response << it->first << it->second << "\r\n";
    response << "Content-Lenght: " << body.size() << "\r\n";
    response << "Connection: " << getConnectionType() << "\r\n";
    response << "\r\n";
    response << body; 
    
    return response.str();
}

void    Response::loadHtmlFile(const std::string &path)
{
    std::string filePath = "./config/html" + path;
    std::ifstream file(filePath.c_str());
    std::cout << filePath << std::endl;

    if (file.is_open())
    {
        std::stringstream buffer;
        buffer << file.rdbuf();
        setBody(buffer.str());
        file.close();
    }
    else
        throw std::exception();
}

void    Response::loadErrorPage(const std::string &status)
{
    std::string errorCode;
    std::string filePath;
    
    errorCode = status.substr(0,3);
    filePath = "./config/html" + errorCode + ".html";

    std::ifstream file(filePath.c_str());
    if (file.is_open())
    {
        std::stringstream buffer;
        buffer << file.rdbuf();
        setBody(buffer.str());
        file.close();
    }
}

