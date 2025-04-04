/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cezou <cezou@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/21 03:15:00 by cviegas           #+#    #+#             */
/*   Updated: 2025/04/04 14:44:03 by cezou            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#include "Server.hpp"
#include <iostream>
#include <string>
#include <fstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <map>
#include <vector>
#include <poll.h>
#include <unistd.h>
#include <set>

#define DEFAULT_PORT 8080

/**
 * @brief Types de tokens pour le parseur
 */
enum TokenType
{
    TOKEN_KEYWORD, // server, location, listen, etc.
    TOKEN_SYMBOL,  // {, }, ;
    TOKEN_ID,      // nombres, identifiants, chemins non quotés
    TOKEN_STRING   // chaînes entre guillemets
};

/**
 * @brief Structure représentant un token avec valeur, type et ligne
 */
struct Token
{
    std::string value;
    TokenType type;
    std::size_t line;
    std::string filename; // Add filename for error reporting

    Token(const std::string &val, TokenType t, std::size_t ln) 
        : value(val), type(t), line(ln), filename("") {}

    void display() const
    {
        std::string typeStr;
        switch (type)
        {
        case TOKEN_KEYWORD:
            typeStr = "KEYWORD";
            break;
        case TOKEN_SYMBOL:
            typeStr = "SYMBOL";
            break;
        case TOKEN_ID:
            typeStr = "ID";
            break;
        case TOKEN_STRING:
            typeStr = "STRING";
            break;
        default:
            typeStr = "UNKNOWN";
            break;
        }
        std::cout << "[" << typeStr << "] " << value << " (line " << line << ")" << std::endl;
    }
};

/**
 * @brief Structure représentant un nœud dans l'arbre de configuration
 */
struct ConfigNode
{
    std::string type;
    std::string value;
    ConfigNode *parent;
    std::vector<ConfigNode *> children;
    std::map<std::string, std::vector<std::string> > directives;
    std::size_t line; // Add line number information

    ConfigNode(const std::string &t = "", const std::string &v = "", ConfigNode *p = NULL, std::size_t ln = 0)
        : type(t), value(v), parent(p), line(ln) {}

    ~ConfigNode()
    {
        for (size_t i = 0; i < children.size(); ++i)
        {
            delete children[i];
        }
    }
};

/**
 * @brief Classe principale du serveur web
 */
class Webserv
{
private:
    int sockfd;
    sockaddr_in adress;
    socklen_t adrLen;
    std::vector<pollfd> fds;
    std::map<int, Server> serveurs;
    ConfigNode *rootConfig;

    bool isKeyword(const std::string &str) const;
    void tokenizeLine(const std::string &line, std::size_t lineNum, std::vector<Token> &tokens,
                      bool &inQuoteSingle, bool &inQuoteDouble, std::string &pendingToken);
    ConfigNode *parseConfigBlock(std::vector<Token> &tokens, size_t &index, ConfigNode *parent);
    bool parseDirective(std::vector<Token> &tokens, size_t &index, ConfigNode *currentNode);
    void buildServers(ConfigNode *config);
    void displayTokens(const std::vector<Token> &tokens);
    void validateNoDuplicateLocations(ConfigNode *node, const std::string &filename);
    void validateNoNestedServers(ConfigNode *node, const std::string &filename);

public:
    Webserv();
    Webserv(std::string config_filename);
    Webserv(const Webserv &cpy);
    Webserv &operator=(const Webserv &rhs);
    ~Webserv();

    void storeServers(std::string &filename);
    void acceptNewClient();
    void displayConfig(ConfigNode *node, int depth = 0);
};

#endif