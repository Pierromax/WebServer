/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv_utils.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cviegas <cviegas@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/15 17:20:00 by cviegas           #+#    #+#             */
/*   Updated: 2025/06/15 17:09:32 by cviegas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Webserv.hpp"
#include "Server.hpp"

ConfigNode* findBestLocation(const std::string& requestPath, Server* server)
{
    if (!server || !server->getConfigNode())
        return NULL;
    ConfigNode* serverNode = server->getConfigNode();
    return findBestLocationRecursive(requestPath, serverNode, serverNode, "");
}

ConfigNode* findBestLocationRecursive(const std::string& requestPath, ConfigNode* currentNode, ConfigNode* bestMatch, const std::string& currentPath)
{
    if (!currentNode)
        return bestMatch;
    size_t currentBestLen = (bestMatch->type == "location") ? bestMatch->value.length() : 0;
    for (size_t i = 0; i < currentNode->children.size(); ++i)
    {
        ConfigNode* location = currentNode->children[i];
        if (location->type != "location")
            continue;
        std::string fullLocationPath = currentPath + location->value;
        if (requestPath.rfind(fullLocationPath, 0) == 0)
        {
            bool fullMatch = (requestPath.length() == fullLocationPath.length()) ||
                             (fullLocationPath == "/") ||
                             (requestPath.length() > fullLocationPath.length() && requestPath[fullLocationPath.length()] == '/');
            if (fullMatch && fullLocationPath.length() >= currentBestLen)
            {
                bestMatch = location;
                bestMatch = findBestLocationRecursive(requestPath, location, bestMatch, fullLocationPath);
            }
        }
    }
    return bestMatch;
}
