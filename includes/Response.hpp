/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ple-guya <ple-guya@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/05 19:04:37 by ple-guya          #+#    #+#             */
/*   Updated: 2025/05/04 18:37:47 by ple-guya         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include "Request.hpp"
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>

class Request;

class Response
{
    private:
        std::string status_code;
        std::map <std::string, std::string> headers;
        std::string connection_type;
        std::string content_type;
        std::string body;
        
    public:
        Response();
        Response(const Request &req);
        Response(const Response &cpy);
        Response &operator=(const Response &rhs);
        ~Response();

        void    setStatusCode(const std::string &status);
        void    setContentType(const std::string &type);
        void    setBody(const std::string &body);
        void    setHeaders(const std::string &key, const std::string &value);
        void    setConnectionType(const std::string &type);
        std::string getConnectionType() const;

        void    handleGetRequest(const Request &req);
        void    handlePostRequest(const Request &req);
        void    handleDeleteRequest(const Request &req);
        
        std::string build(const std::string &path) const;
        void    loadErrorPage(const std::string &status_code);
        void    loadHtmlFile(const std::string &file_path);
};

#endif