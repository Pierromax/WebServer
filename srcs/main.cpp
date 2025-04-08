/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cezou <cezou@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/05 19:03:07 by ple-guya          #+#    #+#             */
/*   Updated: 2025/04/08 17:14:05 by cezou            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include "Webserv.hpp"

int main(int ac, char **av)
{
    try
    {   
        std::string config_file = "config/webserv.conf";
        if (ac > 2)
        {
			std::cerr
				<< B RED "Usage: " R RED << av[0] << B " [config_file] " R RED << std::endl
				<< "       " I D "(no arguments uses by default " B << config_file 
				<< R I D RED ")" R << std::endl;
            return 1;
        }
        if (ac == 2)
            config_file = av[1];
        Webserv webserver(config_file);
        std::cout << B GREEN "Webserver initialized successfully with config: " 
                  << config_file << R << std::endl;
		if (!CONFIG_TESTER)
        	webserver.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << B RED "Error: " R RED << e.what() << R << std::endl;
        return (1);
    }
}
