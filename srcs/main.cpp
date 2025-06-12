/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cviegas <cviegas@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/05 19:03:07 by ple-guya          #+#    #+#             */
/*   Updated: 2025/06/12 16:50:33 by cviegas          ###   ########.fr       */
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
            return (v(B RED "Usage: " R RED << av[0] << B " [config_file]" R RED), 1);
        if (ac == 2)
            config_file = av[1];
        Webserv webserver(config_file);
        std::cout << B GREEN "Webserver initialized successfully with config: " 
                  << config_file << R << std::endl;
		if (!CONFIG_TESTER)
        	webserver.run();
    }
    catch (const Webserv::ParsingError &e)
    {
        return (v(e.what()), 1);
    }
    catch (const std::runtime_error &e)
    {
        return (v(e.what()), 1);
    }
    catch (...)
    {
        return (v(R RED "An unknown error occurred." R), 1);
    }
}

