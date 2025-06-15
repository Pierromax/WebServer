
#include "Response.hpp"
#include "Request.hpp"
#include "Response.hpp"

void Response::handlePostRequest(const Request &req)
{
    this->content_type = req.getHeader("Content-Type");
    std::string path = req.getPath();
    ConfigNode* locationNode = findBestLocation(path, _server);
    std::string filePath = resolveFilePath(locationNode, path);
    
    if (!isMethodAllowed("POST", locationNode))
    {
        status_code = METHOD_NOT_ALLOWED;
        loadErrorPage("405", locationNode);
        return;
    }
    if (req.getStatusCode() == PAYLOAD_TOO_LARGE)
    {
        std::cout << "Payload too large for POST request" << std::endl;
        status_code = PAYLOAD_TOO_LARGE;
        loadErrorPage("413", locationNode);
        return;
    }

    if (filePath.empty())
    {
        status_code = FILE_NOT_FOUND;
        loadErrorPage("404", locationNode);
        return;
    }

    CGIsHandling* cgiHandler = CGIsHandling::findCgiHandler(filePath, locationNode);
    if (cgiHandler)
    {
        delete cgiHandler;
        handleCgiRequest(req, locationNode, filePath);
        return;
    }

    if (this->content_type.find("multipart/form-data") != std::string::npos)
        handleUploading(req, locationNode);
}


void Response::handleUploading(const Request &req,   ConfigNode* locationNode)
{
    std::map<std::string, std::string> bodyHeaders;
    std::vector<std::string> bodies;
    std::string location = findEffectiveRoot(locationNode);
    std::cout << "find boundary " << std::endl;
    size_t pos = content_type.find("boundary=");
    
    if (pos == std::string::npos)
    {
        status_code = BAD_REQUEST; 
        return;
    }
    std::string boundary = content_type.substr(pos + 9);
    std::string delimiter = "--" + boundary;
    std::string content = req.getBody();

    if (content.empty())
    {
        status_code = BAD_REQUEST; 
        return;
    }
    bodies = splitPostBody(content, delimiter);
    bool validFile = false;
    for (std::vector<std::string>::iterator it = bodies.begin(); it != bodies.end(); it++)
    {
        pos = it->find("\r\n\r\n");
        if (pos != std::string::npos)
        {
            std::cout << "extract info" << std::endl;
            std::string headerPart = it->substr(0, pos);
            std::cout << headerPart << std::endl;
            std::string bodyPart = it->substr(pos + 4);
            bodyHeaders = extractPostHeaders(headerPart);
            std::cout << "try to save" << std::endl;
            if (extractFileToSave(bodyHeaders, bodyPart, location))
                validFile = true;
            std::cout << "after saving file" << std::endl;
        }
    }
    std::cout << "is 1 valid file ?"<< (validFile ? "true" : "false") << std::endl;
    if (validFile)
    {
        status_code = "201 Created";
        std::string successPagePath = location + "/upload/201.html";
        std::string pageContent;
        std::cout << "Tentative de chargement: " << successPagePath << std::endl;
        if (loadPageContent(successPagePath, pageContent))
        {
            std::cout << "Page 201.html chargée avec succès" << std::endl;
            setBody(pageContent);
            setContentType(getMimeType(successPagePath));
        }
    }
}

/* ******************************** */
/* exclusive utils for post request */
/********************************** */

std::vector<std::string> Response::splitPostBody(std::string body, std::string delim)
{
    std::vector<std::string>    split;
    std::string                 content;
    size_t                      start = 0;
    size_t                      newStart;

    while (true)
    {
        newStart = body.find(delim, start);
        if (newStart == std::string::npos)
        {
            split.push_back(body.substr(start));
            break;
        }
        content = body.substr(start, newStart - start);
        if (!content.empty())
            split.push_back(content);
        start = newStart + delim.length();
    }
    return split;
}

std::map<std::string, std::string>  Response::extractPostHeaders(std::string &content)
{
    std::map<std::string, std::string>  parseHeader;
    std::stringstream                   ss(content);
    std::string                  line;

       std::cout << "=== PARSING MANUEL ===" << std::endl;
    std::cout << "Contenu: '" << content << "'" << std::endl;

    
    while(std::getline(ss, line))
    {
        std::cout << "=== line parsed : " << line << "===" << std::endl;
        if (line.empty() || line == "\r")
            continue;
        size_t pos = line.find(":");
        if(pos == std::string::npos)
            continue;
        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);
        key = trimString(key, " \t\n\r");
        std::cout << "key = " << key << std::endl;
        value = trimString(value, " \t\n\r");
        std::cout << "value = " << value << std::endl ;
        parseHeader[key] = value;
    }
    return parseHeader;
}

static std::string extractFilename(std::string content)
{
    size_t filepos = content.find("filename=\"");
    if (filepos == std::string::npos)
        return ("");
    filepos += 10;
    size_t endpos = content.find("\"", filepos);
    if (endpos == std::string::npos)
        return ("");

    std::string filename = content.substr(filepos, endpos - filepos);

    if (filename.find("..") != std::string::npos || filename.find("/") != std::string::npos)
        return "";
    
    return filename;
}

bool    Response::saveFile(std::string &filename, std::string &body, std::string location)
{
    std::string path = location + "/" + filename;
    std::ofstream file(path.c_str(), std::ios::binary);

    std::cout << "path = " << path;

    if (!file.is_open())
    {
        status_code = INTERNAL_ERROR;
        return false;
    }
    
    file.write(body.c_str(), body.size());
    file.close();
    
    if (file.fail())
    {
        status_code = INTERNAL_ERROR; 
        return false;
    }
    return true;
}

bool    Response::extractFileToSave(std::map<std::string, std::string> &heads, std::string &content, std::string location)
{
    std::string filename;

    
    for(std::map<std::string, std::string>::iterator it = heads.begin(); it != heads.end(); it++)
    {
        std::cout << "key : " << it->first << ", value = " << it->second << std::endl;
    }
    
    if (!heads.count("Content-Disposition"))
    {
        std::cout << "no content-dispostion" << std::endl;
        return false;
    }

    size_t pos = heads.at("Content-Disposition").find("filename=");
    if (pos == std::string::npos)
    {
        std::cout << "filename noon found" << std::endl;     
        return false;
    }
        
    filename = extractFilename(heads.at("Content-Disposition"));
    if (filename == "")
        return false;

    if (!saveFile(filename, content, location))
        return false;
    
    return true;
}

