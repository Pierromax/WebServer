/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ple-guya <ple-guya@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/05 19:04:33 by ple-guya          #+#    #+#             */
/*   Updated: 2025/05/30 12:49:18 by ple-guya         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"
#include "Utils.hpp"
#include "Webserv.hpp"
#include <cerrno> // For errno
#include <cstring> // For strerror

Request::Request(int client_fd) : fd(client_fd),                     
      statuscode("200 OK"),
      method(""),
      path(""),
      version(""),
      headers(),
      body(""),
      raw_request(""),
      _bytesRead(0),
      _isEmptyInput(false)
{}

Request::Request(const Request &cpy) : fd(cpy.fd),                     
      statuscode(cpy.statuscode),
      method(cpy.method),
      path(cpy.path),
      version(cpy.version),
      headers(cpy.headers),
      body(cpy.body),
      raw_request(cpy.raw_request),
      _bytesRead(cpy._bytesRead),
      _isEmptyInput(cpy._isEmptyInput)
{}

Request &Request::operator=(const Request &rhs)
{
	if (&rhs != this)
    {
        fd = rhs.fd;                     
        statuscode = rhs.statuscode;
        method = rhs.method;
        path = rhs.path;
        version = rhs.version;
        headers = rhs.headers;
        body = rhs.body;
        _bytesRead = rhs._bytesRead;
        _isEmptyInput = rhs._isEmptyInput;
    }
	return (*this);
}

Request::~Request()
{}

int         Request::getfd() const { return fd;}

std::string Request::getMethod() const { return method; }

std::string Request::getPath() const { return path; }

std::string Request::getStatusCode() const { return statuscode; }

ssize_t Request::getBytesRead() const { return _bytesRead; }

bool Request::isEmptyInput() const { return _isEmptyInput; }

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

void    Request::ReadFromSocket()
{
    char buffer[1025] = {0};
    ssize_t bytes_received = recv(this->fd, buffer, sizeof(buffer) - 1, 0);

    if (bytes_received > 0)
    {
        buffer[bytes_received] = '\0';
        this->raw_request.append(buffer); 

        // Détection de fin de requête HTTP (double CRLF pour headers)
        size_t headerEnd = this->raw_request.find("\r\n\r\n");
        if (headerEnd != std::string::npos)
        {
            // Parse juste les headers pour obtenir content-length
            std::string headerPart = this->raw_request.substr(0, headerEnd + 4);
            parseRequest(headerPart);
            
            // Pour POST, vérifier si on a tout le body
            if (method == "POST" && headers.count("content-length"))
            {
                std::istringstream iss(headers["content-length"]);
                int expected_length = 0;
                iss >> expected_length;
                
                size_t current_body_length = this->raw_request.length() - (headerEnd + 4);
                if (current_body_length >= static_cast<size_t>(expected_length))
                {
                    // On a tout le body, re-parser la requête complète
                    parseRequest(this->raw_request);
                    _isEmptyInput = true;
                }
                else
                {
                    _isEmptyInput = false; // Attendre plus de données
                }
            }
            else
            {
                // GET ou requête sans body
                _isEmptyInput = true;
            }
        }
        else
        {
            _isEmptyInput = false;
        }
        _bytesRead += bytes_received;
    }
    else if (bytes_received == 0)
    {
        statuscode = ""; // Indicate disconnect
    }
    else
    {
        if (errno != EAGAIN && errno != EWOULDBLOCK)
        {
            statuscode = BAD_REQUEST;
            std::cerr << "recv error: " << strerror(errno) << std::endl;
        }
    }
}

void	Request::parseRequest(const std::string &buffer)
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

    std::streampos bodystart = raw_request.tellg();
    if (headers.count("content-length") && method != "GET" && bodystart != std::streampos(-1))
    {
        size_t bodyStartIndex = static_cast<size_t>(bodystart);
        if (bodyStartIndex < buffer.length())
            Request::parseBody(buffer.substr(bodyStartIndex));
    }
}

/**
 * @brief Analyse la première ligne de la requête HTTP
 * @param line Première ligne à analyser
 */
void	Request::parseFirstline(const std::string &line)
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

//ignore les header faux, on les utiliseras pas de toute facon (vrai fonctionnement http)
void	Request::parseHeader(const std::string &line)
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
	
	// Convertir la clé en minuscules pour une comparaison cohérente
	for (size_t i = 0; i < key.length(); ++i)
		key[i] = std::tolower(key[i]);
	
	if (headers.count("cookie") && key == "cookie")
		headers[key].append("; " + value);
	else
		headers[key] = value;
}

//a faire en separant si la requete est POST ou DELETE
void Request::parseBody(const std::string &raw_body)
{
	if (raw_body.empty() || !headers.count("content-length"))
	{
		this->body = "";
		return;
	}

	std::istringstream iss(headers["content-length"]);
	int body_length = 0;
	
	if (!(iss >> body_length) || body_length < 0)
	{
		this->body = "";
		return;
	}
	
	// Pour les requêtes POST, on doit lire exactement body_length caractères
	if (body_length > 0)
	{
		if (raw_body.length() >= static_cast<size_t>(body_length))
			this->body = raw_body.substr(0, body_length);
		else
			this->body = raw_body; // Prendre ce qu'on a pour l'instant
	}
	else
		this->body = "";
}

std::string trimString(std::string &str, const std::string &charset)
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
    if (this->raw_request.find("\r\n\r\n") == std::string::npos) //check headers
        return (false);
    if (headers.count("content-length")) //check le body
    {
        std::istringstream iss(getHeader("content-length"));
        size_t lenght;
        iss >> lenght;
        if (this->body.length() < lenght)
            return false;
    }
    return true;
}