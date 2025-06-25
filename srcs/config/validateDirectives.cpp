#include "Webserv.hpp"

/**
 * @brief Validates CGI directive format
 * @param values Values for the CGI directive
 * @param filename Configuration filename (for error reporting)
 * @param line Line number (for error reporting)
 */
void Webserv::validateCgiDirective(const std::vector<std::string> &values, const std::string &filename, std::size_t line)
{
    if (values.size() != 2)
        throw ParsingError("directive \"cgi\" takes exactly 2 arguments", filename, line);
    if (access(values[1].c_str(), F_OK | X_OK) != 0)
        throw ParsingError("cgi executable \"" + values[1] + "\" is not accessible in directive \"cgi\"", filename, line);
    struct stat fileStat;
    if (stat(values[1].c_str(), &fileStat) == 0) {
        if (S_ISDIR(fileStat.st_mode))
            throw ParsingError("cgi path \"" + values[1] + "\" is a directory, not an executable in directive \"cgi\"", filename, line);
    }
}

/**
 * @brief Validates methods directive format
 * @param values Values for the methods directive
 * @param filename Configuration filename (for error reporting)
 * @param line Line number (for error reporting)
 */
void Webserv::validateMethodsDirective(const std::vector<std::string> &values, const std::string &filename, std::size_t line)
{
    std::set<std::string> validMethods;
    std::set<std::string> usedMethods;

    if (values.size() > 3)
        throw ParsingError("directive \"methods\" takes at most 3 arguments", filename, line);
    validMethods.insert("GET");
    validMethods.insert("POST");
    validMethods.insert("DELETE");
    for (size_t i = 0; i < values.size(); ++i) {
        const std::string& method = values[i];
        if (validMethods.find(method) == validMethods.end())
            throw ParsingError("invalid method \"" + method + "\" in directive \"methods\"", filename, line);
        if (usedMethods.find(method) != usedMethods.end())
            throw ParsingError("duplicate method \"" + method + "\" in directive \"methods\"", filename, line);
        usedMethods.insert(method);
    }
}

/**
 * @brief Validates listen directive format
 * @param values Values for the listen directive
 * @param filename Configuration filename (for error reporting)
 * @param line Line number (for error reporting)
 */
void Webserv::validateListenDirective(const std::vector<std::string> &values, const std::string &filename, std::size_t line)
{
    for (size_t i = 0; i < values.size(); ++i) {
        const std::string& listenStr = values[i];
        std::string host = "";
        int port = 0;
        size_t colonPos = listenStr.find(':');

        if (colonPos != std::string::npos)
        {
            host = listenStr.substr(0, colonPos);
            std::string portStr = listenStr.substr(colonPos + 1);
            std::stringstream ss(portStr);
            std::string remaining;

            if (!(ss >> port) || ss >> remaining)
                throw ParsingError("invalid port \"" + listenStr + "\" in directive \"listen\"", filename, line);
        }
        else
        {
            std::stringstream ss(listenStr);
            std::string remaining;

            if (!(ss >> port) || ss >> remaining)
                throw ParsingError("invalid port \"" + listenStr + "\" in directive \"listen\"", filename, line);
        }
        if (port < 1024 || port > 65535)
            throw ParsingError("port " + listenStr + " out of range (1024-65535) in directive \"listen\"", filename, line);
    }
}

/**
 * @brief Validates client_max_body_size directive format
 * @param values Values for the client_max_body_size directive
 * @param filename Configuration filename (for error reporting)
 * @param line Line number (for error reporting)
 * @param node ConfigNode to store the parsed value
 */
void Webserv::validateClientMaxBodySizeDirective(const std::vector<std::string> &values, const std::string &filename, std::size_t line, ConfigNode *node)
{
    if (values.size() != 1)
        throw ParsingError("directive \"client_max_body_size\" takes exactly 1 argument", filename, line);

    const std::string& sizeStr = values[0];
    std::stringstream ss(sizeStr);
    long long baseSize;
    std::string remaining;

    if (!(ss >> baseSize))
        throw ParsingError("invalid size \"" + sizeStr + "\" in directive \"client_max_body_size\"", filename, line);
    if (ss >> remaining) {
        if (remaining.length() != 1)
            throw ParsingError("invalid size format \"" + sizeStr + "\" in directive \"client_max_body_size\"", filename, line);
        char unit = remaining[0];
        if (unit == 'M' || unit == 'm')
            baseSize *= 1000000;
        else if (unit == 'K' || unit == 'k')
            baseSize *= 1000;
        else
            throw ParsingError("invalid size unit \"" + std::string(1, unit) + "\" in directive \"client_max_body_size\"", filename, line);
    } 
    if (baseSize < 1)
        throw ParsingError("size must be at least 1 byte in directive \"client_max_body_size\"", filename, line);
    node->client_max_body_size = baseSize;
}

/**
 * @brief Validates autoindex directive format
 * @param values Values for the autoindex directive
 * @param filename Configuration filename (for error reporting)
 * @param line Line number (for error reporting)
 * @param node ConfigNode to store the parsed value
 */
void Webserv::validateAutoindexDirective(const std::vector<std::string> &values, const std::string &filename, std::size_t line, ConfigNode *node)
{
	if (node->type == "server")
		throw ParsingError("directive \"autoindex\" is not allowed in server context", filename, line);
	if (values.size() != 1)
        throw ParsingError("directive \"autoindex\" takes exactly 1 argument", filename, line);
    const std::string& value = values[0];
    if (value != "on" && value != "off")
        throw ParsingError("invalid value \"" + value + "\" in directive \"autoindex\", must be \"on\" or \"off\"", filename, line);
    
    node->autoindex = (value == "on");
}
/**
 * @brief Initializes valid HTTP status codes
 * @return Set containing all valid HTTP status codes
 */
std::set<std::string> initValidCodes()
{
    std::set<std::string> codes;
    const char* validCodes[] = {VALID_CODES_LIST};
    size_t count = sizeof(validCodes) / sizeof(validCodes[0]);
    for (size_t i = 0; i < count; ++i)
        codes.insert(validCodes[i]);
    return codes;
}
static const std::set<std::string> VALID_HTTP_CODES = initValidCodes();

/**
 * @brief Validates error_page directive format
 * @param values Values for the error_page directive
 * @param filename Configuration filename (for error reporting)
 * @param line Line number (for error reporting)
 */
void Webserv::validateErrorPageDirective(const std::vector<std::string> &values, const std::string &filename, std::size_t line)
{
    if (values.size() != 2)
        throw ParsingError("directive \"error_page\" takes exactly 2 arguments", filename, line);

    const std::string& errorCode = values[0];
    if (VALID_HTTP_CODES.find(errorCode) == VALID_HTTP_CODES.end())
        throw ParsingError("invalid error code \"" + errorCode + "\" in directive \"error_page\"", filename, line);
}

/**
 * @brief Validates return directive format
 * @param values Values for the return directive
 * @param filename Configuration filename (for error reporting)
 * @param line Line number (for error reporting)
 * @param node ConfigNode to store the parsed value
 */
void Webserv::validateRedirectDirective(const std::vector<std::string> &values, const std::string &filename, std::size_t line, ConfigNode *node)
{
    if (values.size() != 2)
        throw ParsingError("directive \"return\" takes exactly 2 arguments", filename, line);
    if (node->parent == NULL || node->parent->type != "server")
        throw ParsingError("directive \"return\" is not allowed in server context", filename, line);

    const std::string& statusCode = values[0];
    if (VALID_HTTP_CODES.find(statusCode) == VALID_HTTP_CODES.end())
        throw ParsingError("invalid status code \"" + statusCode + "\" in directive \"return\"", filename, line);
}

/**
 * @brief Validates server ports for duplicates
 * @param serverNode Server node to validate
 * @param filename Configuration filename (for error reporting)
 * @param usedPorts Map to track duplicate ports
 */
void Webserv::validateServerPorts(ConfigNode *serverNode, const std::string &filename, std::map<int, size_t> &usedPorts)
{
    std::map<std::string, std::vector<std::string> >::iterator it = serverNode->directives.find("listen");
    if (it == serverNode->directives.end() || it->second.empty())
        return;
    
    std::set<int> serverPorts;
    for (size_t i = 0; i < it->second.size(); ++i) {
        std::stringstream ss(it->second[i]);
        int port;
        if (!(ss >> port))
            continue;
        
        if (serverPorts.find(port) != serverPorts.end())
            throw ParsingError("duplicate listen port " + it->second[i] + " in same server", filename, serverNode->line);
        serverPorts.insert(port);
    }
    (void)usedPorts;
}

/**
 * @brief Validates server listen/server_name combinations for duplicates
 * @param serverNode Server node to validate
 * @param filename Configuration filename (for error reporting)
 * @param usedListenServerName Map to track duplicate (listen, server_name) combinations
 */
void Webserv::validateServerListenServerName(ConfigNode *serverNode, const std::string &filename, std::map<std::pair<std::string, std::string>, size_t> &usedListenServerName)
{
    std::map<std::string, std::vector<std::string> >::iterator listenIt = serverNode->directives.find("listen");
    std::map<std::string, std::vector<std::string> >::iterator serverNameIt = serverNode->directives.find("server_name");
    
    // Get listen values (default to "80" if not present)
    std::vector<std::string> listenValues;
    if (listenIt != serverNode->directives.end() && !listenIt->second.empty())
        listenValues = listenIt->second;
    else
        listenValues.push_back("80");
    
    // Get server_name values (default to "" if not present)
    std::vector<std::string> serverNameValues;
    if (serverNameIt != serverNode->directives.end() && !serverNameIt->second.empty())
        serverNameValues = serverNameIt->second;
    else
        serverNameValues.push_back("");
    
    // Check each combination of (listen, server_name)
    for (size_t i = 0; i < listenValues.size(); ++i)
    {
        for (size_t j = 0; j < serverNameValues.size(); ++j)
        {
            std::pair<std::string, std::string> combination = std::make_pair(listenValues[i], serverNameValues[j]);
            std::map<std::pair<std::string, std::string>, size_t>::iterator it = usedListenServerName.find(combination);
            
            if (it != usedListenServerName.end())
            {
                std::string errorMsg = "duplicate server with listen \"" + listenValues[i] + "\"";
                if (!serverNameValues[j].empty())
                    errorMsg += " and server_name \"" + serverNameValues[j] + "\"";
                throw ParsingError(errorMsg, filename, serverNode->line);
            }
            
            usedListenServerName[combination] = serverNode->line;
            d_cout << "Registered server: listen=" << listenValues[i] << ", server_name=" << serverNameValues[j] << std::endl;
        }
    }
}

/**
 * @brief Checks if a port is available for binding
 * @param port Port number to check
 * @return true if port is available, false otherwise
 */
bool isPortAvailable(int port)
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        return false;

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    bool available = (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == 0);
    close(sockfd);
    return available;
}
