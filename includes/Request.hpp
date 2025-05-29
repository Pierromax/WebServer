/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ple-guya <ple-guya@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/05 19:04:40 by ple-guya          #+#    #+#             */
/*   Updated: 2025/05/28 17:26:03 by ple-guya         ###   ########.fr       */
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

#define GOOD_REQUEST "200 OK"
#define BAD_REQUEST "400 Bad Request"
#define FILE_NOT_FOUND "404 Not Found"
#define METHOD_NOT_ALLOWED "405 Method Not Allowed"

class Request
{
    private:
        int         fd;
        std::string statuscode;
        std::string method;
        std::string path;
        std::string version;
        std::map<std::string, std::string> headers;
        std::string body;
        std::string raw_request;
        ssize_t     _bytesRead; // Store the result of recv()
        bool        _isEmptyInput; // Flag for whitespace-only input

        void        parseRequest(const std::string &buffer);
        void        parseFirstline(const std::string &line);
        void        parseHeader(const std::string &buffer);
        void        parseBody(const std::string &buffer);

    public:
        Request(int clientFD);
        Request(const Request &cpy);
        Request &operator=(const Request &rhs);
        ~Request();

        int         getfd() const;
        std::string getMethod() const;
        std::string getPath() const;
        std::string getStatusCode() const;
        std::string getHeader(const std::string &name) const;
        std::string getBody() const;
        ssize_t     getBytesRead() const;
        bool        isEmptyInput() const;
        bool        isComplete() const;
        void        ReadFromSocket();
};

std::string trimString(std::string &str, const std::string &charset);

#endif