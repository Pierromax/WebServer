
#include "Response.hpp"
#include "Request.hpp"
#include "Response.hpp"

std::string Response::getMimeType(const std::string& filePath) const
{
    std::map<std::string, std::string> mimeTypes;
    mimeTypes[".html"] = "text/html";
    mimeTypes[".htm"] = "text/html";
    mimeTypes[".css"] = "text/css";
    mimeTypes[".js"] = "application/javascript";
    mimeTypes[".json"] = "application/json";
    mimeTypes[".jpg"] = "image/jpeg";
    mimeTypes[".jpeg"] = "image/jpeg";
    mimeTypes[".png"] = "image/png";
    mimeTypes[".gif"] = "image/gif";
    mimeTypes[".ico"] = "image/x-icon";
    mimeTypes[".txt"] = "text/plain";

    size_t dotPos = filePath.rfind('.');
    if (dotPos != std::string::npos)
    {
        std::string ext = filePath.substr(dotPos);
        if (mimeTypes.count(ext))
            return mimeTypes[ext];
    }
    return "application/octet-stream";
}

std::string Response::findEffectiveRoot(ConfigNode* contextNode) const
{
    ConfigNode* searchNode = contextNode;
    while (searchNode)
    {
        std::map<std::string, std::vector<std::string> >::iterator it = searchNode->directives.find("root");
        if (it != searchNode->directives.end() && !it->second.empty())
            return it->second[0];
        searchNode = searchNode->parent;
    }
    return "";
}

std::vector<std::string> Response::findEffectiveIndexFiles(ConfigNode* contextNode) const
{
    ConfigNode* searchNode = contextNode;
    while (searchNode)
    {
        std::map<std::string, std::vector<std::string> >::iterator it = searchNode->directives.find("index");
        if (it != searchNode->directives.end() && !it->second.empty())
            return it->second;
        searchNode = searchNode->parent;
    }
    return std::vector<std::string>();
}

std::string Response::tryIndexFiles(const std::string& directoryPath, const std::vector<std::string>& indexFiles) const
{
    for (size_t i = 0; i < indexFiles.size(); ++i)
    {
        std::string indexedPath = directoryPath;
        if (!indexedPath.empty() && indexedPath[indexedPath.length() - 1] != '/')
            indexedPath += "/";
        indexedPath += indexFiles[i];

        struct stat path_stat;
        if (stat(indexedPath.c_str(), &path_stat) == 0 && S_ISREG(path_stat.st_mode))
            return indexedPath;
    }
    return "";
}

std::string Response::buildFullPath(ConfigNode* locationNode, const std::string& requestPath) const
{
    if (!locationNode)
        return "";

    std::string root = findEffectiveRoot(locationNode);
    if (root.empty())
        return "";

    std::string fullPath = root;
    if (!fullPath.empty() && !requestPath.empty() && fullPath[fullPath.length() - 1] == '/' && requestPath[0] == '/')
        fullPath += requestPath.substr(1);
    else if (!fullPath.empty() && !requestPath.empty() && fullPath[fullPath.length() - 1] != '/' && requestPath[0] != '/')
        fullPath += "/" + requestPath;
    else
        fullPath += requestPath;

    return fullPath;
}

std::string Response::resolveFilePath(ConfigNode* locationNode, const std::string& requestPath) const
{
    std::string fullPath = buildFullPath(locationNode, requestPath);
    if (fullPath.empty())
        return "";

    struct stat path_stat;
    if (stat(fullPath.c_str(), &path_stat) == 0)
    {
        if (S_ISDIR(path_stat.st_mode))
        {
            std::vector<std::string> indexFiles = findEffectiveIndexFiles(locationNode);
            return tryIndexFiles(fullPath, indexFiles);
        }
        else if (S_ISREG(path_stat.st_mode))
            return fullPath;
    }
    return "";
}

bool Response::loadPageContent(const std::string& filePath, std::string& content) const
{
    std::ifstream file(filePath.c_str(), std::ios::binary);
    if (file.is_open())
    {
        std::stringstream buffer;
        buffer << file.rdbuf();
        content = buffer.str();
        file.close();
        return true;
    }
    return false;
}

/**
 * @brief Checks if a HTTP method is allowed for a given location
 * @param method HTTP method to check (GET, POST, DELETE)
 * @param locationNode Location node to search in hierarchy
 * @return true if method is allowed, false otherwise
 */
bool Response::isMethodAllowed(const std::string& method, ConfigNode* locationNode) const
{
    ConfigNode* searchNode = locationNode;
    HttpMethod methodEnum;

    if (method == "GET")
        methodEnum = METHOD_GET;
    else if (method == "POST")
        methodEnum = METHOD_POST;
    else if (method == "DELETE")
        methodEnum = METHOD_DELETE;
    else
        return false;
    while (searchNode)
    {
        std::map<std::string, std::vector<std::string> >::iterator it = searchNode->directives.find("methods");
        if (it != searchNode->directives.end())
            return searchNode->allowedMethods[methodEnum];
        searchNode = searchNode->parent;
    }
    return true;
}