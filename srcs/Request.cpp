/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ple-guya <ple-guya@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/05 19:04:33 by ple-guya          #+#    #+#             */
/*   Updated: 2025/05/07 15:03:17 by ple-guya         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"
#include <cerrno> // For errno
#include <cstring> // For strerror

Request::Request(int client_fd) : statuscode(GOOD_REQUEST), body(""), _bytesRead(-1), _isEmptyInput(false)
{
	this->fd = client_fd;
	
    char buffer[1025] = {0};
    ssize_t bytes_received = recv(this->fd, buffer, sizeof(buffer) - 1, 0);
    
    if (bytes_received > 0)
    {
        buffer[_bytesRead] = '\0';
        bool isOnlyNewline = (_bytesRead == 1 && buffer[0] == '\n');
        bool isOnlyCRLF = (_bytesRead == 2 && buffer[0] == '\r' && buffer[1] == '\n');

        if (isOnlyNewline || isOnlyCRLF)
            _isEmptyInput = true;
        else 
        {
            _isEmptyInput = false; 
            Request::parseRequest(buffer);
        }
    }
    else if (_bytesRead == 0)
        statuscode = ""; // Indicate disconnect
    else // _bytesRead < 0
    {
        if (errno != EAGAIN && errno != EWOULDBLOCK)
        {
            statuscode = BAD_REQUEST;
            std::cerr << "recv error: " << strerror(errno) << std::endl;
        }
    }
}

Request::Request(const Request &cpy) : headers(cpy.headers)
{
	
}

Request &Request::operator=(const Request &rhs)
{
	if (&rhs != this)
		return (*this);
	return (*this);
}

Request::~Request()
{}

std::string Request::getMethod() const { return method; }

std::string Request::getPath() const { return path; }

std::string Request::getStatusCode() const { return statuscode; }

ssize_t Request::getBytesRead() const { return _bytesRead; }

bool Request::isEmptyInput() const { return _isEmptyInput; }

std::string Request::getHeader(const std::string &name) const 
{
    std::map<std::string, std::string>::const_iterator it = headers.find(name);

    if (it != headers.end())
        return it->second;
    return "";
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
    if (headers.count("content-length") && method != "GET")
        Request::parseBody(buffer.substr(bodystart));
}

/**
 * @brief Analyse la première ligne de la requête HTTP
 * @param line Première ligne à analyser
 */
void	Request::parseFirstline(const std::string &line)
{
	std::istringstream firstline(line);
	method = "";
	path = "";
	version = "";

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
	
	if (headers.count("Cookie") && key == "Cookie")
		headers[key] = headers[key] + "; " + value;
	else
		headers[key] = value;

	key = trimString(key, " \t");
	value = trimString(value, " \t");
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
	if (body_length > 0 && raw_body.length() >= static_cast<size_t>(body_length))
		this->body = raw_body.substr(0, body_length);
	else
		this->body = raw_body;
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

std::string		Request::getroot() const
{
	return (node->value);
}