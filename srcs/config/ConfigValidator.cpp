/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigValidator.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cviegas <cviegas@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/04 16:00:10 by cezou             #+#    #+#             */
/*   Updated: 2025/06/12 15:17:54 by cviegas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Webserv.hpp"
#include <sstream>
#include <set>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstdlib>
#include <sys/stat.h>

/**
 * @brief Validates CGI directive format
 * @param values Values for the CGI directive
 * @param filename Configuration filename (for error reporting)
 * @param line Line number (for error reporting)
 */
void Webserv::validateCgiDirective(const std::vector<std::string> &values, const std::string &filename, std::size_t line)
{
    if (values.size() != 2)
        throw ParsingError("directive \"cgi\" takes exactly 2 arguments", filename, line);
    if (access(values[1].c_str(), F_OK | X_OK) != 0)
        throw ParsingError("cgi executable \"" + values[1] + "\" is not accessible in directive \"cgi\"", filename, line);
    struct stat fileStat;
    if (stat(values[1].c_str(), &fileStat) == 0) {
        if (S_ISDIR(fileStat.st_mode))
            throw ParsingError("cgi path \"" + values[1] + "\" is a directory, not an executable in directive \"cgi\"", filename, line);
    }
}

/**
 * @brief Validates methods directive format
 * @param values Values for the methods directive
 * @param filename Configuration filename (for error reporting)
 * @param line Line number (for error reporting)
 */
void Webserv::validateMethodsDirective(const std::vector<std::string> &values, const std::string &filename, std::size_t line)
{
    std::set<std::string> validMethods;
    std::set<std::string> usedMethods;

    if (values.size() > 3)
        throw ParsingError("directive \"methods\" takes at most 3 arguments", filename, line);
    validMethods.insert("GET");
    validMethods.insert("POST");
    validMethods.insert("DELETE");
    for (size_t i = 0; i < values.size(); ++i) {
        const std::string& method = values[i];
        if (validMethods.find(method) == validMethods.end())
            throw ParsingError("invalid method \"" + method + "\" in directive \"methods\"", filename, line);
        if (usedMethods.find(method) != usedMethods.end())
            throw ParsingError("duplicate method \"" + method + "\" in directive \"methods\"", filename, line);
        usedMethods.insert(method);
    }
}

/**
 * @brief Validates error_page directive format
 * @param values Values for the error_page directive
 * @param filename Configuration filename (for error reporting)
 * @param line Line number (for error reporting)
 */
void Webserv::validateErrorPageDirective(const std::vector<std::string> &values, const std::string &filename, std::size_t line)
{
    if (values.size() != 2)
        throw ParsingError("directive \"error_page\" takes exactly 2 arguments", filename, line);

    std::set<std::string> validCodes;
    validCodes.insert("200");
    validCodes.insert("400");
    validCodes.insert("404");
    validCodes.insert("405");
    validCodes.insert("413");
    validCodes.insert("500");

    const std::string& errorCode = values[0];
    if (validCodes.find(errorCode) == validCodes.end())
        throw ParsingError("invalid error code \"" + errorCode + "\" in directive \"error_page\"", filename, line);
}

/**
 * @brief Validates listen directive format
 * @param values Values for the listen directive
 * @param filename Configuration filename (for error reporting)
 * @param line Line number (for error reporting)
 */
void Webserv::validateListenDirective(const std::vector<std::string> &values, const std::string &filename, std::size_t line)
{
    for (size_t i = 0; i < values.size(); ++i) {
        const std::string& portStr = values[i];
        std::stringstream ss(portStr);
        int port;
        std::string remaining;

        if (!(ss >> port) || ss >> remaining)
            throw ParsingError("invalid port \"" + portStr + "\" in directive \"listen\"", filename, line);
        if (port < 1024 || port > 65535)
            throw ParsingError("port " + portStr + " out of range (1024-65535) in directive \"listen\"", filename, line);
        if (!isPortAvailable(port))
            throw ParsingError("port " + portStr + " is not available in directive \"listen\"", filename, line);
    }
}

/**
 * @brief Validates client_max_body_size directive format
 * @param values Values for the client_max_body_size directive
 * @param filename Configuration filename (for error reporting)
 * @param line Line number (for error reporting)
 * @param node ConfigNode to store the parsed value
 */
void Webserv::validateClientMaxBodySizeDirective(const std::vector<std::string> &values, const std::string &filename, std::size_t line, ConfigNode *node)
{
    if (values.size() != 1)
        throw ParsingError("directive \"client_max_body_size\" takes exactly 1 argument", filename, line);

    const std::string& sizeStr = values[0];
    std::stringstream ss(sizeStr);
    long long baseSize;
    std::string remaining;

    if (!(ss >> baseSize))
        throw ParsingError("invalid size \"" + sizeStr + "\" in directive \"client_max_body_size\"", filename, line);
    if (ss >> remaining) {
        if (remaining.length() != 1)
            throw ParsingError("invalid size format \"" + sizeStr + "\" in directive \"client_max_body_size\"", filename, line);
        char unit = remaining[0];
        if (unit == 'M' || unit == 'm')
            baseSize *= 1000000;
        else if (unit == 'K' || unit == 'k')
            baseSize *= 1000;
        else
            throw ParsingError("invalid size unit \"" + std::string(1, unit) + "\" in directive \"client_max_body_size\"", filename, line);
    } 
    if (baseSize < 1)
        throw ParsingError("size must be at least 1 byte in directive \"client_max_body_size\"", filename, line);
    node->client_max_body_size = baseSize;
}

/**
 * @brief Validates configuration tree structure and detects errors
 * @param node Node to check
 * @param filename Configuration filename (for error reporting)
 * @param depth Current depth in tree (0 for root)
 * @param usedPorts Map to track duplicate ports across servers
 * @throw std::runtime_error if configuration errors are detected
 */
void Webserv::validateConfigTree(ConfigNode *node, const std::string &filename, int depth, std::map<int, size_t> &usedPorts)
{
    if (!node)
        return;
    if (depth > 1 && node->type == "server")
        throw ParsingError("\"server\" directive is not allowed here", filename, node->line);
    validateDirectives(node, filename);
    validateChildNodes(node, filename, depth, usedPorts);
}

/**
 * @brief Validates directives within a node
 * @param node Node to validate
 * @param filename Configuration filename (for error reporting)
 */
void Webserv::validateDirectives(ConfigNode *node, const std::string &filename)
{
    for (std::map<std::string, std::vector<std::string> >::iterator it = node->directives.begin(); 
         it != node->directives.end(); ++it)
    {
        std::size_t directiveLine = node->line; // Valeur par défaut
        std::map<std::string, std::size_t>::iterator lineIt = node->directiveLines.find(it->first);
        if (lineIt != node->directiveLines.end())
            directiveLine = lineIt->second;

        if (it->first == "cgi")
            validateCgiDirective(it->second, filename, directiveLine);
        else if (it->first == "methods")
            validateMethodsDirective(it->second, filename, directiveLine);
        else if (it->first == "error_page")
            validateErrorPageDirective(it->second, filename, directiveLine);
        else if (it->first == "listen")
            validateListenDirective(it->second, filename, directiveLine);
        else if (it->first == "client_max_body_size")
            validateClientMaxBodySizeDirective(it->second, filename, directiveLine, node);
    }
}

/**
 * @brief Validates child nodes and checks for duplicates
 * @param node Parent node
 * @param filename Configuration filename (for error reporting) 
 * @param depth Current depth in tree
 * @param usedPorts Map to track duplicate ports across servers
 */
void Webserv::validateChildNodes(ConfigNode *node, const std::string &filename, int depth, std::map<int, size_t> &usedPorts)
{
    std::map<std::string, size_t> locationPaths;
    
    for (size_t i = 0; i < node->children.size(); ++i)
    {
        ConfigNode *child = node->children[i];
        
        if (node->type == "server" && child->type == "server")
            throw ParsingError("\"server\" directive is not allowed here", filename, child->line);
        if (child->type == "location")
        {
            std::string path = child->value;
            std::map<std::string, size_t>::iterator it = locationPaths.find(path);
            if (it != locationPaths.end())
                throw ParsingError("duplicate location \"" + path + "\"", filename, child->line);
            locationPaths[path] = child->line;
        }
        if (child->type == "server")
            validateServerPorts(child, filename, usedPorts);
        validateConfigTree(child, filename, depth + 1, usedPorts);
    }
}

/**
 * @brief Validates server ports for duplicates
 * @param serverNode Server node to validate
 * @param filename Configuration filename (for error reporting)
 * @param usedPorts Map to track duplicate ports
 */
void Webserv::validateServerPorts(ConfigNode *serverNode, const std::string &filename, std::map<int, size_t> &usedPorts)
{
    std::map<std::string, std::vector<std::string> >::iterator it = serverNode->directives.find("listen");
    if (it == serverNode->directives.end() || it->second.empty())
        return;
    
    std::set<int> serverPorts;
    for (size_t i = 0; i < it->second.size(); ++i) {
        std::stringstream ss(it->second[i]);
        int port;
        if (!(ss >> port))
            continue;
        
        if (serverPorts.find(port) != serverPorts.end())
            throw ParsingError("duplicate listen port " + it->second[i] + " in same server", filename, serverNode->line);
        serverPorts.insert(port);
        
        std::map<int, size_t>::iterator portIt = usedPorts.find(port);
        if (portIt != usedPorts.end()) {
            std::stringstream err;
            err << "duplicate listen port " << port << " (first defined at line " << portIt->second << ")";
            throw ParsingError(err.str(), filename, serverNode->line);
        }
        usedPorts[port] = serverNode->line;
    }
}

/**
 * @brief Checks if a port is available for binding
 * @param port Port number to check
 * @return true if port is available, false otherwise
 */
bool isPortAvailable(int port)
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        return false;

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    bool available = (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == 0);
    close(sockfd);
    return available;
}

/**
 * @brief Validates the configuration tree
 * @param root Root node of the configuration
 * @param filename Configuration filename (for error reporting)
 */
void Webserv::validateServers(ConfigNode *root, const std::string &filename)
{
    std::map<int, size_t> usedPorts;
    validateConfigTree(root, filename, 0, usedPorts);
    displayConfig(root);
}

