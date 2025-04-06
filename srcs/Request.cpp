/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cezou <cezou@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/05 19:04:33 by ple-guya          #+#    #+#             */
/*   Updated: 2025/04/06 23:54:59 by cezou            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"

Request::Request(int client_fd) : fd(client_fd), statuscode(GOOD_REQUEST), body("")
{
	char buffer[1024];
	recv(client_fd, buffer, sizeof(buffer), 0);
	Request::parseRequest(buffer);
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

void	Request::parseFirstline(const std::string &line)
{
	std::istringstream firstline(line);

	if (!(firstline >> method >> path >> version))
		statuscode = BAD_REQUEST;
	if (method != "GET" && method != "POST" && method != "DELETE")
		statuscode = METHOD_NOT_ALLOWED;
	if (path[0] != '/')
		statuscode = BAD_REQUEST;
	if (version != "HTTP/1.1")
		statuscode = BAD_REQUEST;
}

//ignore les header faux, on les utiliseras pas de toute facon (vrai fonctionnement http)
//passer de map a multimap so on fait bonus (pour les COOKIES)
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

	headers[key] = value;
}

//a faire en separant si la requete est POST ou DELETE
void	Request::parseBody(const std::string &raw_body)
{
	std::istringstream iss(headers["content-length"]);
	int body_lenght;
	iss >> body_lenght;
	
	this->body = raw_body.substr(0, body_lenght);
}

std::string	trimString(std::string &str, const std::string &charset)
{
	size_t start;
	size_t end;

	start = str.find_first_not_of(charset);
	if (start == std::string::npos)
		start = 0;
	end = str.find_last_not_of(charset);

	return (str.substr(start, end - start + 1));
}

