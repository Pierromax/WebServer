/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Tokenizer.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cviegas <cviegas@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/04 16:00:00 by cezou             #+#    #+#             */
/*   Updated: 2025/06/09 19:30:29 by cviegas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Webserv.hpp"
#include <set>
#include <sstream>

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
            pendingToken += c;
    }
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