/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ple-guya <ple-guya@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/05 19:04:37 by ple-guya          #+#    #+#             */
/*   Updated: 2025/06/22 15:09:06 by ple-guya         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include "Request.hpp"
#include "Server.hpp"
#include "Webserv.hpp"
#include "Utils.hpp"
#include "CGIsHandling.hpp"
#include <cstdlib>
#include <map>
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <sys/stat.h> // For stat()
#include <dirent.h>   // For opendir/readdir
#include <map>        // Added for getMimeType

// Forward declarations
struct ConfigNode;

struct sessionData;

class Response
{
    private:
        int         fd;
        std::string status_code;
        std::map <std::string, std::string> headers;
        std::map <std::string, std::string> Cookies;
        std::pair<std::string, sessionData> lastSession;
        std::string connection_type;
        std::string content_type;
        std::string body;
        Server*     _server;

        // --- Request Handling Helpers ---
        ConfigNode* findBestLocation(const std::string& requestPath) const;
        ConfigNode* findBestLocationRecursive(const std::string& requestPath, ConfigNode* currentNode, ConfigNode* bestMatch, const std::string& currentPath) const;
        std::string findEffectiveRoot(ConfigNode* contextNode) const;
        std::vector<std::string>    findEffectiveIndexFiles(ConfigNode* contextNode) const;
        std::string tryIndexFiles(const std::string& directoryPath, const std::vector<std::string>& indexFiles) const;
        std::string resolveFilePath(ConfigNode* locationNode, const std::string& requestPath) const;
        std::string getMimeType(const std::string& filePath) const;
        bool        loadPageContent(const std::string& filePath, std::string& content) const;

        // --- Methods Handling ---
        bool        isMethodAllowed(const std::string& method, ConfigNode* locationNode) const;

        // --- Redirect & Autoindex Helpers ---
        bool        checkForRedirect(ConfigNode* locationNode, const std::string& requestPath);
        bool        shouldGenerateAutoindex(ConfigNode* locationNode) const;
        std::string generateDirectoryListing(const std::string& directoryPath, const std::string& requestPath) const;
        
        // --- Directory Listing Helpers ---
        void        generateHtmlHeader(std::stringstream& html, const std::string& requestPath) const;
        void        generateTableHeader(std::stringstream& html) const;
        void        generateParentDirectoryLink(std::stringstream& html, const std::string& requestPath) const;
        std::string calculateParentPath(const std::string& requestPath) const;
        void        generateDirectoryEntries(std::stringstream& html, const std::string& directoryPath, const std::string& requestPath) const;
        void        generateSingleEntry(std::stringstream& html, const std::string& directoryPath, const std::string& requestPath, const std::string& entryName) const;
        std::string buildEntryFullPath(const std::string& directoryPath, const std::string& entryName) const;
        std::string buildEntryLinkPath(const std::string& requestPath, const std::string& entryName) const;
        std::string formatFileTime(time_t mtime) const;
        void        generateEntryRow(std::stringstream& html, const std::string& linkPath, const std::string& entryName, const std::string& timeStr, bool isDirectory, off_t fileSize) const;
        void        generateHtmlFooter(std::stringstream& html) const;
        
        // --- Path Resolution Helper ---
        std::string buildFullPath(ConfigNode* locationNode, const std::string& requestPath) const;
        std::string getHttpStatusMessage(const std::string& statusCode) const;

        // --- CGI Handling ---
        void        handleCgiRequest(const Request &req, ConfigNode* locationNode, const std::string& filePath);

        // --- Error Handling Helpers ---
        std::string findErrorPageUri(const std::string& errorCode, ConfigNode* locationNode, ConfigNode*& directiveContext) const;
        std::string resolveErrorPagePath(const std::string& errorPageUri, ConfigNode* directiveContext) const;
        void        generateDefaultErrorPage(const std::string& errorCode);
        void        loadErrorPage(const std::string& errorCode, ConfigNode* locationNode = NULL);

        //login handling
        bool        checkLogin(const Request &req);
        void        handleLogin(const Request &req, ConfigNode *locationNode);
        void        createSession(std::string username);
        void        redirectTo(std::string const &path);
        
        // Cookies Gestion
        void        extractCookie(const std::string &cookie);
        void        setCookie(const std::string &key, const std::string &value);
        void        setCookieUser();
        void        deleteCookie();
        bool        checkDB(std::string path, std::string userName, std::string password);
        
        // CreateReponse function
        void                                handleGetRequest(const Request &req);
        void                                handlePostRequest(const Request &req);
        void                                handleDeleteRequest(const Request &req);
        void                                handleUploading(const Request &req,   ConfigNode* locationNode);
        std::vector<std::string>            splitPostBody(std::string body, std::string delim);
        std::map<std::string, std::string>  extractPostHeaders(std::string &content);
        bool                                extractFileToSave(std::map<std::string, std::string> &headers, std::string &content, std::string location);
        bool                                saveFile(std::string &filename, std::string &body, std::string location);
        
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