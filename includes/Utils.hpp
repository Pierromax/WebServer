/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cviegas <cviegas@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/19 12:27:22 by ple-guya          #+#    #+#             */
/*   Updated: 2025/06/12 15:09:13 by cviegas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <iostream>

// Définitions des codes ANSI pour la formatation du texte
#define RED "\033[31m"
#define GREEN "\033[32m"
#define BLUE "\033[34m"
#define YELLOW "\033[33m"
#define WHITE "\033[37m"
#define R "\033[0m"
#define B "\033[1m"
#define D "\033[2m"
#define I "\033[3m"

#define v(x) (std::cerr << x << std::endl)
/**
 * @brief Deletes a pointer and sets it to NULL
 * @tparam T Pointer type
 * @param ptr Reference to the pointer to delete
 */
template<typename T>
void deleteAndNull(T*& ptr)
{
    if (ptr)
    {
        delete ptr;
        ptr = NULL;
    }
}

/**
 * @brief Debug function to trace execution and identify where a segfault occurs
 * @param msg Optional additional message
 */
inline void s(const std::string& msg = "")
{
    static int count = 0;
    std::cout << "Squid #" << count++;
    if (!msg.empty())
        std::cout << " - " << msg;
    std::cout << std::endl;
}

/**
 * @brief Debug function to trace execution with a specific number
 * @param n Line number or custom identifier
 * @param msg Optional additional message
 */
inline void sn(int n, const std::string& msg = "")
{
    std::cout << "Squid #" << n;
    if (!msg.empty())
        std::cout << " - " << msg;
    std::cout << std::endl;
}

#endif
