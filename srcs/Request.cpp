/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ple-guya <ple-guya@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/05 19:04:33 by ple-guya          #+#    #+#             */
/*   Updated: 2025/06/23 16:10:57 by ple-guya         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"
#include "Utils.hpp"
#include "Webserv.hpp"
#include "Server.hpp"
#include <cerrno>  // For errno
#include <cstring> // For strerror

Request::Request(int client_fd, Server* server) : fd(client_fd),
                                  _server(server),
                                  statuscode("200 OK"),
                                  method(""),
                                  path(""),
                                  version(""),
                                  headers(),
                                  body(""),
                                  raw_request(""),
                                  _bytesRead(0),
                                  _step(0),
                                  _headerSize(0)
{
}

Request::Request(const Request &cpy) : fd(cpy.fd),
                                       _server(cpy._server),
                                       statuscode(cpy.statuscode),
                                       method(cpy.method),
                                       path(cpy.path),
                                       version(cpy.version),
                                       headers(cpy.headers),
                                       body(cpy.body),
                                       raw_request(cpy.raw_request),
                                       _bytesRead(cpy._bytesRead),
                                       _maxBodySize(cpy._maxBodySize),
                                       _step(cpy._step),
                                       _headerSize(cpy._headerSize)
{
}

Request &Request::operator=(const Request &rhs)
{
    if (&rhs != this)
    {
        fd = rhs.fd;
        _server = rhs._server;
        statuscode = rhs.statuscode;
        method = rhs.method;
        path = rhs.path;
        version = rhs.version;
        headers = rhs.headers;
        body = rhs.body;
        _bytesRead = rhs._bytesRead;
        _maxBodySize = rhs._maxBodySize;
        _step = rhs._step;
        _headerSize = rhs._headerSize;
    }
    return (*this);
}

Request::~Request()
{
}

void Request::reset()
{
    statuscode = "200 OK";
    method = "";
    path = "";
    version = "";
    headers.clear();
    body = "";
    raw_request = "";
    _bytesRead = 0;
    _step = 0;
    _headerSize = 0;
    // _maxBodySize reste inchangé car il dépend du serveur
}

int Request::getfd() const { return fd; }

std::string Request::getMethod() const { return method; }

std::string Request::getPath() const { return path; }

std::string Request::getStatusCode() const { return statuscode; }

ssize_t Request::getBytesRead() const { return _bytesRead; }


std::string Request::getBody() const
{
    return body;
}

std::string Request::getHeader(const std::string &name) const
{
    std::map<std::string, std::string>::const_iterator it = headers.find(name);

    if (it != headers.end())
        return it->second;
    return "";
}

/**
 * @brief Trouve la taille des headers HTTP
 * @return Taille des headers + 4 (pour \r\n\r\n), ou 0 si non trouvé
 */
size_t Request::findHeaderSize()
{
    size_t headerEnd = raw_request.find("\r\n\r\n");
    if (headerEnd != std::string::npos)
        return headerEnd + 4;
    return 0;
}

/**
 * @brief Extrait temporairement le path de la première ligne
 * @return Le path extrait ou une string vide
 */
std::string Request::extractTempPath()
{
    size_t firstLineEnd = raw_request.find("\r\n");
    if (firstLineEnd == std::string::npos)
        return "";
    
    std::string firstLine = raw_request.substr(0, firstLineEnd);
    std::istringstream iss(firstLine);
    std::string tempMethod, tempPath, tempVersion;
    
    if (iss >> tempMethod >> tempPath >> tempVersion)
    {
        std::cout << "Temporary path extracted: " << tempPath << std::endl;
        return tempPath;
    }
    return "";
}

/**
 * @brief Met à jour _maxBodySize selon le path trouvé
 * @param tempPath Le path temporaire extrait
 */
void Request::updateMaxBodySize(const std::string& tempPath)
{
    if (!_server)
        return;
    
    ConfigNode* locationNode = findBestLocation(tempPath, _server);
    if (locationNode)
        _maxBodySize = locationNode->client_max_body_size;
    std::cout << "Max body size for this request: " << _maxBodySize << " bytes." << std::endl;
}

/**
 * @brief Extrait la valeur Content-Length des headers
 * @return La valeur Content-Length ou 0 si non trouvé
 */
size_t Request::extractContentLength()
{
    size_t contentLengthPos = raw_request.find("Content-Length:");
    if (contentLengthPos == std::string::npos)
        return 0;
    
    size_t start = raw_request.find(":", contentLengthPos) + 1;
    size_t end = raw_request.find("\r\n", start);
    if (start == std::string::npos || end == std::string::npos)
        return 0;
    
    std::string contentLengthStr = raw_request.substr(start, end - start);
    contentLengthStr = trimString(contentLengthStr, " \t");
    
    std::istringstream iss(contentLengthStr);
    size_t contentLength;
    if (iss >> contentLength)
        return contentLength;
    return 0;
}

/**
 * @brief Vérifie si Content-Length dépasse la limite
 * @param contentLength La valeur de Content-Length
 * @return true si dépasse la limite
 */
bool Request::checkContentLengthLimit(size_t contentLength)
{
    if (contentLength > _maxBodySize)
    {
        std::cout << "Content-Length (" << contentLength << ") exceeds max body size (" << _maxBodySize << ")" << std::endl;
        return true;
    }
    return false;
}

/**
 * @brief Vérifie si la taille du body actuel dépasse la limite
 * @return true si dépasse la limite
 */
bool Request::checkCurrentBodySize()
{
    if (static_cast<int>(_bytesRead) - static_cast<int>(_headerSize) > static_cast<int>(_maxBodySize))
    {
        std::cout << "Body size (" << (_bytesRead - _headerSize) << ") exceeds max body size (" << _maxBodySize << ")" << std::endl;
        return true;
    }
    return false;
}

/**
 * @brief Vérifie si la taille de la requête dépasse la limite maximale maxBodySize
 * @return true si la requête est trop longue, false sinon
 */
bool Request::isRequestTooLong()
{
    std::cout << "Step " << _step << ": Checking request length." << std::endl;

    if (_step == 0)
    {
        _headerSize = findHeaderSize();
        if (_headerSize == 0)
            return false;
        updateMaxBodySize(extractTempPath());
        size_t contentLength = extractContentLength();
        if (contentLength > 0)
            return checkContentLengthLimit(contentLength);
    }
    else
        return checkCurrentBodySize();
    
    return false;
}


void Request::ReadFromSocket()
{
    char buffer[2048] = {0};
    ssize_t bytes_received = recv(this->fd, buffer, sizeof(buffer) - 1, 0);
    
    if (bytes_received > 0)
    {
        _bytesRead += bytes_received;
        this->raw_request.append(buffer, bytes_received);
        if (isRequestTooLong())
        {
            std::cerr << "Request too long: " << _bytesRead << " bytes received, max allowed: " << _maxBodySize << " bytes." << std::endl;
            statuscode = PAYLOAD_TOO_LARGE;
             _step++;
            parseRequest(this->raw_request);
            return;
        }
        _step++;
        if (isComplete())
            parseRequest(this->raw_request);
    }
    else if (bytes_received == 0)
        statuscode = ""; // Indicate disconnect
    else
        statuscode = BAD_REQUEST;
}

void Request::parseRequest(const std::string &buffer)
{
    std::istringstream raw_request(buffer);
    std::string line;

    std::getline(raw_request, line, '\r');
    Request::parseFirstline(line);
    if (raw_request.peek() == '\n')
        raw_request.ignore(1);
    while (std::getline(raw_request, line, '\r'))
    {
        if (raw_request.peek() == '\n')
            raw_request.ignore(1);
        if (line.empty())
            break;
        Request::parseHeader(line);
    }
    size_t headerEndPos = buffer.find("\r\n\r\n");
    if (headerEndPos != std::string::npos && headers.count("Content-Length") && method != "GET")
    {
        size_t bodyStartPos = headerEndPos + 4; // +4 pour passer "\r\n\r\n"
        Request::parseBody(buffer.substr(bodyStartPos));
    }
}

/**
 * @brief Analyse la première ligne de la requête HTTP
 * @param line Première ligne à analyser
 */
void Request::parseFirstline(const std::string &line)
{
    std::istringstream firstline(line);

    if (line.empty() || !(firstline >> method >> path >> version))
    {
        statuscode = BAD_REQUEST;
        return;
    }
    if (method != "GET" && method != "POST" && method != "DELETE")
        statuscode = METHOD_NOT_ALLOWED;
    if (path.empty() || path[0] != '/')
        statuscode = BAD_REQUEST;
    if (version != "HTTP/1.1")
        statuscode = BAD_REQUEST;
}

// ignore les header faux, on les utiliseras pas de toute facon (vrai fonctionnement http)
void Request::parseHeader(const std::string &line)
{
    size_t pos;
    std::string key;
    std::string value;

    pos = line.find(":");
    if (pos == std::string::npos)
        return;
    key = line.substr(0, pos);
    value = line.substr(pos + 1);

    key = trimString(key, " \t");
    value = trimString(value, " \t");

    if (headers.count("Cookie") && key == "Cookie")
        headers[key].append("; " + value);
    else
        headers[key] = value;
}

// a faire en separant si la requete est POST ou DELETE
void Request::parseBody(const std::string &raw_body)
{
    std::cout << "enter parse body" << std::endl;
    if (raw_body.empty() || !headers.count("Content-Length"))
    {
        this->body = "";
        return;
    }

    std::istringstream iss(headers["Content-Length"]);
    int body_length = 0;

    if (!(iss >> body_length) || body_length < 0)
    {
        this->body = "";
        return;
    }
    if (body_length > 0 && raw_body.length() >= static_cast<size_t>(body_length))
        this->body = raw_body.substr(0, body_length);
    else
        this->body = raw_body;
}

std::string trimString(const std::string &str, const std::string &charset)
{
    if (str.empty())
        return "";

    size_t start;
    size_t end;

    start = str.find_first_not_of(charset);
    if (start == std::string::npos)
        return ""; // La chaîne ne contient que des caractères à supprimer

    end = str.find_last_not_of(charset);
    return str.substr(start, end - start + 1);
}

bool Request::isComplete() const
{
    size_t headerEnd = this->raw_request.find("\r\n\r\n");
    if (headerEnd == std::string::npos)
        return (false);
    size_t pos = raw_request.find("Content-Length");
    if (pos == std::string::npos)
        return true;

    size_t start = raw_request.find(":", pos) + 1;
    size_t end = raw_request.find("\r\n", start);

    std::string strValue = raw_request.substr(start, end - start);
    strValue = trimString(strValue, " \t");

    std::istringstream iss(strValue);
    size_t value;
    
    iss >> value;
    if (value == 0)
    {
        std::cout << "value == 0" << std::endl;
        return true;
    }
    if (raw_request.length() - (headerEnd + 4) < value)
        return false;
    return true;
}