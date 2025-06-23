/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ValidateConfig.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cviegas <cviegas@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/04 16:00:10 by cezou             #+#    #+#             */
/*   Updated: 2025/06/12 17:40:41 by cviegas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Webserv.hpp"

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
        std::size_t directiveLine = node->line;
        std::map<std::string, std::size_t>::iterator lineIt = node->directiveLines.find(it->first);
        if (lineIt != node->directiveLines.end())
            directiveLine = lineIt->second;
        if (it->first == "methods")
            validateMethodsDirective(it->second, filename, directiveLine);
        else if (it->first == "error_page")
            validateErrorPageDirective(it->second, filename, directiveLine);
        else if (it->first == "listen")
            validateListenDirective(it->second, filename, directiveLine);
        else if (it->first == "client_max_body_size")
            validateClientMaxBodySizeDirective(it->second, filename, directiveLine, node);
        else if (it->first == "autoindex")
            validateAutoindexDirective(it->second, filename, directiveLine, node);
        else if (it->first == "return")
            validateRedirectDirective(it->second, filename, directiveLine, node);
    }
    for (std::map<std::string, std::string>::iterator it = node->cgiHandlers.begin(); 
         it != node->cgiHandlers.end(); ++it)
    {
        std::vector<std::string> values;
        values.push_back(it->first);
        values.push_back(it->second);
        std::size_t directiveLine = node->line;
        std::map<std::string, std::size_t>::iterator lineIt = node->directiveLines.find("cgi");
        if (lineIt != node->directiveLines.end())
            directiveLine = lineIt->second;
        validateCgiDirective(values, filename, directiveLine);
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

