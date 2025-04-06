/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cezou <cezou@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/05 19:04:37 by ple-guya          #+#    #+#             */
/*   Updated: 2025/04/06 19:08:01 by cezou            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <iostream>
#include <string>
#include <sstream>
#include <fstream>

class Response
{
    private:
        std::string status_code;
        std::string content_type;
        std::string content;

    public:
        Response();
        Response(const std::string &status);
        Response(const Response &cpy);
        Response &operator=(const Response &rhs);
        ~Response();

        void setStatusCode(const std::string &status);
        void setContentType(const std::string &type);
        void setContent(const std::string &body);
        
        std::string build() const;
        void loadErrorPage(const std::string &file_path);
};

#endif