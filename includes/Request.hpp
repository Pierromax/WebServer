/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ple-guya <ple-guya@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/05 19:04:40 by ple-guya          #+#    #+#             */
/*   Updated: 2025/03/20 20:11:51 by ple-guya         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <iostream>
#include <string>
#include <map>
#include <sys/socket.h>

class Request
{
    private:
        int         fd;
        std::string method;
        std::string path;
        std::string version;
        std::map<std::string, std::string> headers;
        void        parseRequest();

    public:
        Request(int clientFD);
        Request(const Request &cpy);
        Request &operator=(const Request &rhs);
        ~Request();
};

#endif