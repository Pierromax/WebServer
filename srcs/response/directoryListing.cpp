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
    html << "<table>\n";
    html << "<tr><th>Name</th><th>Last modified</th><th>Size</th></tr>\n";
    
    if (requestPath != "/")
    {
        std::string parentPath = requestPath;
        if (parentPath.length() > 1 && parentPath[parentPath.length() - 1] == '/')
            parentPath = parentPath.substr(0, parentPath.length() - 1);
        size_t lastSlash = parentPath.rfind('/');
        if (lastSlash != std::string::npos)
            parentPath = parentPath.substr(0, lastSlash + 1);
        else
            parentPath = "/";
        html << "<tr><td><a href=\"" << parentPath << "\">../</a></td><td>-</td><td>-</td></tr>\n";
    }
    
    DIR* dir = opendir(directoryPath.c_str());
    if (dir)
    {
        struct dirent* entry;
        while ((entry = readdir(dir)) != NULL)
        {
            if (entry->d_name[0] == '.')
                continue;
            
            std::string fullPath = directoryPath;
            if (!fullPath.empty() && fullPath[fullPath.length() - 1] != '/')
                fullPath += "/";
            fullPath += entry->d_name;
            
            struct stat fileStat;
            std::string linkPath = requestPath;
            if (!linkPath.empty() && linkPath[linkPath.length() - 1] != '/')
                linkPath += "/";
            linkPath += entry->d_name;
            
            if (stat(fullPath.c_str(), &fileStat) == 0)
            {
                char timeStr[256];
                struct tm* timeinfo = localtime(&fileStat.st_mtime);
                strftime(timeStr, sizeof(timeStr), "%d-%b-%Y %H:%M", timeinfo);
                
                if (S_ISDIR(fileStat.st_mode))
                {
                    html << "<tr><td><a href=\"" << linkPath << "/\">" << entry->d_name << "/</a></td>";
                    html << "<td class=\"date\">" << timeStr << "</td>";
                    html << "<td class=\"size\">-</td></tr>\n";
                }
                else
                {
                    html << "<tr><td><a href=\"" << linkPath << "\">" << entry->d_name << "</a></td>";
                    html << "<td class=\"date\">" << timeStr << "</td>";
                    html << "<td class=\"size\">" << fileStat.st_size << "</td></tr>\n";
                }
            }
        }
        closedir(dir);
    }
    
    html << "</table>\n";
    html << "<hr>\n";
    html << "<address>Webserv/1.0 Server</address>\n";
    html << "</body>\n</html>\n";
    
    return html.str();
}

