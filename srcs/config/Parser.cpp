/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cviegas <cviegas@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/04 16:10:02 by cezou             #+#    #+#             */
/*   Updated: 2025/06/25 18:47:19 by cviegas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Webserv.hpp"

/**
 * @brief Tokenize un fichier de configuration
 * @param filename Chemin vers le fichier
 * @return Vector de tokens bruts
 */
std::vector<Token> Webserv::tokenizeConfigFile(const std::string &filename)
{
    std::ifstream file(filename.c_str());
    std::vector<Token> tokens;
    std::string line;
    std::string pendingToken;
    bool inQuoteSingle = false;
    bool inQuoteDouble = false;
    std::size_t lineNum = 1;
    
    if (!file.is_open())
        throw std::runtime_error("Could not open config file");
    while (std::getline(file, line))
    {
        tokenizeLine(line, lineNum, tokens, inQuoteSingle, inQuoteDouble, pendingToken);
        lineNum++;
    }
    file.close();
    if (!pendingToken.empty())
    {
        TokenType type = isKeyword(pendingToken) ? TOKEN_KEYWORD : TOKEN_ID;
        tokens.push_back(Token(pendingToken, type, lineNum));
    }
    if (inQuoteSingle || inQuoteDouble)
        throw ParsingError("Unterminated quoted string at end of file", filename);
    return tokens;
}

/**
 * @brief Filtre les tokens non nécessaires et prépare pour le parsing
 * @param tokens Tokens bruts
 * @param filename Nom du fichier (pour les erreurs)
 * @return Vector de tokens filtrés
 */
std::vector<Token> Webserv::filterTokens(const std::vector<Token> &tokens, const std::string &filename)
{
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
        throw ParsingError("No valid configuration directives found in file", filename);
    return filteredTokens;
}

/**
 * @brief Analyse les tokens et construit l'arbre de configuration
 * @param tokens Tokens filtrés
 * @param filename Nom du fichier (pour les erreurs)
 * @return Pointeur vers le noeud racine
 */
ConfigNode* Webserv::parseTokens(std::vector<Token> &tokens, const std::string &filename)
{
    ConfigNode *root = new ConfigNode("root");
    size_t index = 0;
    
    try
    {
        while (index < tokens.size())
        {
            if (tokens[index].type != TOKEN_KEYWORD || tokens[index].value != "server")
            {
                if (tokens[index].type == TOKEN_ID)
                    throw ParsingError("unknown directive \"" + tokens[index].value + "\"", filename, tokens[index].line);
                index++;
                continue;
            }
            ConfigNode *serverNode = parseConfigBlock(tokens, index, root);
            if (serverNode)
                root->children.push_back(serverNode);
        }
        if (root->children.empty())
            throw ParsingError("No server config found", filename);
        return root;
    }
    catch (const std::exception &e)
    {
        delete root;
        throw;
    }
}

/**
 * @brief Initialise un nœud de configuration
 * @param tokens Liste des tokens
 * @param index Position actuelle
 * @param parent Nœud parent
 * @return Le nœud créé ou NULL en cas d'erreur
 */
ConfigNode* Webserv::initConfigNode(std::vector<Token> &tokens, size_t &index, ConfigNode *parent)
{
    if (index >= tokens.size())
        throw ParsingError("Unexpected end of configuration file", tokens[index-1].filename);
    if (tokens[index].type != TOKEN_KEYWORD)
        throw ParsingError("Expected keyword, got " + tokens[index].value, tokens[index].filename, tokens[index].line);
    std::size_t lineNumber = tokens[index].line;
    ConfigNode *node = new ConfigNode(tokens[index].value, "", parent, lineNumber);
    index++;
    return node;
}

/**
 * @brief Gère le cas spécial des directives de type location
 * @param node Nœud à compléter
 * @param tokens Liste des tokens
 * @param index Position actuelle
 */
void Webserv::handleLocationDirective(ConfigNode *node, std::vector<Token> &tokens, size_t &index)
{
    if (node->type != "location")
        return;
    if (index >= tokens.size() || tokens[index].type != TOKEN_ID)
        throw ParsingError("Expected path after 'location'", tokens[index].filename, tokens[index - 1].line);
    node->value = tokens[index].value;
    index++;
}

/**
 * @brief Vérifie la présence de l'accolade ouvrante
 * @param tokens Liste des tokens
 * @param index Position actuelle
 * @param keyword Mot-clé de la directive (pour les messages d'erreur)
 * @param lineNumber Numéro de ligne (pour les messages d'erreur)
 */
void Webserv::checkOpeningBrace(std::vector<Token> &tokens, size_t &index, 
                               const std::string &keyword, std::size_t lineNumber)
{
    if (index >= tokens.size() || tokens[index].type != TOKEN_SYMBOL || 
        tokens[index].value != "{")
        throw ParsingError("directive \"" + keyword + "\" has no opening \"{\"", tokens[index].filename, lineNumber);
    index++;
}

/**
 * @brief Traite une directive server à l'intérieur d'un bloc
 * @param tokens Liste des tokens
 * @param index Position actuelle
 * @param node Nœud parent
 * @return true si le traitement a réussi
 */
bool Webserv::handleServerDirective(std::vector<Token> &tokens, size_t &index, ConfigNode *node)
{
    if (tokens[index].value != "server")
        return false;
    if (node->parent != rootConfig)
        throw ParsingError("\"server\" directive is not allowed here", tokens[index].filename, tokens[index].line);
    ConfigNode *child = parseConfigBlock(tokens, index, node);
    if (child)
        node->children.push_back(child);
    return true;
}

/**
 * @brief Traite une directive location à l'intérieur d'un bloc
 * @param tokens Liste des tokens
 * @param index Position actuelle
 * @param node Nœud parent
 * @return true si le traitement a réussi
 */
bool Webserv::handleLocationDirective(std::vector<Token> &tokens, size_t &index, ConfigNode *node)
{
    if (tokens[index].value != "location")
        return false;
    ConfigNode *child = parseConfigBlock(tokens, index, node);
    if (child)
        node->children.push_back(child);
    return true;
}

/**
 * @brief Parse un bloc de configuration (server, location)
 * @param tokens Liste des tokens à analyser
 * @param index Position actuelle dans la liste des tokens
 * @param parent Noeud parent dans l'arborescence
 * @return Pointeur vers le nouveau noeud créé
 */
ConfigNode *Webserv::parseConfigBlock(std::vector<Token> &tokens, size_t &index, ConfigNode *parent)
{
    ConfigNode *node = NULL;
    std::string keyword;
    
    try
    {
        node = initConfigNode(tokens, index, parent);
        keyword = node->type;
        handleLocationDirective(node, tokens, index);
        checkOpeningBrace(tokens, index, keyword, node->line);
        while (index < tokens.size())
        {
            if (tokens[index].type == TOKEN_SYMBOL && tokens[index].value == "}")
            {
                index++;
                return node;
            }
            if (tokens[index].type == TOKEN_KEYWORD)
            {
                if (handleServerDirective(tokens, index, node) || 
                    handleLocationDirective(tokens, index, node))
                    continue;
                if (!parseDirective(tokens, index, node))
                    throw ParsingError("Failed to parse directive", tokens[index].filename, tokens[index].line);
            }
            else
                throw ParsingError("Unknown directive '" + tokens[index].value + "'", tokens[index].filename, tokens[index].line);
        }
        throw ParsingError("Missing closing brace '}'", tokens[index-1].filename, tokens[index-1].line);
    }
    catch (const std::exception &e)
    {
        if (node)
            delete node;
        throw;
    }
    return NULL;
}

/**
 * @brief Parse une directive simple (clé-valeur terminée par ";")
 * @param tokens Liste des tokens
 * @param index Position actuelle dans la liste
 * @param currentNode Noeud où stocker la directive
 * @return true si parsing réussi, false sinon
 */
bool Webserv::parseDirective(std::vector<Token> &tokens, size_t &index, ConfigNode *currentNode)
{
    std::string key = tokens[index].value;
    std::size_t directiveLine = tokens[index].line; // Stocker le numéro de ligne de la directive
    index++;
    std::vector<std::string> values;
    
    while (index < tokens.size() &&
           !(tokens[index].type == TOKEN_SYMBOL && tokens[index].value == ";"))
    {
        if (tokens[index].type == TOKEN_ID || tokens[index].type == TOKEN_STRING)
            values.push_back(tokens[index].value);
        else if (tokens[index].type == TOKEN_SYMBOL && tokens[index].value == "{")
            throw ParsingError("Unexpected '{' in directive", tokens[index].filename, tokens[index].line);
        index++;
    }
    if (index >= tokens.size() || tokens[index].value != ";") {
        std::string file = index < tokens.size() ? 
            tokens[index].filename : tokens[index-1].filename;
        std::size_t line = index < tokens.size() ? 
            tokens[index].line : tokens[index-1].line;
        throw ParsingError("Missing semicolon ';' for directive '" + key + "'", file, line);
    }
    index++;

    currentNode->directiveLines[key] = directiveLine;

    if (key == "cgi")
    {
        if (values.size() != 2)
            throw ParsingError("directive \"cgi\" takes exactly 2 arguments", tokens[index-1].filename, directiveLine);
        currentNode->cgiHandlers[values[0]] = values[1];
    }
    else if (key == "listen")
    {
        if (currentNode->type != "server")
            throw ParsingError(key + " directive is not allowed here", tokens[index-1].filename, directiveLine);
        for (size_t i = 0; i < values.size(); ++i)
        {
            std::string host = "";
            int port = 0;
            size_t colonPos = values[i].find(':');
            
            if (colonPos != std::string::npos)
            {
                host = values[i].substr(0, colonPos);
                std::string portStr = values[i].substr(colonPos + 1);
                std::stringstream ss(portStr);
                std::string remaining;
                
                if (!(ss >> port) || ss >> remaining)
                    throw ParsingError("invalid port \"" + values[i] + "\" in directive \"listen\"", tokens[index-1].filename, directiveLine);
            }
            else
            {
                std::stringstream ss(values[i]);
                std::string remaining;
                
                if (!(ss >> port) || ss >> remaining)
                    throw ParsingError("invalid port \"" + values[i] + "\" in directive \"listen\"", tokens[index-1].filename, directiveLine);
            }
            if (port < 1024 || port > 65535)
                throw ParsingError("port " + values[i] + " out of range (1024-65535) in directive \"listen\"", tokens[index-1].filename, directiveLine);
            currentNode->listenPairs.push_back(std::make_pair(host, port));
            currentNode->directives[key].push_back(values[i]);
        }
    }
    else if (key == "AUTH_REQUIRED")
    {
        if (currentNode->type == "server")
            throw ParsingError(key + " directive is not allowed at root", tokens[index-1].filename, directiveLine);
        if (values.size())
            throw ParsingError("AUTH_REQUIRED don't need arguments", tokens[index-1].filename, directiveLine);
        currentNode->isAuthRequired = true;


    }
    else if (key == "methods")
    {
        currentNode->directives[key] = values;
        for (int i = 0; i < METHOD_COUNT; ++i)
            currentNode->allowedMethods[i] = false;
        for (size_t i = 0; i < values.size(); ++i)
        {
            if (values[i] == "GET")
                currentNode->allowedMethods[METHOD_GET] = true;
            else if (values[i] == "POST")
                currentNode->allowedMethods[METHOD_POST] = true;
            else if (values[i] == "DELETE")
                currentNode->allowedMethods[METHOD_DELETE] = true;
        }
    }
    else
        currentNode->directives[key] = values;
    return true;
}
