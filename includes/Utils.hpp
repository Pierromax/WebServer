/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cezou <cezou@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/06 20:26:30 by cezou             #+#    #+#             */
/*   Updated: 2025/04/06 20:26:31 by cezou            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILS_HPP
#define UTILS_HPP

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
