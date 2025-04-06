/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigValidator.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cezou <cezou@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/04 16:00:00 by cezou             #+#    #+#             */
/*   Updated: 2025/04/06 20:20:01 by cezou            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Webserv.hpp"
#include <sstream>

/**
 * @brief Vérifie qu'il n'y a pas de locations dupliquées dans le même noeud parent
 * @param node Noeud à vérifier
 * @param filename Nom du fichier de configuration
 * @throw std::runtime_error si des duplications sont détectées
 */
void Webserv::validateNoDuplicateLocations(ConfigNode *node,
                                          const std::string &filename)
{
    if (!node)
        return;
    std::map<std::string, size_t> locationPaths;
    
    for (size_t i = 0; i < node->children.size(); ++i)
    {
        ConfigNode *child = node->children[i];
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
        validateNoDuplicateLocations(child, filename);
    }
}

/**
 * @brief Vérifie qu'il n'y a pas de servers dans d'autres servers
 * @param node Noeud à vérifier
 * @param filename Nom du fichier de configuration
 * @throw std::runtime_error si des servers sont imbriqués
 */
void Webserv::validateNoNestedServers(ConfigNode *node, const std::string &filename)
{
    if (!node)
        return;
    
    if (node->type == "server")
    {
        for (size_t i = 0; i < node->children.size(); ++i)
        {
            if (node->children[i]->type == "server")
            {
                std::stringstream err;
                err << "\"server\" directive is not allowed here in " 
                    << filename << ":" << node->children[i]->line;
                throw std::runtime_error(err.str());
            }
            validateNoNestedServers(node->children[i], filename);
        }
    }
    else
    {
        for (size_t i = 0; i < node->children.size(); ++i)
            validateNoNestedServers(node->children[i], filename);
    }
}

/**
 * @brief Construit les objets Server à partir de l'arbre de configuration
 * @param config Noeud racine de la configuration
 */
void Webserv::buildServers(ConfigNode *config)
{
    if (!config)
        return;
    for (size_t i = 0; i < config->children.size(); ++i)
    {
        ConfigNode *serverNode = config->children[i];
        if (serverNode->type != "server")
            throw std::runtime_error("Expected 'server' block");
        Server server;
        std::map<std::string, std::vector<std::string> >::const_iterator it; 

        it = serverNode->directives.find("listen");
        if (it != serverNode->directives.end() && !it->second.empty())
        {
            std::stringstream portStream(it->second[0]);
            if (!(portStream >> server.port))
            {
                std::cout << "Warning: Invalid port value, using default port " 
                          << DEFAULT_PORT << std::endl;
                server.port = DEFAULT_PORT;
            }
        }
        else
            server.port = DEFAULT_PORT;
        it = serverNode->directives.find("server_name");
        if (it != serverNode->directives.end())
            server.serverNames = it->second;
        for (size_t j = 0; j < serverNode->children.size(); ++j)
        {
            ConfigNode *locationNode = serverNode->children[j];
            if (locationNode->type != "location")
                continue;
            t_Route route;
            route.directoryListing = false;
            route.allowUploads = false;
            
            it = locationNode->directives.find("root");
            if (it != locationNode->directives.end() && !it->second.empty())
                route.root = it->second[0];
            it = locationNode->directives.find("index");
            if (it != locationNode->directives.end() && !it->second.empty())
                route.defaultFile = it->second[0];
            server.routes[locationNode->value] = route;
        }
        std::cout << "Added server on port " << server.port << " with " 
                  << server.routes.size() << " locations" << std::endl;
        serveurs[server.port] = server;
    }
}