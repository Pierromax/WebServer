/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Tokenizer.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cviegas <cviegas@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/04 16:00:00 by cezou             #+#    #+#             */
/*   Updated: 2025/06/23 20:15:36 by cviegas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Webserv.hpp"

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
    keywords.insert("cgi");
    keywords.insert("methods");
    keywords.insert("autoindex");
    keywords.insert("redirect");
    keywords.insert("AUTH_REQUIRED");
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
 * @brief Ajoute un token en attente s'il n'est pas vide
 * @param pendingToken Token en cours de construction
 * @param tokens Vector où stocker les tokens
 * @param lineNum Numéro de la ligne
 */
void Webserv::flushPendingToken(std::string &pendingToken, std::vector<Token> &tokens, std::size_t lineNum)
{
    if (!pendingToken.empty())
    {
        TokenType type = isKeyword(pendingToken) ? TOKEN_KEYWORD : TOKEN_ID;
        tokens.push_back(Token(pendingToken, type, lineNum));
        pendingToken.clear();
    }
}

/**
 * @brief Gère les guillemets (simples ou doubles) dans la tokenisation
 * @param pendingToken Token en cours de construction
 * @param tokens Vector où stocker les tokens
 * @param lineNum Numéro de la ligne
 * @param inQuote État des guillemets (référence vers inQuoteSingle ou inQuoteDouble)
 */
void Webserv::handleQuote(std::string &pendingToken, std::vector<Token> &tokens, 
                          std::size_t lineNum, bool &inQuote)
{
    if (inQuote)
    {
        tokens.push_back(Token(pendingToken, TOKEN_STRING, lineNum));
        pendingToken.clear();
    }
    else
        flushPendingToken(pendingToken, tokens, lineNum);
    inQuote = !inQuote;
}

/**
 * @brief Gère les symboles spéciaux ({, }, ;)
 * @param c Caractère à traiter
 * @param pendingToken Token en cours de construction
 * @param tokens Vector où stocker les tokens
 * @param lineNum Numéro de la ligne
 */
void Webserv::handleSymbol(char c, std::string &pendingToken, std::vector<Token> &tokens, std::size_t lineNum)
{
    flushPendingToken(pendingToken, tokens, lineNum);
    tokens.push_back(Token(std::string(1, c), TOKEN_SYMBOL, lineNum));
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
void Webserv::tokenizeLine(const std::string &line, std::size_t lineNum, std::vector<Token> &tokens,
                           bool &inQuoteSingle, bool &inQuoteDouble, std::string &pendingToken)
{
    for (std::size_t i = 0; i < line.length(); ++i)
    {
        char c = line[i];
        
        if (c == '"' && !inQuoteSingle)
        {
            handleQuote(pendingToken, tokens, lineNum, inQuoteDouble);
            continue;
        }
        else if (c == '\'' && !inQuoteDouble)
        {
            handleQuote(pendingToken, tokens, lineNum, inQuoteSingle);
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
            handleSymbol(c, pendingToken, tokens, lineNum);
        else if (isspace(c))
            flushPendingToken(pendingToken, tokens, lineNum);
        else
            pendingToken += c;
    }
}

/**
 * @brief Affiche les tokens trouvés dans le fichier de configuration
 * @param tokens Liste des tokens à afficher
 */
void Webserv::displayTokens(const std::vector<Token> &tokens)
{
    d_cout << "Tokens found: " << tokens.size() << std::endl;
    for (std::size_t i = 0; i < tokens.size(); ++i)
    {
        d_cout << "Token " << i << ": ";
        tokens[i].display();
    }
}