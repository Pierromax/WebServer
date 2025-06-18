/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   login.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ple-guya <ple-guya@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/16 15:14:45 by ple-guya          #+#    #+#             */
/*   Updated: 2025/06/18 17:56:34 by ple-guya         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"
#include "Request.hpp"
#include "Response.hpp"

void    Response::handleLogin(const Request &req, ConfigNode *locationNode)
{
    std::string body = req.getBody();
    std::string username;
    std::string password;
    size_t      delimPos;

    delimPos = body.find("&");
    if (delimPos == std::string::npos)
        return;
        
    size_t userPos = body.find("username=") + 9;
    if (userPos != std::string::npos)
        username = body.substr(userPos, delimPos - 9);
        
    size_t passPos = body.find("password=");
    if (passPos != std::string::npos)
        password = body.substr(passPos + 9);

    
}



bool    Response::checkLogin(const Request &req)
{
    extractCookie(req.getHeader("Cookies"));
    if (this->Cookies.empty())
        return false;
    return true;
}

/* ********************* */
/*   Cookie gestion      */
/* ********************* */

void     Response::extractCookie(const std::string &cookie)
{
    std::map<std::string, std::string>  splitCookie;
    std::stringstream                   ss(cookie);
    std::string                         line;
    
    while(std::getline(ss, line, ';'))
    {
        if (line.empty())
            continue;
        size_t pos = line.find("=");
        if(pos == std::string::npos)
            continue;
        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);
        key = trimString(key, " \t\n\r");
        value = trimString(value, " \t\n\r");
        splitCookie[key] = value;
    }
    this->Cookies = splitCookie;
}

void    Response::deleteCookie()
{
    Cookies.clear();
}
