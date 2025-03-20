/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cviegas <cviegas@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/05 19:04:30 by ple-guya          #+#    #+#             */
/*   Updated: 2025/03/21 00:47:00 by cviegas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Webserv.hpp"
#include "../includes/Token.hpp"
#include <set>

// Liste des mots-clés reconnus - C++98 compatible initialization
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

// Vérifie si un token est un mot-clé
bool Webserv::isKeyword(const std::string &str) const
{
    return KEYWORDS.find(str) != KEYWORDS.end();
}

Webserv::Webserv()
{
}

Webserv::Webserv(std::string config_filename)
{
    storeServers(config_filename);
}

Webserv::Webserv(const Webserv &cpy)
{
    *this = cpy;
}

Webserv &Webserv::operator=(const Webserv &rhs)
{
    if (&rhs != this)
    {
        // Ajouter la copie des membres si nécessaire
    }
    return (*this);
}

Webserv::~Webserv()
{
}

// Fonction pour tokenizer une ligne
void Webserv::tokenizeLine(const std::string &line, std::size_t lineNum, std::vector<Token> &tokens,
                           bool &inQuoteSingle, bool &inQuoteDouble, std::string &pendingToken)
{
    for (std::size_t i = 0; i < line.length(); ++i)
    {
        char c = line[i];

        // Handle quotes - preserve whitespace inside quotes
        if (c == '"' && !inQuoteSingle)
        {
            if (inQuoteDouble)
            {
                // Fin d'une chaîne entre guillemets doubles
                tokens.push_back(Token(pendingToken, TOKEN_STRING, lineNum));
                pendingToken.clear();
            }
            else
            {
                // Début d'une chaîne entre guillemets doubles
                if (!pendingToken.empty())
                {
                    // Si nous avions un token en cours, l'ajouter d'abord
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
                // Fin d'une chaîne entre guillemets simples
                tokens.push_back(Token(pendingToken, TOKEN_STRING, lineNum));
                pendingToken.clear();
            }
            else
            {
                // Début d'une chaîne entre guillemets simples
                if (!pendingToken.empty())
                {
                    // Si nous avions un token en cours, l'ajouter d'abord
                    TokenType type = isKeyword(pendingToken) ? TOKEN_KEYWORD : TOKEN_ID;
                    tokens.push_back(Token(pendingToken, type, lineNum));
                    pendingToken.clear();
                }
            }
            inQuoteSingle = !inQuoteSingle;
            continue;
        }

        // Si en mode guillemets, ajouter le caractère au token actuel
        if (inQuoteSingle || inQuoteDouble)
        {
            pendingToken += c;
            continue;
        }

        // Handle comments - skip to the end of the line
        if (c == '#')
            break; // On sort de la boucle, on ignore le reste de la ligne

        // Special characters are individual tokens of type SYMBOL
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
        // Skip whitespace between tokens
        else if (isspace(c))
        {
            if (!pendingToken.empty())
            {
                TokenType type = isKeyword(pendingToken) ? TOKEN_KEYWORD : TOKEN_ID;
                tokens.push_back(Token(pendingToken, type, lineNum));
                pendingToken.clear();
            }
        }
        // Build token
        else
        {
            pendingToken += c;
        }
    }
}

// storeServers
void Webserv::storeServers(std::string &filename)
{
    std::ifstream file(filename.c_str());
    if (!file.is_open())
        throw std::runtime_error("Could not open config file");

    std::vector<Token> tokens;
    std::string line;
    std::string pendingToken;
    bool inQuoteSingle = false;
    bool inQuoteDouble = false;
    std::size_t lineNum = 0;

    while (std::getline(file, line))
    {
        lineNum++;
        tokenizeLine(line, lineNum, tokens, inQuoteSingle, inQuoteDouble, pendingToken);
    }

    file.close();

    // Si on a un token en attente à la fin du fichier, l'ajouter aussi
    if (!pendingToken.empty())
    {
        TokenType type = isKeyword(pendingToken) ? TOKEN_KEYWORD : TOKEN_ID;
        tokens.push_back(Token(pendingToken, type, lineNum));
    }

    // Vérifier si nous sommes toujours dans une chaîne quotée (erreur de syntaxe)
    if (inQuoteSingle || inQuoteDouble)
    {
        throw std::runtime_error("Unterminated quoted string at end of file");
    }

    // Display tokens for debugging
    std::cout << "Tokens found: " << tokens.size() << std::endl;
    for (std::size_t i = 0; i < tokens.size(); ++i)
    {
        std::cout << "Token " << i << ": ";
        tokens[i].display();
    }
}
