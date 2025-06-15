
#include "Response.hpp"
#include "Request.hpp"
#include "Response.hpp"

std::string Response::getHttpStatusMessage(const std::string& statusCode) const
{
    if (statusCode == "301") return "Moved Permanently";
    if (statusCode == "302") return "Found";
    if (statusCode == "303") return "See Other";
    if (statusCode == "307") return "Temporary Redirect";
    if (statusCode == "308") return "Permanent Redirect";
    return "Found";
}

bool Response::checkForRedirect(ConfigNode* locationNode, const std::string& requestPath)
{
    (void)requestPath;
    if (!locationNode)
        return false;
    
    std::map<std::string, std::vector<std::string> >::iterator it = locationNode->directives.find("return");
    if (it != locationNode->directives.end() && it->second.size() == 2)
    {
        const std::string& statusCode = it->second[0];
        const std::string& redirectUrl = it->second[1];
        
        status_code = statusCode + " " + getHttpStatusMessage(statusCode);
        setHeaders("Location", redirectUrl);
        setBody("");
        return true;
    }
    return false;
}