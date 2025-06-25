/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ple-guya <ple-guya@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/05 19:04:40 by ple-guya          #+#    #+#             */
/*   Updated: 2025/06/25 21:21:31 by ple-guya         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <iostream>
#include <string>
#include <map>
#include <sys/socket.h>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <cstdio>

#define GOOD_REQUEST "200 OK"
#define BAD_REQUEST "400 Bad Request"
#define FORBIDDEN "403 Forbidden"
#define FILE_NOT_FOUND "404 Not Found"
#define NO_CONTENT "204 No Content"
#define METHOD_NOT_ALLOWED "405 Method Not Allowed"
#define PAYLOAD_TOO_LARGE "413 Payload Too Large"
#define INTERNAL_ERROR "500 Internal Server Error"

class Server;

class Request
{
    private:
        int         fd;
        Server*     _server; 
        std::string statuscode;
        std::string method;
        std::string path;
        std::string version;
        std::map<std::string, std::string> headers;
        std::string body;
        std::string raw_request;
        ssize_t     _bytesRead;
        size_t      _maxBodySize;
        size_t      _step;
        size_t      _headerSize; 

        void        parseRequest(const std::string &buffer);
        void        parseFirstline(const std::string &line);
        void        parseHeader(const std::string &buffer);
        void        parseBody(const std::string &buffer);
        bool        isRequestTooLong();
        
        // --- Helper methods for isRequestTooLong() ---
        size_t      findHeaderSize();
        std::string extractTempPath();
        void        updateMaxBodySize(const std::string& tempPath);
        size_t      extractContentLength();
        bool        checkContentLengthLimit(size_t contentLength);
        bool        checkCurrentBodySize();


    public:
        Request();
        Request(int clientFD, Server* server = NULL); // Ajout du paramètre Server*
        Request(const Request &cpy);
        Request &operator=(const Request &rhs);
        ~Request();

        // --- Getters ---
        int         getfd() const;
        std::string getMethod() const;
        std::string getPath() const;
        std::string getStatusCode() const;
        std::string getBody() const;
        std::string getHeader(const std::string &name) const;
        ssize_t     getBytesRead() const;
        void        createShutdownRequest();
        bool        isComplete() const;
        void        ReadFromSocket();
        void        reset(); // Reset pour les connexions keep-alive
};

std::string trimString(const std::string &str, const std::string &charset);

#endif