/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cviegas <cviegas@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/05 19:03:07 by ple-guya          #+#    #+#             */
/*   Updated: 2025/03/20 23:24:52 by cviegas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include "../includes/Webserv.hpp"

#define RED "\033[31m"
#define GREEN "\033[32m"
#define RESET "\033[0m"
#define B "\033[1m"

int main(int ac, char **av)
{
	try
	{
		if (ac != 2)
		{
			std::cerr << B RED "Usage: " RESET RED << av[0] << B " [config_file]" RESET << std::endl;
			return (1);
		}

		Webserv webserver;
		if (ac == 2)
			webserver = Webserv(av[1]);

		std::cout << B GREEN "Webserver initialized successfully." RESET << std::endl;

		// Future server running code will go here
	}
	catch (const std::exception &e)
	{
		std::cerr << B RED "Error: " RESET RED << e.what() << RESET << std::endl;
		return (1);
	}

	return (0);
}