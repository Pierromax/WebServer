/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ParsingError.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cviegas <cviegas@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/12 15:00:00 by cviegas           #+#    #+#             */
/*   Updated: 2025/06/12 15:17:54 by cviegas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PARSING_ERROR_HPP
#define PARSING_ERROR_HPP

#include <stdexcept>
#include <string>
#include <sstream>
#include "Utils.hpp"

class ParsingError : public std::runtime_error
{
private:
    std::string formatError(const std::string& message, const std::string& filename, std::size_t line) const;
    std::string formatDirectiveError(const std::string& directive, const std::string& message, 
                                   const std::string& filename, std::size_t line) const;

public:
    ParsingError(const std::string& message);
    ParsingError(const std::string& message, const std::string& filename, std::size_t line);
    ParsingError(const std::string& directive, const std::string& message, 
                 const std::string& filename, std::size_t line);
};

#endif
