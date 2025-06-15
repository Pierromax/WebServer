/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ParsingError.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cviegas <cviegas@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/12 15:00:00 by cviegas           #+#    #+#             */
/*   Updated: 2025/06/12 15:17:54 by cviegas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ParsingError.hpp"

std::string ParsingError::formatError(const std::string& message, const std::string& filename, std::size_t line) const
{
    std::stringstream ss;
    ss << B RED "Parsing Error: " R RED << message << " in " << filename << ":" << line << R;
    return ss.str();
}

std::string ParsingError::formatDirectiveError(const std::string& directive, const std::string& message, 
                                             const std::string& filename, std::size_t line) const
{
    std::stringstream ss;
    ss << B RED "Parsing Error: " R RED << message << " in directive \"" << directive 
       << "\" in " << filename << ":" << line << R;
    return ss.str();
}

ParsingError::ParsingError(const std::string& message) 
    : std::runtime_error(std::string(B RED "Parsing Error: " R RED) + message + R)
{
}

ParsingError::ParsingError(const std::string& message, const std::string& filename, std::size_t line)
    : std::runtime_error(formatError(message, filename, line))
{
}

ParsingError::ParsingError(const std::string& directive, const std::string& message, 
                          const std::string& filename, std::size_t line)
    : std::runtime_error(formatDirectiveError(directive, message, filename, line))
{
}
