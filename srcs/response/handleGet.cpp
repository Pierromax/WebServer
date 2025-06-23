#include "Response.hpp"
#include "Request.hpp"
#include "Response.hpp"

void Response::handleGetRequest(const Request &req)
{
    std::string path = req.getPath();
    ConfigNode* locationNode = findBestLocation(path, _server);
    std::string content;
    
    d_cout << "URL demandée: " << req.getPath() << std::endl;

    if (!isMethodAllowed("GET", locationNode))
    {
        status_code = METHOD_NOT_ALLOWED;
        loadErrorPage("405", locationNode);
        return;
    }

    if (req.getStatusCode() == PAYLOAD_TOO_LARGE)
    {
        status_code = PAYLOAD_TOO_LARGE;
        loadErrorPage("413", locationNode);
        return;
    }

    if (checkForRedirect(locationNode, path))
        return;

    if (path == "/logout")
        return handleLogout(req);

    std::string filePath = resolveFilePath(locationNode, path);
    d_cout << "in complet du fichier: " << filePath << std::endl;

    if (filePath.empty())
    {
        std::string fullPath = buildFullPath(locationNode, path);
        if (!fullPath.empty())
        {
            struct stat path_stat;
            if (stat(fullPath.c_str(), &path_stat) == 0 && S_ISDIR(path_stat.st_mode))
            {
                if (shouldGenerateAutoindex(locationNode))
                {
                    std::string directoryListing = generateDirectoryListing(fullPath, path);
                    setBody(directoryListing);
                    setContentType("text/html");
                    return;
                }
            }
        }
        status_code = FILE_NOT_FOUND;
        loadErrorPage("404", locationNode);
        return;
    }

    CGIsHandling* cgiHandler = CGIsHandling::findCgiHandler(filePath, locationNode);
    if (cgiHandler)
    {
        delete cgiHandler;
        handleCgiRequest(req, locationNode, filePath);
    }
    else if (loadPageContent(filePath, content))
    {
        setBody(content);
        setContentType(getMimeType(filePath));
    }
    else
    {
        status_code = "403 Forbidden";
        loadErrorPage("403", locationNode);
    }
}

void Response::handleLogout(const Request &req)
{    
    std::string cookieHeader = req.getHeader("Cookie");
    if (!cookieHeader.empty()) 
    {
        extractCookie(cookieHeader);
        std::map<std::string, std::string>::iterator it = Cookies.find("session_id");
        if (it != Cookies.end())
            _server->deleteSession(it->second);
    }
    deleteCookie();
    redirectTo("/");
}
