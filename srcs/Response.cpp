/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ple-guya <ple-guya@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/05 19:04:28 by ple-guya          #+#    #+#             */
/*   Updated: 2025/05/30 18:14:16 by ple-guya         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"
#include "Request.hpp"
#include "Response.hpp"

/*****************************/
/* Constructor & Destructors */
/*****************************/

Response::Response() : status_code("200 OK"), content_type("text/html"), body("")
{
    _server = NULL;
}

Response::Response(const Request &req, Server* server) : _server(server)
{
    std::string method = req.getMethod();
    fd = req.getfd();
    status_code = req.getStatusCode();
    setConnectionType(req.getHeader("Connection"));
        
    if (method == "GET")
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

/*****************************/
/*      Getter & Setter      */
/*****************************/

std::string Response::getbody() const
{
    return(body);
}

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

/**********************************/
/* Request Handling Member Funcs  */
/**********************************/

std::string Response::getMimeType(const std::string& filePath) const
{
    std::map<std::string, std::string> mimeTypes;
    mimeTypes[".html"] = "text/html";
    mimeTypes[".htm"] = "text/html";
    mimeTypes[".css"] = "text/css";
    mimeTypes[".js"] = "application/javascript";
    mimeTypes[".json"] = "application/json";
    mimeTypes[".jpg"] = "image/jpeg";
    mimeTypes[".jpeg"] = "image/jpeg";
    mimeTypes[".png"] = "image/png";
    mimeTypes[".gif"] = "image/gif";
    mimeTypes[".ico"] = "image/x-icon";
    mimeTypes[".txt"] = "text/plain";

    size_t dotPos = filePath.rfind('.');
    if (dotPos != std::string::npos)
    {
        std::string ext = filePath.substr(dotPos);
        if (mimeTypes.count(ext))
            return mimeTypes[ext];
    }
    return "application/octet-stream";
}

ConfigNode* Response::findBestLocation(const std::string& requestPath) const
{
    if (!_server || !_server->getConfigNode())
        return NULL;

    ConfigNode* serverNode = _server->getConfigNode();
    ConfigNode* bestMatch = serverNode;
    size_t longestMatchLen = 0;

    for (size_t i = 0; i < serverNode->children.size(); ++i)
    {
        ConfigNode* location = serverNode->children[i];
        if (location->type != "location")
            continue;

        const std::string& locationPath = location->value;
        if (requestPath.rfind(locationPath, 0) == 0)
        {
            bool fullMatch = (requestPath.length() == locationPath.length()) ||
                             (locationPath == "/") ||
                             (requestPath.length() > locationPath.length() && requestPath[locationPath.length()] == '/');

            if (fullMatch && locationPath.length() >= longestMatchLen) // Use >= to prefer more specific matches
            {
                longestMatchLen = locationPath.length();
                bestMatch = location;
            }
        }
    }
    return bestMatch;
}

std::string Response::findEffectiveRoot(ConfigNode* contextNode) const
{
    ConfigNode* searchNode = contextNode;
    while (searchNode)
    {
        std::map<std::string, std::vector<std::string> >::iterator it = searchNode->directives.find("root");
        if (it != searchNode->directives.end() && !it->second.empty())
            return it->second[0];
        searchNode = searchNode->parent;
    }
    return "";
}

std::vector<std::string> Response::findEffectiveIndexFiles(ConfigNode* contextNode) const
{
    ConfigNode* searchNode = contextNode;
    while (searchNode)
    {
        std::map<std::string, std::vector<std::string> >::iterator it = searchNode->directives.find("index");
        if (it != searchNode->directives.end() && !it->second.empty())
            return it->second;
        searchNode = searchNode->parent;
    }
    return std::vector<std::string>();
}

std::string Response::tryIndexFiles(const std::string& directoryPath, const std::vector<std::string>& indexFiles) const
{
    for (size_t i = 0; i < indexFiles.size(); ++i)
    {
        std::string indexedPath = directoryPath;
        if (!indexedPath.empty() && indexedPath[indexedPath.length() - 1] != '/')
            indexedPath += "/";
        indexedPath += indexFiles[i];

        struct stat path_stat;
        if (stat(indexedPath.c_str(), &path_stat) == 0 && S_ISREG(path_stat.st_mode))
            return indexedPath;
    }
    return "";
}

std::string Response::resolveFilePath(ConfigNode* locationNode, const std::string& requestPath) const
{
    if (!locationNode)
        return "";

    std::string root = findEffectiveRoot(locationNode);
    if (root.empty())
        return "";

    std::string fullPath = root;
    if (!fullPath.empty() && !requestPath.empty() && fullPath[fullPath.length() - 1] == '/' && requestPath[0] == '/')
        fullPath += requestPath.substr(1);
    else if (!fullPath.empty() && !requestPath.empty() && fullPath[fullPath.length() - 1] != '/' && requestPath[0] != '/')
        fullPath += "/" + requestPath;
    else
        fullPath += requestPath;

    struct stat path_stat;
    if (stat(fullPath.c_str(), &path_stat) == 0)
    {
        if (S_ISDIR(path_stat.st_mode))
        {
            std::vector<std::string> indexFiles = findEffectiveIndexFiles(locationNode);
            return tryIndexFiles(fullPath, indexFiles);
        }
        else if (S_ISREG(path_stat.st_mode))
            return fullPath;
    }
    return "";
}

bool Response::loadPageContent(const std::string& filePath, std::string& content) const
{
    std::ifstream file(filePath.c_str(), std::ios::binary);
    if (file.is_open())
    {
        std::stringstream buffer;
        buffer << file.rdbuf();
        content = buffer.str();
        file.close();
        return true;
    }
    return false;
}

/**
 * @brief Handles CGI request execution for both GET and POST
 * @param req The HTTP request
 * @param locationNode Configuration context node
 * @param filePath Path to the CGI script
 */
void Response::handleCgiRequest(const Request &req, ConfigNode* locationNode, const std::string& filePath)
{
    CGIsHandling* cgiHandler = CGIsHandling::findCgiHandler(filePath, locationNode);
    if (!cgiHandler)
        return;

    try
    {
        cgiHandler->setScriptPath(filePath);
        cgiHandler->setEnvironmentVariable("REQUEST_METHOD", req.getMethod());
        cgiHandler->setEnvironmentVariable("SCRIPT_FILENAME", filePath);
        cgiHandler->setEnvironmentVariable("SCRIPT_NAME", req.getPath());
        cgiHandler->setEnvironmentVariable("REQUEST_URI", req.getPath());
        cgiHandler->setEnvironmentVariable("QUERY_STRING", "");
        
        std::string contentType = req.getHeader("Content-Type");
        if (contentType.empty() && req.getMethod() == "POST")
            contentType = "application/x-www-form-urlencoded";
        cgiHandler->setEnvironmentVariable("CONTENT_TYPE", contentType);
        
        std::string requestBody = req.getBody();
        std::stringstream contentLengthStream;
        contentLengthStream << requestBody.length();
        cgiHandler->setEnvironmentVariable("CONTENT_LENGTH", contentLengthStream.str());
        
        if (req.getMethod() == "POST" && !requestBody.empty())
            cgiHandler->setEnvironmentVariable("REQUEST_BODY", requestBody);
        
        cgiHandler->setEnvironmentVariable("SERVER_SOFTWARE", "Webserv/1.0");
        cgiHandler->setEnvironmentVariable("SERVER_NAME", "localhost");
        
        std::stringstream portStream;
        portStream << _server->port;
        cgiHandler->setEnvironmentVariable("SERVER_PORT", portStream.str());
        
        cgiHandler->setEnvironmentVariable("GATEWAY_INTERFACE", "CGI/1.1");
        cgiHandler->setEnvironmentVariable("SERVER_PROTOCOL", "HTTP/1.1");
        cgiHandler->setEnvironmentVariable("REDIRECT_STATUS", "200");
        
        std::string cgiOutput = cgiHandler->execute();
        size_t headerEnd = cgiOutput.find("\r\n\r\n");
        if (headerEnd != std::string::npos)
        {
            std::string cgiBody = cgiOutput.substr(headerEnd + 4);
            setBody(cgiBody);
        }
        else
            setBody(cgiOutput);
        setContentType("text/html");
        delete cgiHandler;
    }
    catch (const std::exception& e)
    {
        delete cgiHandler;
        status_code = "500 Internal Server Error";
        loadErrorPage("500", locationNode);
    }
}

void Response::handleGetRequest(const Request &req)
{
    std::string path = req.getPath();
    ConfigNode* locationNode = findBestLocation(path);
    std::string filePath = resolveFilePath(locationNode, path);
    std::string content;
    
    std::cout << "URL demandée: " << req.getPath() << std::endl;
    std::cout << "Chemin complet du fichier: " << filePath << std::endl;

    if (filePath.empty())
    {
        status_code = "404 Not Found";
        loadErrorPage("404", locationNode);
        return;
    }

    CGIsHandling* cgiHandler = CGIsHandling::findCgiHandler(filePath, locationNode);
    if (cgiHandler)
    {
        delete cgiHandler;
        handleCgiRequest(req, locationNode, filePath);
    }
    else if (loadPageContent(filePath, content))
    {
        setBody(content);
        setContentType(getMimeType(filePath));
    }
    else
    {
        status_code = "403 Forbidden";
        loadErrorPage("403", locationNode);
    }
}

void Response::handlePostRequest(const Request &req)
{
    std::string path = req.getPath();
    ConfigNode* locationNode = findBestLocation(path);
    std::string filePath = resolveFilePath(locationNode, path);
    
    if (filePath.empty())
    {
        status_code = "404 Not Found";
        loadErrorPage("404", locationNode);
        return;
    }

    CGIsHandling* cgiHandler = CGIsHandling::findCgiHandler(filePath, locationNode);
    if (cgiHandler)
    {
        delete cgiHandler;
        handleCgiRequest(req, locationNode, filePath);
        return;
    }

    std::string contentType = req.getHeader("Content-Type");
    std::string boundary;
    size_t      pos = contentType.find("boundary=");
    std::string end;
    std::string delimiter;
    std::string content;
    std::map<std::string, std::string> bodyHeaders;
    std::vector<std::string> bodies;
        
    if (contentType.find("multipart/form-data") != std::string::npos)
    {
        content = req.getBody();
        if (pos != std::string::npos)
        {
            boundary = contentType.substr(pos + 9);
            delimiter = "--" + boundary;
            end = delimiter + "--";
        }
        else
        {
            status_code = BAD_REQUEST;
            return;
        }
        bodies = splitPostBody(content, delimiter);
        for (std::vector<std::string>::iterator it = bodies.begin(); it != bodies.end(); it++)
        {
            pos = it->find("\r\n\r\n");
            if (pos != std::string::npos)
            {
                std::string headerPart = it->substr(0, pos);
                std::string bodyPart = it->substr(pos + 4);
                bodyHeaders = extractPostHeaders(headerPart);
            }
        }
    }
}

void Response::handleDeleteRequest(const Request &req)
{
    (void)req;
}

/**********************************/
/* Error Handling Member Funcs    */
/**********************************/

std::string Response::findErrorPageUri(const std::string& errorCode, ConfigNode* locationNode, ConfigNode*& directiveContext) const
{
    ConfigNode* searchNode = locationNode;
    directiveContext = NULL;
    while (searchNode)
    {
        std::map<std::string, std::vector<std::string> >::iterator it = searchNode->directives.find("error_page");
        if (it != searchNode->directives.end())
        {
            const std::vector<std::string>& values = it->second;
            for (size_t i = 0; i < values.size() - 1; ++i)
            {
                if (values[i] == errorCode)
                {
                    directiveContext = searchNode;
                    return values.back();
                }
            }
        }
        searchNode = searchNode->parent;
    }
    return "";
}

std::string Response::resolveErrorPagePath(const std::string& errorPageUri, ConfigNode* directiveContext) const
{
    std::string fullPath = "";
    std::string locationPath = "";

    if (errorPageUri.empty())
        return "";
    if (errorPageUri[0] == '/')
    {
        if (!_server)
            return "";
		fullPath = findEffectiveRoot(_server->getConfigNode());
    }
    else
    {
        fullPath = findEffectiveRoot(directiveContext);
        if (directiveContext && directiveContext->type == "location")
            locationPath = directiveContext->value;
    }
    if (fullPath.empty())
        return "";
    if (!locationPath.empty())
    {
        if (!fullPath.empty() && fullPath[fullPath.length() - 1] != '/' && locationPath[0] != '/')
            fullPath += '/';
        else if (!fullPath.empty() && fullPath[fullPath.length() - 1] == '/' && locationPath[0] == '/')
            locationPath = locationPath.substr(1);
        fullPath += locationPath;
    }

    if (!fullPath.empty() && fullPath[fullPath.length() - 1] != '/')
        fullPath += '/';

    if (!errorPageUri.empty())
    {
        if (errorPageUri[0] == '/')
            fullPath += errorPageUri.substr(1);
        else
            fullPath += errorPageUri;
    }
    return fullPath;
}

void Response::generateDefaultErrorPage(const std::string& errorCode)
{
    (void)errorCode;
    std::string defaultBody = "<html><head><title>" + status_code + "</title></head>"
                              "<body><h1>" + status_code + "</h1></body></html>";
    setBody(defaultBody);
    setContentType("text/html");
}

void Response::loadErrorPage(const std::string& errorCode, ConfigNode* locationNode)
{
    ConfigNode* directiveContext = NULL;
    std::string errorPageUri = findErrorPageUri(errorCode, locationNode, directiveContext);
    std::string errorPagePath = "";
    std::string content = "";

    if (!errorPageUri.empty())
        errorPagePath = resolveErrorPagePath(errorPageUri, directiveContext);

    if (!errorPagePath.empty() && loadPageContent(errorPagePath, content))
    {
        setBody(content);
        setContentType(getMimeType(errorPagePath));
    }
    else
        generateDefaultErrorPage(errorCode);
}

/*****************************/
/*      Build Response       */
/*****************************/

std::string Response::build() const
{
    std::stringstream response;

    response << "HTTP/1.1 " << status_code << "\r\n";
    response << "Content-Type: " << content_type << "\r\n";
    response << "Content-Length: " << body.size() << "\r\n";
    for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it)
        response << it->first << ": " << it->second << "\r\n";
    response << "Connection: " << getConnectionType() << "\r\n"; 
    response << "\r\n";
    response << body;

    return response.str();
}

std::vector<std::string> Response::splitPostBody(std::string body, std::string delim)
{
    std::vector<std::string>    split;
    std::string                 content;
    size_t                      start = 0;
    size_t                      newStart;

    while (true)
    {
        newStart = body.find(delim, start);
        if (newStart == std::string::npos)
        {
            split.push_back(body.substr(start));
            break;
        }
        content = body.substr(start, newStart - start);
        split.push_back(content);
        start = newStart + delim.length();
    }
    return split;
}

std::map<std::string, std::string>  Response::extractPostHeaders(std::string content)
{
    std::map<std::string, std::string>  parseHeader;
    std::stringstream                   ss(content);
    std::string                         line;

    while(std::getline(ss, line))
    {
        if (line.empty() || line == "\r")
            break;
        size_t pos = line.find(":");
        if(pos == std::string::npos)
            continue;
        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);
        key = trimString(key, " \t\n\r");
        value = trimString(value, " \t\n\r");
        parseHeader[key] = value;
    }
    return parseHeader;
}
