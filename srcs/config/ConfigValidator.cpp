/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigValidator.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cezou <cezou@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/04 16:00:10 by cezou             #+#    #+#             */
/*   Updated: 2025/04/08 15:46:40 by cezou            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Webserv.hpp"
#include <sstream>

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
 * @brief Validates configuration tree structure and detects errors
 * @param node Node to check
 * @param filename Configuration filename (for error reporting)
 * @param depth Current depth in tree (0 for root)
 * @throw std::runtime_error if configuration errors are detected
 */
void Webserv::validateConfigTree(ConfigNode *node, const std::string &filename, int depth)
{
    if (!node)
        return;
    std::map<std::string, size_t> locationPaths;
    
    if (depth > 1 && node->type == "server")
    {
        std::stringstream err;
        err << "\"server\" directive is not allowed here in " 
            << filename << ":" << node->line;
        throw std::runtime_error(err.str());
    }
    
    for (std::map<std::string, std::vector<std::string> >::iterator it = node->directives.begin(); 
         it != node->directives.end(); ++it)
    {
        if (it->first == "cgi")
            validateCgiDirective(it->second, filename, node->line);
    }
    
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
    }
    
    for (size_t i = 0; i < node->children.size(); ++i)
        validateConfigTree(node->children[i], filename, depth + 1);
}

/**
 * @brief Validates the configuration tree
 * @param root Root node of the configuration
 * @param filename Configuration filename (for error reporting)
 */
void Webserv::validateServers(ConfigNode *root, const std::string &filename)
{
    validateConfigTree(root, filename, 0);
    displayConfig(root);
}

