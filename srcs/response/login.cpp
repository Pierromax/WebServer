/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   login.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ple-guya <ple-guya@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/16 15:14:45 by ple-guya          #+#    #+#             */
/*   Updated: 2025/06/16 16:51:05 by ple-guya         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"
#include "Request.hpp"
#include "Response.hpp"


bool    Response::checkLogin(const Request &req)
{
    extractCookie(req.getHeader("Cookies"));
    
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