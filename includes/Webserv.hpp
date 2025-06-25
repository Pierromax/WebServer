/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ple-guya <ple-guya@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/21 03:15:00 by cviegas           #+#    #+#             */
/*   Updated: 2025/06/25 22:12:16 by ple-guya         ###   ########.fr       */
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
#include <csignal> 
#include <set>
#include <cstdlib>
#include <sys/stat.h>


#ifndef CONFIG_TESTER
# define CONFIG_TESTER 0
#endif

#ifndef DEBUG
# define DEBUG 0
#endif

#define DEFAULT_PORT 8080

/**
 * @brief Debug stream class that only outputs when DEBUG is enabled
 */
class DebugStream
{
public:
    template<typename T>
    DebugStream& operator<<(const T& value)
    {
        if (DEBUG)
            std::cout << value;
        return *this;
    }
    
    DebugStream& operator<<(std::ostream& (*manip)(std::ostream&))
    {
        if (DEBUG)
            std::cout << manip;
        return *this;
    }
};

extern DebugStream d_cout;

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
        d_cout << "[" << typeStr << "] " << value << " (line " << line << ")" << std::endl;
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
    bool isAuthRequired;
    size_t client_max_body_size;
    bool autoindex;
    std::map<std::string, std::string> cgiHandlers;
    std::vector<std::pair<std::string, int> > listenPairs;

    ConfigNode(const std::string &t = "", const std::string &v = "", ConfigNode *p = NULL, std::size_t ln = 0)
        : type(t), value(v), parent(p), line(ln)
    {
        if (p != NULL)
        {
            directives = p->directives;
            directiveLines = p->directiveLines;
            client_max_body_size = p->client_max_body_size;
            autoindex = p->autoindex;
            cgiHandlers = p->cgiHandlers;
            listenPairs = p->listenPairs;
            for (int i = 0; i < METHOD_COUNT; ++i)
                allowedMethods[i] = p->allowedMethods[i];
            isAuthRequired = p->isAuthRequired;
        }
        else
        {
            client_max_body_size = 1048576;
            autoindex = false;
            isAuthRequired = false;
            for (int i = 0; i < METHOD_COUNT; ++i)
                allowedMethods[i] = true;
        }
    }

    ConfigNode(const ConfigNode &other)
        : type(other.type), value(other.value), parent(other.parent), line(other.line)
    {
        directives = other.directives;
        directiveLines = other.directiveLines;
        client_max_body_size = other.client_max_body_size;
        autoindex = other.autoindex;
        cgiHandlers = other.cgiHandlers;
        listenPairs = other.listenPairs;
        isAuthRequired = other.isAuthRequired;
        for (int i = 0; i < METHOD_COUNT; ++i)
            allowedMethods[i] = other.allowedMethods[i];
        for (size_t i = 0; i < other.children.size(); ++i)
            children.push_back(new ConfigNode(*other.children[i]));
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
    std::vector<Server*> orderedServers;
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

    // Tokenizer helper functions
    void flushPendingToken(std::string &pendingToken, std::vector<Token> &tokens, std::size_t lineNum);
    void handleQuote(std::string &pendingToken, std::vector<Token> &tokens, std::size_t lineNum, bool &inQuote);
    void handleSymbol(char c, std::string &pendingToken, std::vector<Token> &tokens, std::size_t lineNum);

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
    Server* findBestServer(int port, const std::string& hostHeader);
};

bool isPortAvailable(int port);

// --- Location Finding Helpers (moved from Response) ---
ConfigNode* findBestLocation(const std::string& requestPath, Server* server);
ConfigNode* findBestLocationRecursive(const std::string& requestPath, ConfigNode* currentNode, ConfigNode* bestMatch, const std::string& currentPath);

#endif