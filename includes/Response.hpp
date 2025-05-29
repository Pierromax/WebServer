/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ple-guya <ple-guya@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/05 19:04:37 by ple-guya          #+#    #+#             */
/*   Updated: 2025/05/29 15:47:59 by ple-guya         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include "Request.hpp"
#include "Server.hpp"
#include "Webserv.hpp"
#include "Utils.hpp"
#include <map>
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <sys/stat.h> // For stat()
#include <map>        // Added for getMimeType

// Forward declarations
struct ConfigNode;

class Response
{
    private:
        int         fd;
        std::string status_code;
        std::map <std::string, std::string> headers;
        std::string connection_type;
        std::string content_type;
        std::string body;
        Server*     _server;

        // --- Request Handling Helpers ---
        ConfigNode* findBestLocation(const std::string& requestPath) const;
        std::string findEffectiveRoot(ConfigNode* contextNode) const;
        std::vector<std::string>    findEffectiveIndexFiles(ConfigNode* contextNode) const;
        std::string tryIndexFiles(const std::string& directoryPath, const std::vector<std::string>& indexFiles) const;
        std::string resolveFilePath(ConfigNode* locationNode, const std::string& requestPath) const;
        std::string getMimeType(const std::string& filePath) const;
        bool        loadPageContent(const std::string& filePath, std::string& content) const;

        // --- Error Handling Helpers ---
        std::string findErrorPageUri(const std::string& errorCode, ConfigNode* locationNode, ConfigNode*& directiveContext) const;
        std::string resolveErrorPagePath(const std::string& errorPageUri, ConfigNode* directiveContext) const;
        void        generateDefaultErrorPage(const std::string& errorCode);
        void        loadErrorPage(const std::string& errorCode, ConfigNode* locationNode = NULL);

        // CreateReponse function
        void                        handleGetRequest(const Request &req);
        void                        handlePostRequest(const Request &req);
        void                        handleDeleteRequest(const Request &req);
        std::vector<std::string>    splitPostBody(std::string body, std::string delim);
        
    public:
        Response();
        Response(const Request &req, Server* server);
        Response(const Response &cpy);
        Response &operator=(const Response &rhs);
        ~Response();

        void        setStatusCode(const std::string &status);
        void        setContentType(const std::string &type);
        void        setBody(const std::string &body);
        void        setHeaders(const std::string &key, const std::string &value);
        void        setConnectionType(const std::string &type);
        std::string getConnectionType() const;
        std::string getbody() const;
        std::string build() const;
        void        sendResponse() const;
};

#endif