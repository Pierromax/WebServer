/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cezou <cezou@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/05 19:04:30 by ple-guya          #+#    #+#             */
/*   Updated: 2025/04/04 15:54:40 by cezou            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Webserv.hpp"
#include <set>
#include <sstream>

/**
 * @brief Initialise les mots-clés reconnus par le parser
 * @return Set contenant tous les mots-clés valides
 */
std::set<std::string> initKeywords()
{
    std::set<std::string> keywords;
    keywords.insert("server");
    keywords.insert("location");
    keywords.insert("listen");
    keywords.insert("server_name");
    keywords.insert("root");
    keywords.insert("index");
    keywords.insert("error_page");
    keywords.insert("client_max_body_size");
    keywords.insert("return");
    return keywords;
}

const std::set<std::string> KEYWORDS = initKeywords();

/**
 * @brief Vérifie si une chaîne est un mot-clé reconnu
 * @param str Chaîne à vérifier
 * @return true si c'est un mot-clé, false sinon
 */
bool Webserv::isKeyword(const std::string &str) const
{
    return KEYWORDS.find(str) != KEYWORDS.end();
}

/**
 * @brief Constructeur par défaut
 */
Webserv::Webserv() : rootConfig(NULL)
{
}

/**
 * @brief Constructeur avec fichier de configuration
 * @param config_filename Chemin vers le fichier de configuration
 */
Webserv::Webserv(std::string config_filename) : rootConfig(NULL)
{
    storeServers(config_filename);
}

/**
 * @brief Constructeur de copie
 * @param cpy Objet à copier
 */
Webserv::Webserv(const Webserv &cpy) : rootConfig(NULL)
{
    *this = cpy;
}

/**
 * @brief Opérateur d'assignation
 * @param rhs Objet à copier
 * @return Référence vers cet objet
 */
Webserv &Webserv::operator=(const Webserv &rhs)
{
    if (this != &rhs)
    {
        if (rootConfig)
        {
            delete rootConfig;
            rootConfig = NULL;
        }
    }
    return (*this);
}

/**
 * @brief Destructeur
 */
Webserv::~Webserv()
{
    if (rootConfig)
    {
        delete rootConfig;
    }
}

/**
 * @brief Tokenize une ligne du fichier de configuration
 * @param line Ligne à analyser
 * @param lineNum Numéro de la ligne
 * @param tokens Vector où stocker les tokens trouvés
 * @param inQuoteSingle État des guillemets simples
 * @param inQuoteDouble État des guillemets doubles
 * @param pendingToken Token en cours de construction
 */
void Webserv::tokenizeLine(const std::string &line, std::size_t lineNum,
                          std::vector<Token> &tokens, bool &inQuoteSingle,
                          bool &inQuoteDouble, std::string &pendingToken)
{
    for (std::size_t i = 0; i < line.length(); ++i)
    {
        char c = line[i];
        if (c == '"' && !inQuoteSingle)
        {
            if (inQuoteDouble)
            {
                tokens.push_back(Token(pendingToken, TOKEN_STRING, lineNum));
                pendingToken.clear();
            }
            else
            {
                if (!pendingToken.empty())
                {
                    TokenType type = isKeyword(pendingToken) ? TOKEN_KEYWORD : TOKEN_ID;
                    tokens.push_back(Token(pendingToken, type, lineNum));
                    pendingToken.clear();
                }
            }
            inQuoteDouble = !inQuoteDouble;
            continue;
        }
        else if (c == '\'' && !inQuoteDouble)
        {
            if (inQuoteSingle)
            {
                tokens.push_back(Token(pendingToken, TOKEN_STRING, lineNum));
                pendingToken.clear();
            }
            else
            {
                if (!pendingToken.empty())
                {
                    TokenType type = isKeyword(pendingToken) ? TOKEN_KEYWORD : TOKEN_ID;
                    tokens.push_back(Token(pendingToken, type, lineNum));
                    pendingToken.clear();
                }
            }
            inQuoteSingle = !inQuoteSingle;
            continue;
        }
        if (inQuoteSingle || inQuoteDouble)
        {
            pendingToken += c;
            continue;
        }
        if (c == '#')
            break;
        if (c == '{' || c == '}' || c == ';')
        {
            if (!pendingToken.empty())
            {
                TokenType type = isKeyword(pendingToken) ? TOKEN_KEYWORD : TOKEN_ID;
                tokens.push_back(Token(pendingToken, type, lineNum));
                pendingToken.clear();
            }
            tokens.push_back(Token(std::string(1, c), TOKEN_SYMBOL, lineNum));
        }
        else if (isspace(c))
        {
            if (!pendingToken.empty())
            {
                TokenType type = isKeyword(pendingToken) ? TOKEN_KEYWORD : TOKEN_ID;
                tokens.push_back(Token(pendingToken, type, lineNum));
                pendingToken.clear();
            }
        }
        else
        {
            pendingToken += c;
        }
    }
}

/**
 * @brief Parse un bloc de configuration (server, location)
 * @param tokens Liste des tokens à analyser
 * @param index Position actuelle dans la liste des tokens
 * @param parent Noeud parent dans l'arborescence
 * @return Pointeur vers le nouveau noeud créé
 */
ConfigNode *Webserv::parseConfigBlock(std::vector<Token> &tokens, size_t &index,
                                    ConfigNode *parent)
{
    if (index >= tokens.size())
        throw std::runtime_error("Unexpected end of configuration file");
    if (tokens[index].type != TOKEN_KEYWORD)
    {
        std::stringstream err;
        err << "Expected keyword in " << tokens[index].filename << ":" 
            << tokens[index].line << ", got " << tokens[index].value;
        throw std::runtime_error(err.str());
    }
    std::size_t lineNumber = tokens[index].line;
    std::string keyword = tokens[index].value;
    ConfigNode *node = new ConfigNode(tokens[index].value, "", parent, lineNumber);
    index++;
    
    try
    {
        if (node->type == "location")
        {
            if (index >= tokens.size() || tokens[index].type != TOKEN_ID)
            {
                std::stringstream err;
                err << "Expected path after 'location' in " 
                    << tokens[index].filename << ":" 
                    << tokens[index - 1].line;
                throw std::runtime_error(err.str());
            }
            node->value = tokens[index].value;
            index++;
        }
        if (index >= tokens.size() || tokens[index].type != TOKEN_SYMBOL || 
            tokens[index].value != "{")
        {
            std::stringstream err;
            err << "directive \"" << keyword << "\" has no opening \"{\" in " 
                << tokens[index].filename << ":" << lineNumber;
            throw std::runtime_error(err.str());
        }
        index++;
        while (index < tokens.size())
        {
            if (tokens[index].type == TOKEN_SYMBOL && tokens[index].value == "}")
            {
                index++;
                return node;
            }
            if (tokens[index].type == TOKEN_KEYWORD)
            {
                if (tokens[index].value == "server")
                {
                    if (parent != rootConfig)
                    {
                        std::stringstream err;
                        err << "\"server\" directive is not allowed here in " 
                            << tokens[index].filename << ":" << tokens[index].line;
                        throw std::runtime_error(err.str());
                    }
                    ConfigNode *child = parseConfigBlock(tokens, index, node);
                    if (child)
                        node->children.push_back(child);
                }
                else if (tokens[index].value == "location")
                {
                    ConfigNode *child = parseConfigBlock(tokens, index, node);
                    if (child)
                        node->children.push_back(child);
                }
                else
                {
                    if (!parseDirective(tokens, index, node))
                        throw std::runtime_error("Failed to parse directive");
                }
            }
            else
            {
                std::stringstream err;
                err << "Unknown directive '" << tokens[index].value << "' in " 
                    << tokens[index].filename << ":" << tokens[index].line;
                throw std::runtime_error(err.str());
            }
        }
        throw std::runtime_error("Missing closing brace '}'");
    }
    catch (const std::exception &e)
    {
        delete node;
        throw;
    }
    delete node;
    return NULL;
}

/**
 * @brief Parse une directive simple (clé-valeur terminée par ";")
 * @param tokens Liste des tokens
 * @param index Position actuelle dans la liste
 * @param currentNode Noeud où stocker la directive
 * @return true si parsing réussi, false sinon
 */
bool Webserv::parseDirective(std::vector<Token> &tokens, size_t &index,
                           ConfigNode *currentNode)
{
    std::string key = tokens[index].value;
    index++;
    std::vector<std::string> values;

    while (index < tokens.size() &&
           !(tokens[index].type == TOKEN_SYMBOL && tokens[index].value == ";"))
    {
        if (tokens[index].type == TOKEN_ID || tokens[index].type == TOKEN_STRING)
        {
            values.push_back(tokens[index].value);
        }
        else if (tokens[index].type == TOKEN_SYMBOL && 
                tokens[index].value == "{")
        {
            std::stringstream err;
            err << "Unexpected '{' in directive in " << tokens[index].filename 
                << ":" << tokens[index].line;
            throw std::runtime_error(err.str());
        }
        index++;
    }
    if (index >= tokens.size() || tokens[index].value != ";")
    {
        std::stringstream err;
        std::string file = index < tokens.size() ? 
            tokens[index].filename : tokens[index-1].filename;
        std::size_t line = index < tokens.size() ? 
            tokens[index].line : tokens[index-1].line;
        err << "Missing semicolon ';' for directive '" << key << "' in " 
            << file << ":" << line;
        throw std::runtime_error(err.str());
    }
    index++;
    currentNode->directives[key] = values;
    return true;
}

/**
 * @brief Affiche la configuration parsée (pour debug)
 * @param node Noeud à afficher
 * @param depth Niveau d'indentation
 */
void Webserv::displayConfig(ConfigNode *node, int depth)
{
    if (!node)
        return;
    std::string indent(depth * 4, ' ');
    std::cout << indent << node->type;
    if (!node->value.empty())
    {
        std::cout << " " << node->value;
    }
    std::cout << " {" << std::endl;
    for (std::map<std::string, std::vector<std::string> >::const_iterator it = 
        node->directives.begin(); it != node->directives.end(); ++it)
    {
        std::cout << indent << "    " << it->first << " ";
        const std::vector<std::string> &values = it->second;
        for (size_t i = 0; i < values.size(); ++i)
        {
            std::cout << values[i] << " ";
        }
        std::cout << ";" << std::endl;
    }
    for (size_t i = 0; i < node->children.size(); ++i)
    {
        displayConfig(node->children[i], depth + 1);
    }
    std::cout << indent << "}" << std::endl;
}

/**
 * @brief Affiche les tokens trouvés dans le fichier de configuration
 * @param tokens Liste des tokens à afficher
 */
void Webserv::displayTokens(const std::vector<Token> &tokens)
{
    std::cout << "Tokens found: " << tokens.size() << std::endl;
    for (std::size_t i = 0; i < tokens.size(); ++i)
    {
        std::cout << "Token " << i << ": ";
        tokens[i].display();
    }
}

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
        {
            validateNoNestedServers(node->children[i], filename);
        }
    }
}

/**
 * @brief Charge la configuration depuis un fichier et crée les serveurs
 * @param filename Chemin vers le fichier de configuration
 */
void Webserv::storeServers(std::string &filename)
{
    if (rootConfig)
    {
        delete rootConfig;
        rootConfig = NULL;
    }
    std::ifstream file(filename.c_str());
    if (!file.is_open())
        throw std::runtime_error("Could not open config file");
    std::vector<Token> tokens;
    std::string line;
    std::string pendingToken;
    bool inQuoteSingle = false;
    bool inQuoteDouble = false;
    std::size_t lineNum = 1;

    while (std::getline(file, line))
    {
        tokenizeLine(line, lineNum, tokens, inQuoteSingle, inQuoteDouble, 
                    pendingToken);
        lineNum++;
    }
    file.close();
    if (!pendingToken.empty())
    {
        TokenType type = isKeyword(pendingToken) ? TOKEN_KEYWORD : TOKEN_ID;
        tokens.push_back(Token(pendingToken, type, lineNum));
    }
    if (inQuoteSingle || inQuoteDouble)
    {
        throw std::runtime_error("Unterminated quoted string at end of file");
    }
    std::vector<Token> filteredTokens;
    for (std::size_t i = 0; i < tokens.size(); ++i)
    {
        if (tokens[i].type != TOKEN_STRING)
        {
            filteredTokens.push_back(tokens[i]);
            filteredTokens.back().filename = filename;
        }
    }
    if (filteredTokens.empty())
    {
        throw std::runtime_error("No valid configuration directives found in file");
    }
    rootConfig = new ConfigNode("root");
    size_t index = 0;
    try
    {
        while (index < filteredTokens.size())
        {
            if (filteredTokens[index].type != TOKEN_KEYWORD || 
                filteredTokens[index].value != "server")
            {
                if (filteredTokens[index].type == TOKEN_ID)
                {
                    std::stringstream err;
                    err << "unknown directive \"" << filteredTokens[index].value
                        << "\" in " << filename << ":" 
                        << filteredTokens[index].line;
                    throw std::runtime_error(err.str());
                }
                index++;
                continue;
            }
            ConfigNode *serverNode = parseConfigBlock(filteredTokens, index, 
                                                     rootConfig);
            if (serverNode)
            {
                rootConfig->children.push_back(serverNode);
            }
        }
        if (rootConfig->children.empty())
        {
            throw std::runtime_error("No server config found");
        }
        validateNoDuplicateLocations(rootConfig, filename);
        validateNoNestedServers(rootConfig, filename);
        std::cout << "\nParsed Configuration:" << std::endl;
        displayConfig(rootConfig);
        buildServers(rootConfig);
    }
    catch (const std::exception &e)
    {
        if (rootConfig)
        {
            delete rootConfig;
            rootConfig = NULL;
        }
        throw std::runtime_error(e.what());
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
        {
            throw std::runtime_error("Expected 'server' block");
        }
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
        {
            server.port = DEFAULT_PORT;
        }
        it = serverNode->directives.find("server_name");
        if (it != serverNode->directives.end())
        {
            server.serverNames = it->second;
        }
        for (size_t j = 0; j < serverNode->children.size(); ++j)
        {
            ConfigNode *locationNode = serverNode->children[j];
            if (locationNode->type != "location")
            {
                continue;
            }
            t_Route route;
            route.directoryListing = false;
            route.allowUploads = false;
            it = locationNode->directives.find("root");
            if (it != locationNode->directives.end() && !it->second.empty())
            {
                route.root = it->second[0];
            }
            it = locationNode->directives.find("index");
            if (it != locationNode->directives.end() && !it->second.empty())
            {
                route.defaultFile = it->second[0];
            }
            server.routes[locationNode->value] = route;
        }
        std::cout << "Added server on port " << server.port << " with " 
                  << server.routes.size() << " locations" << std::endl;
        serveurs[server.port] = server;
    }
}