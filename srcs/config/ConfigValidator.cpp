/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigValidator.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cviegas <cviegas@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/04 16:00:10 by cezou             #+#    #+#             */
/*   Updated: 2025/06/09 19:30:29 by cviegas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Webserv.hpp"
#include <sstream>
#include <set>

/**
 * @brief Validates CGI directive format
 * @param values Values for the CGI directive
 * @param filename Configuration filename (for error reporting)
 * @param line Line number (for error reporting)
 */
void Webserv::validateCgiDirective(const std::vector<std::string> &values, const std::string &filename, std::size_t line)
{
    if (values.size() != 2)
    {
        std::stringstream err;
        err << "directive \"cgi\" takes exactly 2 arguments in " 
            << filename << ":" << line;
        throw std::runtime_error(err.str());
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
        if (it->first == "cgi")
            validateCgiDirective(it->second, filename, node->line);
        else if (it->first == "methods")
            validateMethodsDirective(it->second, filename, node->line);
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

