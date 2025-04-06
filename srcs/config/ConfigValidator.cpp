/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigValidator.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cezou <cezou@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/04 16:00:10 by cezou             #+#    #+#             */
/*   Updated: 2025/04/07 00:03:36 by cezou            ###   ########.fr       */
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
        
        int port = DEFAULT_PORT;
        
        std::map<std::string, std::vector<std::string> >::const_iterator it; 
        it = serverNode->directives.find("listen");
        if (it != serverNode->directives.end() && !it->second.empty())
        {
            std::stringstream portStream(it->second[0]);
            if (!(portStream >> port))
            {
                std::cout << "Warning: Invalid port value, using default port " 
                          << DEFAULT_PORT << std::endl;
                port = DEFAULT_PORT;
            }
        }
        
        Serveur *newServer = new Serveur();
        serveurs[newServer->getfd()] = newServer;
        
        std::cout << "Added server on port " << port << std::endl;
    }
}