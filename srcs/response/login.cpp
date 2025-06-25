/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   login.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ple-guya <ple-guya@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/16 15:14:45 by ple-guya          #+#    #+#             */
/*   Updated: 2025/06/25 19:29:20 by ple-guya         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"
#include "Request.hpp"

void    Response::handleLogin(const Request &req, ConfigNode *locationNode)
{
    std::string body = req.getBody();
    std::string path = req.getPath();
    std::string root = findEffectiveRoot(locationNode);
    std::string location = root + path;
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
    if (!checkDB(location, username, password))
        return redirectTo("/login");
    createSession(username);
    redirectTo("/");
}

void    Response::createSession(std::string username)
{
    lastSession = this->_server->createSession(username);
    setCookie("session_id", lastSession.first);
    setCookie("user", username);
}

bool    Response::checkDB(std::string path, std::string username, std::string password)
{
    std::string                         locDB = path + "/database.txt";
    std::map<std::string, std::string>  users;
    std::string                         line;
    
    std::ifstream                       dataBase(locDB.c_str());
    if (dataBase.fail() || !dataBase.is_open())
        return false;

    while(std::getline(dataBase, line))
    {
        if (line.empty())
            continue;
        line = trimString(line, " \r\t");
        size_t pos = line.find (":");
        if (pos == std::string::npos)
            continue;
        std::string dbUser = line.substr(0, pos);
        std::string dbPassword = line.substr(pos + 1);
        if (username == dbUser && dbPassword == password)
        {
            dataBase.close();
            return true;
        }
    }
    dataBase.close();
    return false;
}

bool Response::checkLogin(const Request &req)
{    
    std::string cookieHeader = req.getHeader("Cookie");
    if (cookieHeader.empty())
        return false;
    extractCookie(cookieHeader);
    
    std::map<std::string, std::string>::iterator it = this->Cookies.find("session_id");
    if (it == this->Cookies.end())
        return false;
    
    std::string sessionID = it->second;
    if (sessionID.empty())
        return false;

    if (!_server->isActiveSession(sessionID)) 
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
    setCookie("session_id", "; Max-Age=0; path=/; HttpOnly");
    setCookie("user", "; Max-Age=0; path=/; HttpOnly");
}

void    Response::setCookie(const std::string &key, const std::string &value)
{
    this->Cookies.insert(std::make_pair(key, value));
}

void    Response::redirectTo(std::string const &path)
{
    status_code = "302 found";
    setHeaders("Location", path);
    setHeaders("Content-Type", "text/html");
    setBody("");  
}
