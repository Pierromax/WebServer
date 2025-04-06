/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cezou <cezou@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/19 12:27:22 by ple-guya          #+#    #+#             */
/*   Updated: 2025/04/06 23:13:26 by cezou            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>

/**
 * @brief Supprime un pointeur et le met à NULL
 * @tparam T Type du pointeur
 * @param ptr Référence vers le pointeur à supprimer
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

#endif
