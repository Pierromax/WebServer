/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ple-guya <ple-guya@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/05 19:03:07 by ple-guya          #+#    #+#             */
/*   Updated: 2025/03/21 01:53:28 by ple-guya         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Webserv.hpp"

int main()
{
    try {
        Webserv serveurs;
        serveurs.run();
    }
    catch(std::exception &e){
        std::cerr << e.what() << std::endl;
    }
}