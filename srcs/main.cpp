/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cezou <cezou@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/05 19:03:07 by ple-guya          #+#    #+#             */
/*   Updated: 2025/04/07 00:24:06 by cezou            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include "Webserv.hpp"

#define RED "\033[31m"
#define GREEN "\033[32m"
#define RESET "\033[0m"
#define B "\033[1m"

/**
 * @brief Point d'entrée principal du programme
 * @param ac Nombre d'arguments
 * @param av Tableau d'arguments
 * @return Code de sortie
 */
int main(int ac, char **av)
{
	try
	{
		std::string config_file;
		
		if (ac > 2)
		{
			std::cerr << B RED "Usage: " RESET RED << av[0] << B " [config_file]" RESET << std::endl;
			return (1);
		}
		else if (ac == 2)
			config_file = av[1];
		else
			config_file = "config/webserv.conf";
			
		Webserv webserver(config_file);
		std::cout << B GREEN "Webserver initialized successfully with config: " 
		          << config_file << RESET << std::endl;
		webserver.run();
	}
	catch (const std::exception &e)
	{
		std::cerr << B RED "Error: " RESET RED << e.what() << RESET << std::endl;
		return (1);
	}
	return (0);
}
