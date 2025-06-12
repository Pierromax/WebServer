/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigValidator.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cviegas <cviegas@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/04 16:00:10 by cezou             #+#    #+#             */
/*   Updated: 2025/06/12 14:28:54 by cviegas          ###   ########.fr       */
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
    if (values.size() != 2) {
        std::stringstream err;
        err << "directive \"cgi\" takes exactly 2 arguments in " 
            << filename << ":" << line;
        throw std::runtime_error(err.str());
    }
    if (access(values[1].c_str(), F_OK | X_OK) != 0) {
        std::stringstream err;
        err << "cgi executable \"" << values[1] << "\" is not accessible in directive \"cgi\" in " 
            << filename << ":" << line;
        throw std::runtime_error(err.str());
    }
    struct stat fileStat;
    if (stat(values[1].c_str(), &fileStat) == 0) {
        if (S_ISDIR(fileStat.st_mode)) {
            std::stringstream err;
            err << "cgi path \"" << values[1] << "\" is a directory, not an executable in directive \"cgi\" in " 
                << filename << ":" << line;
            throw std::runtime_error(err.str());
        }
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
    {
        std::stringstream err;
        err << "directive \"methods\" takes at most 3 arguments in " 
            << filename << ":" << line;
        throw std::runtime_error(err.str());
    }
    validMethods.insert("GET");
    validMethods.insert("POST");
    validMethods.insert("DELETE");
    for (size_t i = 0; i < values.size(); ++i)
    {
        const std::string& method = values[i];
        if (validMethods.find(method) == validMethods.end())
        {
            std::stringstream err;
            err << "invalid method \"" << method << "\" in directive \"methods\" in " 
                << filename << ":" << line;
            throw std::runtime_error(err.str());
        }
        if (usedMethods.find(method) != usedMethods.end())
        {
            std::stringstream err;
            err << "duplicate method \"" << method << "\" in directive \"methods\" in " 
                << filename << ":" << line;
            throw std::runtime_error(err.str());
        }
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
    if (values.size() != 2) {
        std::stringstream err;
        err << "directive \"error_page\" takes exactly 2 arguments in " 
            << filename << ":" << line;
        throw std::runtime_error(err.str());
    }

    std::set<std::string> validCodes;
    validCodes.insert("200");
    validCodes.insert("400");
    validCodes.insert("404");
    validCodes.insert("405");
    validCodes.insert("413");
    validCodes.insert("500");

    const std::string& errorCode = values[0];
    if (validCodes.find(errorCode) == validCodes.end()) {
        std::stringstream err;
        err << "invalid error code \"" << errorCode << "\" in directive \"error_page\" in " 
            << filename << ":" << line;
        throw std::runtime_error(err.str());
    }
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

        if (!(ss >> port) || ss >> remaining) {
            std::stringstream err;
            err << "invalid port \"" << portStr << "\" in directive \"listen\" in " 
                << filename << ":" << line;
            throw std::runtime_error(err.str());
        }
        if (port < 1024 || port > 65535) {
            std::stringstream err;
            err << "port " << port << " out of range (1024-65535) in directive \"listen\" in " 
                << filename << ":" << line;
            throw std::runtime_error(err.str());
        }
        if (!isPortAvailable(port)) {
            std::stringstream err;
            err << "port " << port << " is not available in directive \"listen\" in " 
                << filename << ":" << line;
            throw std::runtime_error(err.str());
        }
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
    if (values.size() != 1) {
        std::stringstream err;
        err << "directive \"client_max_body_size\" takes exactly 1 argument in " 
            << filename << ":" << line;
        throw std::runtime_error(err.str());
    }

    const std::string& sizeStr = values[0];
    std::stringstream ss(sizeStr);
    long long baseSize;
    std::string remaining;

    if (!(ss >> baseSize)) {
        std::stringstream err;
        err << "invalid size \"" << sizeStr << "\" in directive \"client_max_body_size\" in " 
            << filename << ":" << line;
        throw std::runtime_error(err.str());
    }
    if (ss >> remaining) {
        if (remaining.length() != 1) {
            std::stringstream err;
            err << "invalid size format \"" << sizeStr << "\" in directive \"client_max_body_size\" in " 
                << filename << ":" << line;
            throw std::runtime_error(err.str());
        }
        char unit = remaining[0];
        if (unit == 'M' || unit == 'm')
            baseSize *= 1000000;
        else if (unit == 'K' || unit == 'k')
            baseSize *= 1000;
        else {
            std::stringstream err;
            err << "invalid size unit \"" << unit << "\" in directive \"client_max_body_size\" in " 
                << filename << ":" << line;
            throw std::runtime_error(err.str());
        }
    } 
    if (baseSize < 1)
    {
        std::stringstream err;
        err << "size must be at least 1 byte in directive \"client_max_body_size\" in " 
            << filename << ":" << line;
        throw std::runtime_error(err.str());
    }
    node->client_max_body_size = baseSize;
    std::cout << "Taille: " <<  node->client_max_body_size;
    
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
    {
        std::stringstream err;
        err << "\"server\" directive is not allowed here in " 
            << filename << ":" << node->line;
        throw std::runtime_error(err.str());
    }
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
        {
            std::stringstream err;
            err << "\"server\" directive is not allowed here in " 
                << filename << ":" << child->line;
            throw std::runtime_error(err.str());
        }
        if (child->type == "location")
        {
            std::string path = child->value;
            std::map<std::string, size_t>::iterator it = locationPaths.find(path);
            if (it != locationPaths.end())
            {
                std::stringstream err;
                err << "duplicate location \"" << path << "\" in " << filename 
                    << ":" << child->line;
                throw std::runtime_error(err.str());
            }
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
    std::stringstream ss(it->second[0]);
    int port;
    if (!(ss >> port))
        return;
    std::map<int, size_t>::iterator portIt = usedPorts.find(port);
    if (portIt != usedPorts.end())
    {
        std::stringstream err;
        err << "duplicate listen port " << port << " in " 
            << filename << ":" << serverNode->line 
            << " (first defined at line " << portIt->second << ")";
        throw std::runtime_error(err.str());
    }
    usedPorts[port] = serverNode->line;
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

