/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cezou <cezou@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/05 19:04:40 by ple-guya          #+#    #+#             */
/*   Updated: 2025/04/07 17:46:16 by cezou            ###   ########.fr       */
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
#include "Utils.hpp"


#define GOOD_REQUEST "200 OK"
#define BAD_REQUEST "400 Bad Request"
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

        void        parseRequest(const std::string &buffer);
        void        parseFirstline(const std::string &line);
        void        parseHeader(const std::string &buffer);
        void        parseBody(const std::string &buffer);

    public:
        Request(int clientFD);
        Request(const Request &cpy);
        Request &operator=(const Request &rhs);
        ~Request();

        std::string getMethod() const;
        std::string getPath() const;
        std::string getStatusCode() const;
        std::string getHeader(const std::string &name) const;
};

std::string trimString(std::string &str, const std::string &charset);

#endif