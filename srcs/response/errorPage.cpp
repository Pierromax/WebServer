#include "Response.hpp"
#include "Request.hpp"
#include "Response.hpp"

std::string Response::findErrorPageUri(const std::string& errorCode, ConfigNode* locationNode, ConfigNode*& directiveContext) const
{
    ConfigNode* searchNode = locationNode;
    directiveContext = NULL;
    while (searchNode)
    {
        std::map<std::string, std::vector<std::string> >::iterator it = searchNode->directives.find("error_page");
        if (it != searchNode->directives.end())
        {
            const std::vector<std::string>& values = it->second;
            for (size_t i = 0; i < values.size() - 1; ++i)
            {
                if (values[i] == errorCode)
                {
                    directiveContext = searchNode;
                    return values.back();
                }
            }
        }
        searchNode = searchNode->parent;
    }
    return "";
}

std::string Response::resolveErrorPagePath(const std::string& errorPageUri, ConfigNode* directiveContext) const
{
    std::string fullPath = "";
    std::string locationPath = "";

    if (errorPageUri.empty())
        return "";
    if (errorPageUri[0] == '/')
    {
        if (!_server)
            return "";
		fullPath = findEffectiveRoot(_server->getConfigNode());
    }
    else
    {
        fullPath = findEffectiveRoot(directiveContext);
        if (directiveContext && directiveContext->type == "location")
            locationPath = directiveContext->value;
    }
    if (fullPath.empty())
        return "";
    if (!locationPath.empty())
    {
        if (!fullPath.empty() && fullPath[fullPath.length() - 1] != '/' && locationPath[0] != '/')
            fullPath += '/';
        else if (!fullPath.empty() && fullPath[fullPath.length() - 1] == '/' && locationPath[0] == '/')
            locationPath = locationPath.substr(1);
        fullPath += locationPath;
    }

    if (!fullPath.empty() && fullPath[fullPath.length() - 1] != '/')
        fullPath += '/';

    if (!errorPageUri.empty())
    {
        if (errorPageUri[0] == '/')
            fullPath += errorPageUri.substr(1);
        else
            fullPath += errorPageUri;
    }
    return fullPath;
}

void Response::generateDefaultErrorPage(const std::string& errorCode)
{
    (void)errorCode;
    std::string defaultBody = "<html><head><title>" + status_code + "</title></head>"
                              "<body><h1>" + status_code + "</h1></body></html>";
    setBody(defaultBody);
    setContentType("text/html");
}

void Response::loadErrorPage(const std::string& errorCode, ConfigNode* locationNode)
{
    ConfigNode* directiveContext = NULL;
    std::string errorPageUri = findErrorPageUri(errorCode, locationNode, directiveContext);
    std::string errorPagePath = "";
    std::string content = "";

    if (!errorPageUri.empty())
        errorPagePath = resolveErrorPagePath(errorPageUri, directiveContext);

    if (!errorPagePath.empty() && loadPageContent(errorPagePath, content))
    {
        setBody(content);
        setContentType(getMimeType(errorPagePath));
    }
    else
        generateDefaultErrorPage(errorCode);
}
