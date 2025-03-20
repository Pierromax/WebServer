/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cviegas <cviegas@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/05 19:04:35 by ple-guya          #+#    #+#             */
/*   Updated: 2025/03/21 00:44:43 by cviegas          ###   ########.fr       */
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

// Types de tokens possibles
enum TokenType
{
    TOKEN_KEYWORD, // server, location, listen, etc.
    TOKEN_SYMBOL,  // {, }, ;
    TOKEN_ID,      // nombres, identifiants, chemins non quotés
    TOKEN_STRING   // chaînes entre guillemets
};

// Structure représentant un token avec sa valeur, son type et son numéro de ligne
struct Token
{
    std::string value;
    TokenType type;
    std::size_t line; // Numéro de ligne où le token apparaît

    Token(const std::string &val, TokenType t, std::size_t ln) : value(val), type(t), line(ln) {}

    // Méthode pour afficher le token (utile pour le débogage)
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

class Webserv
{
private:
    int sockfd;
    sockaddr_in adress;
    socklen_t adrLen;
    std::vector<pollfd> fds;
    std::map<int, Server> serveurs;

    // Fonction pour tokenizer une ligne
    void tokenizeLine(const std::string &line, std::size_t lineNum, std::vector<Token> &tokens,
                      bool &inQuoteSingle, bool &inQuoteDouble, std::string &pendingToken);

    // Vérifie si un token est un mot-clé
    bool isKeyword(const std::string &str) const;

public:
    Webserv();
    Webserv(std::string config_filename);
    Webserv(const Webserv &cpy);
    Webserv &operator=(const Webserv &rhs);
    ~Webserv();

    void storeServers(std::string &filename);
    void acceptNewClient();
};

#endif