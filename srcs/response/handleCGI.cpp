#include "Response.hpp"
#include "Request.hpp"
#include "Response.hpp"

/**
 * @brief Handles CGI request execution for both GET and POST
 * @param req The HTTP request
 * @param locationNode Configuration context node
 * @param filePath Path to the CGI script
 */
void Response::handleCgiRequest(const Request &req, ConfigNode* locationNode, const std::string& filePath)
{
    CGIsHandling* cgiHandler = CGIsHandling::findCgiHandler(filePath, locationNode);
    if (!cgiHandler)
        return;

    try
    {
        cgiHandler->setScriptPath(filePath);
        cgiHandler->setEnvironmentVariable("REQUEST_METHOD", req.getMethod());
        cgiHandler->setEnvironmentVariable("SCRIPT_FILENAME", filePath);
        cgiHandler->setEnvironmentVariable("SCRIPT_NAME", req.getPath());
        cgiHandler->setEnvironmentVariable("REQUEST_URI", req.getPath());
        cgiHandler->setEnvironmentVariable("QUERY_STRING", "");
        
        std::string contentType = req.getHeader("Content-Type");
        if (contentType.empty() && req.getMethod() == "POST")
            contentType = "application/x-www-form-urlencoded";
        cgiHandler->setEnvironmentVariable("CONTENT_TYPE", contentType);
        
        std::string requestBody = req.getBody();
        std::stringstream contentLengthStream;
        contentLengthStream << requestBody.length();
        cgiHandler->setEnvironmentVariable("CONTENT_LENGTH", contentLengthStream.str());
        
        if (req.getMethod() == "POST" && !requestBody.empty())
            cgiHandler->setEnvironmentVariable("REQUEST_BODY", requestBody);
        
        cgiHandler->setEnvironmentVariable("SERVER_SOFTWARE", "Webserv/1.0");
        cgiHandler->setEnvironmentVariable("SERVER_NAME", "localhost");
        
        std::stringstream portStream;
        portStream << _server->port;
        cgiHandler->setEnvironmentVariable("SERVER_PORT", portStream.str());
        
        cgiHandler->setEnvironmentVariable("GATEWAY_INTERFACE", "CGI/1.1");
        cgiHandler->setEnvironmentVariable("SERVER_PROTOCOL", "HTTP/1.1");
        cgiHandler->setEnvironmentVariable("REDIRECT_STATUS", "200");
        
        std::string cgiOutput = cgiHandler->execute();
        size_t headerEnd = cgiOutput.find("\r\n\r\n");
        if (headerEnd != std::string::npos)
        {
            std::string cgiBody = cgiOutput.substr(headerEnd + 4);
            setBody(cgiBody);
        }
        else
            setBody(cgiOutput);
        setContentType("text/html");
        delete cgiHandler;
    }
    catch (const std::exception& e)
    {
        delete cgiHandler;
        status_code = "500 Internal Server Error";
        loadErrorPage("500", locationNode);
    }
}
