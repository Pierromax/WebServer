/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cviegas <cviegas@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/21 03:15:00 by cviegas           #+#    #+#             */
/*   Updated: 2025/06/12 16:06:56 by cviegas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#include "Server.hpp"
#include "Client.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "Utils.hpp"

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <map>
#include <vector>
#include <poll.h> 
#include <unistd.h>
#include <set>
#include <cstring>
#include <fcntl.h>
#include <signal.h>
#include <csignal> // Pour sig_atomic_t

#ifndef CONFIG_TESTER
# define CONFIG_TESTER 0
#endif

#define DEFAULT_PORT 8080

#define VALID_CODES_LIST \
    "200", "301", "302", "400", "404", "405", "413", "500"
// valid codes string array
#define VALID_CODES {"200", "404", "500", "400", "405", "413"}

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
 * @brief Méthodes HTTP supportées
 */
enum HttpMethod
{
    METHOD_GET = 0,
    METHOD_POST = 1,
    METHOD_DELETE = 2,
    METHOD_COUNT = 3
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
    std::map<std::string, std::size_t> directiveLines; 
    std::size_t line;
    bool allowedMethods[METHOD_COUNT];
    size_t client_max_body_size;
    bool autoindex;

    ConfigNode(const std::string &t = "", const std::string &v = "", ConfigNode *p = NULL, std::size_t ln = 0)
        : type(t), value(v), parent(p), line(ln), client_max_body_size(p ? p->client_max_body_size : 1048576), autoindex(false)
    {
        for (int i = 0; i < METHOD_COUNT; ++i)
            allowedMethods[i] = true;
    }

    ~ConfigNode()
    {
        for (size_t i = 0; i < children.size(); ++i)
            delete children[i];
    }
};

/**
 * @brief Classe principale du serveur web
 */
class Webserv
{
public:
    /**
     * @brief Exception class for parsing errors
     */
    class ParsingError : public std::exception
    {
    private:
        std::string message;

    public:
        ParsingError(const std::string& error, const std::string& filename, std::size_t line)
        {
            std::stringstream ss;
            ss << B RED "Parsing Error: " R RED << error << " in " << filename << ":" << line << R;
            message = ss.str();
        }

        ParsingError(const std::string& error, const std::string& filename)
        {
            std::stringstream ss;
            ss << B RED "Parsing Error: " R RED << error << " in " << filename << R;
            message = ss.str();
        }

        ParsingError(const std::string& error)
        {
            std::stringstream ss;
            ss << B RED "Parsing Error: " R RED << error << R;
            message = ss.str();
        }

        virtual ~ParsingError() throw() {}

        virtual const char* what() const throw()
        {
            return message.c_str();
        }
    };

private:
    int sockfd;
    sockaddr_in adress;
    socklen_t adrLen;
    std::vector<pollfd> fds;
    std::map<int, Server*> servers;
    std::map<int, Client*> clients;
    ConfigNode *rootConfig;

    bool isKeyword(const std::string &str) const;
    void tokenizeLine(const std::string &line, std::size_t lineNum, std::vector<Token> &tokens,
                      bool &inQuoteSingle, bool &inQuoteDouble, std::string &pendingToken);
    
    std::vector<Token> tokenizeConfigFile(const std::string &filename);
    std::vector<Token> filterTokens(const std::vector<Token> &tokens, const std::string &filename);
    ConfigNode* parseTokens(std::vector<Token> &tokens, const std::string &filename);
    void validateServers(ConfigNode *root, const std::string &filename);
    
    void validateConfigTree(ConfigNode *node, const std::string &filename, int depth, std::map<int, size_t> &usedPorts);
    void validateCgiDirective(const std::vector<std::string> &values, const std::string &filename, std::size_t line);
    void validateMethodsDirective(const std::vector<std::string> &values, const std::string &filename, std::size_t line);
    void validateErrorPageDirective(const std::vector<std::string> &values, const std::string &filename, std::size_t line);
    void validateListenDirective(const std::vector<std::string> &values, const std::string &filename, std::size_t line);
    void validateClientMaxBodySizeDirective(const std::vector<std::string> &values, const std::string &filename, std::size_t line, ConfigNode *node);
    void validateAutoindexDirective(const std::vector<std::string> &values, const std::string &filename, std::size_t line, ConfigNode *node);
    void validateRedirectDirective(const std::vector<std::string> &values, const std::string &filename, std::size_t line, ConfigNode *node);
    void validateDirectives(ConfigNode *node, const std::string &filename);
    void validateChildNodes(ConfigNode *node, const std::string &filename, int depth, std::map<int, size_t> &usedPorts);
    void validateServerPorts(ConfigNode *serverNode, const std::string &filename, std::map<int, size_t> &usedPorts);

    ConfigNode *parseConfigBlock(std::vector<Token> &tokens, size_t &index, ConfigNode *parent);
    bool parseDirective(std::vector<Token> &tokens, size_t &index, ConfigNode *currentNode);
    void displayTokens(const std::vector<Token> &tokens);
    
    ConfigNode* initConfigNode(std::vector<Token> &tokens, size_t &index, ConfigNode *parent);
    void handleLocationDirective(ConfigNode *node, std::vector<Token> &tokens, size_t &index);
    void checkOpeningBrace(std::vector<Token> &tokens, size_t &index, 
                           const std::string &keyword, std::size_t lineNumber);
    bool handleServerDirective(std::vector<Token> &tokens, size_t &index, ConfigNode *node);
    bool handleLocationDirective(std::vector<Token> &tokens, size_t &index, ConfigNode *node);
    void cleanInvalidFileDescriptors();
    std::string processRequest(int client_fd);
    void closeClientConnection(int clientFd);

public:
    Webserv();
    Webserv(std::string config_filename);
    Webserv(const Webserv &cpy);
    Webserv &operator=(const Webserv &rhs);
    ~Webserv();

    void acceptNewClient(Server *server); // Changed parameter to Server*
    void handleClients(pollfd &server);
    void handleServers(pollfd &client);
    void run();
    void storeServers(std::string &filename);
    void launchServers();
    void displayConfig(ConfigNode *node, int depth = 0);

    void setPollEvent(int fd, short events);
};

bool isPortAvailable(int port);

#endif