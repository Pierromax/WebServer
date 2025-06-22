/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   login.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ple-guya <ple-guya@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/16 15:14:45 by ple-guya          #+#    #+#             */
/*   Updated: 2025/06/22 03:30:29 by ple-guya         ###   ########.fr       */
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

    if (checkDB(location, username, password))
    {
        createSession(username);
        status_code = "302 found";
        setHeaders("location", "/");
        setHeaders("Content-Type", "text/html");
        setBody("");
    }
}

void    Response::createSession(std::string username)
{
    lastSession = this->_server->createSession(username);
    setCookie("session_id", lastSession.first);
    setCookie("Max_Age", intToString(lastSession.second.max_age));
    setCookieUser();
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
    return false;
}

bool Response::checkLogin(const Request &req)
{
    std::cout << "=== DÉBUT checkLogin ===" << std::endl;
    
    std::string cookieHeader = req.getHeader("Cookie");
    if (cookieHeader.empty()) {
        std::cout << "Aucun cookie dans la requête" << std::endl;
        return false;
    }
    
    extractCookie(cookieHeader);
    
    std::map<std::string, std::string>::iterator it = this->Cookies.find("session_id");
    if (it == this->Cookies.end()) {
        std::cout << "Cookie session_id non trouvé" << std::endl;
        return false;
    }
    
    std::string sessionID = it->second;
    if (sessionID.empty()) {
        std::cout << "SessionID vide" << std::endl;
        return false;
    }
    
    std::cout << "SessionID trouvé: " << sessionID << std::endl;
    
    if (!_server->isActiveSession(sessionID)) {
        std::cout << "Session invalide ou expirée: " << sessionID << std::endl;
        return false;
    }
    
    std::cout << "Session valide: " << sessionID << std::endl;
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
    setHeaders("Set-Cookie", "session_id=; expires=Thu, 01 Jan 1970 00:00:00 GMT; path=/; HttpOnly");
}

void    Response::setCookie(const std::string &key, const std::string &value)
{
    this->Cookies.insert(std::make_pair(key, value));
}

void        Response::setCookieUser()
{
    std::map<std::string, std::string>::iterator it = Cookies.begin();
    std::string cookieLine;
    
    cookieLine = it->first + "=" + it->second + ";";
    it++;
    
    for (; it != Cookies.end(); it++)
        cookieLine.append(" " + it->first + "=" + it->second + ";");
        
    setHeaders("Set-Cookie", cookieLine);
}
