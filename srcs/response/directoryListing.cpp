#include "Response.hpp"
#include "Request.hpp"
#include "Response.hpp"

bool Response::shouldGenerateAutoindex(ConfigNode* locationNode) const
{
    ConfigNode* searchNode = locationNode;
    while (searchNode)
    {
        if (searchNode->autoindex)
            return true;
        searchNode = searchNode->parent;
    }
    return false;
}

std::string Response::generateDirectoryListing(const std::string& directoryPath, const std::string& requestPath) const
{
    std::stringstream html;
    
    generateHtmlHeader(html, requestPath);
    generateTableHeader(html);
    generateParentDirectoryLink(html, requestPath);
    generateDirectoryEntries(html, directoryPath, requestPath);
    generateHtmlFooter(html);
    
    return html.str();
}

void Response::generateHtmlHeader(std::stringstream& html, const std::string& requestPath) const
{
    html << "<!DOCTYPE html>\n";
    html << "<html>\n<head>\n";
    html << "<title>Index of " << requestPath << "</title>\n";
    html << "<style>\n";
    html << "body { font-family: Arial, sans-serif; margin: 40px; }\n";
    html << "h1 { color: #333; }\n";
    html << "table { border-collapse: collapse; width: 100%; }\n";
    html << "th, td { text-align: left; padding: 8px 12px; border-bottom: 1px solid #ddd; }\n";
    html << "th { background-color: #f5f5f5; }\n";
    html << "a { text-decoration: none; color: #0066cc; }\n";
    html << "a:hover { text-decoration: underline; }\n";
    html << ".size { text-align: right; }\n";
    html << ".date { font-family: monospace; }\n";
    html << "</style>\n";
    html << "</head>\n<body>\n";
    html << "<h1>Index of " << requestPath << "</h1>\n";
}

void Response::generateTableHeader(std::stringstream& html) const
{
    html << "<table>\n";
    html << "<tr><th>Name</th><th>Last modified</th><th>Size</th></tr>\n";
}

void Response::generateParentDirectoryLink(std::stringstream& html, const std::string& requestPath) const
{
    if (requestPath != "/")
    {
        std::string parentPath = calculateParentPath(requestPath);
        html << "<tr><td><a href=\"" << parentPath << "\">../</a></td><td>-</td><td>-</td></tr>\n";
    }
}

std::string Response::calculateParentPath(const std::string& requestPath) const
{
    std::string parentPath = requestPath;
    if (parentPath.length() > 1 && parentPath[parentPath.length() - 1] == '/')
        parentPath = parentPath.substr(0, parentPath.length() - 1);
    size_t lastSlash = parentPath.rfind('/');
    if (lastSlash != std::string::npos)
        parentPath = parentPath.substr(0, lastSlash + 1);
    else
        parentPath = "/";
    return parentPath;
}

void Response::generateDirectoryEntries(std::stringstream& html, const std::string& directoryPath, const std::string& requestPath) const
{
    DIR* dir = opendir(directoryPath.c_str());
    if (!dir)
        return;
    
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_name[0] == '.')
            continue;
        
        generateSingleEntry(html, directoryPath, requestPath, entry->d_name);
    }
    closedir(dir);
}

void Response::generateSingleEntry(std::stringstream& html, const std::string& directoryPath, 
                                  const std::string& requestPath, const std::string& entryName) const
{
    std::string fullPath = buildEntryFullPath(directoryPath, entryName);
    std::string linkPath = buildEntryLinkPath(requestPath, entryName);
    
    struct stat fileStat;
    if (stat(fullPath.c_str(), &fileStat) != 0)
        return;
    
    std::string timeStr = formatFileTime(fileStat.st_mtime);
    
    if (S_ISDIR(fileStat.st_mode))
        generateEntryRow(html, linkPath, entryName, timeStr, true, 0);
    else
        generateEntryRow(html, linkPath, entryName, timeStr, false, fileStat.st_size);
}

std::string Response::buildEntryFullPath(const std::string& directoryPath, const std::string& entryName) const
{
    std::string fullPath = directoryPath;
    if (!fullPath.empty() && fullPath[fullPath.length() - 1] != '/')
        fullPath += "/";
    fullPath += entryName;
    return fullPath;
}

std::string Response::buildEntryLinkPath(const std::string& requestPath, const std::string& entryName) const
{
    std::string linkPath = requestPath;
    if (!linkPath.empty() && linkPath[linkPath.length() - 1] != '/')
        linkPath += "/";
    linkPath += entryName;
    return linkPath;
}

std::string Response::formatFileTime(time_t mtime) const
{
    char timeStr[256];
    struct tm* timeinfo = localtime(&mtime);
    strftime(timeStr, sizeof(timeStr), "%d-%b-%Y %H:%M", timeinfo);
    return std::string(timeStr);
}

void Response::generateEntryRow(std::stringstream& html, const std::string& linkPath, 
                               const std::string& entryName, const std::string& timeStr, 
                               bool isDirectory, off_t fileSize) const
{
    html << "<tr><td><a href=\"" << linkPath;
    if (isDirectory)
        html << "/\">" << entryName << "/</a></td>";
    else
        html << "\">" << entryName << "</a></td>";
    
    html << "<td class=\"date\">" << timeStr << "</td>";
    html << "<td class=\"size\">";
    
    if (isDirectory)
        html << "-";
    else
        html << fileSize;
    
    html << "</td></tr>\n";
}

void Response::generateHtmlFooter(std::stringstream& html) const
{
    html << "</table>\n";
    html << "<hr>\n";
    html << "<address>Webserv/1.0 Server</address>\n";
    html << "</body>\n</html>\n";
}

