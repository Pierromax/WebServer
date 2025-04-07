/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cezou <cezou@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/19 12:27:22 by ple-guya          #+#    #+#             */
/*   Updated: 2025/04/07 17:58:32 by cezou            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <iostream>

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

/**
 * @brief Fonction de debug pour tracer l'exécution et identifier où un segfault se produit
 * @param n Numéro de ligne ou identificateur personnalisé
 * @param msg Message supplémentaire optionnel
 */
inline void s(int n, const std::string& msg = "")
{
    std::cout << "Squid #" << n;
    if (!msg.empty())
        std::cout << " - " << msg;
    std::cout << std::endl;
}

#endif
