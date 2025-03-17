/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cviegas <cviegas@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/05 19:03:07 by ple-guya          #+#    #+#             */
/*   Updated: 2025/03/17 22:50:28 by cviegas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include "../includes/Webserv.hpp"
#include "../includes/Config.hpp"

#include <iostream>
#include <string>
#include <vector>

#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN "\033[36m"

#define RESET "\033[0m"
#define B "\033[1m"

int main(int ac, char **av)
{
	std::string configFile = "default.conf";

	if (ac == 2)
		configFile = av[1];
	else if (ac > 2)
	{
		std::cerr << B RED "Usage: " RESET RED << av[0] << B " <config_file>" RESET << std::endl;
		return (1);
	}

	Config config;
	if (!config.loadFromFile(configFile))
	{
		std::cerr << B RED "Error: " RESET RED "Failed to load configuration file." RESET << std::endl;
		return (1);
	}

	std::cout << B GREEN "Configuration loaded successfully." RESET << std::endl;

	// The rest of the server initialization code will go here

	return (0);
}