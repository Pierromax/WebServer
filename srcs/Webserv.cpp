/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cezou <cezou@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/05 19:04:30 by ple-guya          #+#    #+#             */
/*   Updated: 2025/04/07 00:06:32 by cezou            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Webserv.hpp"
#include "Utils.hpp"
#include <sstream>

/**
 * @brief Constructeur par défaut
 */
Webserv::Webserv() : rootConfig(NULL)
{
    Serveur *newserv = new Serveur();
    pollfd newfd;
    newfd.fd = newserv->getfd();
    newfd.events = POLLIN;
    fds.push_back(newfd);
    serveurs[newfd.fd] = newserv;
    std::cout << "Serveur ajouté à la map avec fd = " << newfd.fd << std::endl;
}

/**
 * @brief Constructeur avec fichier de configuration
 * @param config_filename Chemin vers le fichier de configuration
 */
Webserv::Webserv(std::string config_filename) : rootConfig(NULL)
{
    storeServers(config_filename);
    launchServers();
}

/**
 * @brief Constructeur de copie
 * @param cpy Objet à copier
 */
Webserv::Webserv(const Webserv &cpy) : rootConfig(NULL)
{
    *this = cpy;
}

/**
 * @brief Opérateur d'assignation
 * @param rhs Objet à copier
 * @return Référence vers cet objet
 */
Webserv &Webserv::operator=(const Webserv &rhs)
{
    if (this != &rhs)
    {
        deleteAndNull(rootConfig);
        this->fds = rhs.fds;
        this->serveurs = rhs.serveurs;
    }
    return (*this);
}

/**
 * @brief Destructeur
 */
Webserv::~Webserv()
{
    deleteAndNull(rootConfig);
    for (std::map<int, Serveur*>::iterator it = serveurs.begin(); it != serveurs.end(); ++it)
        delete it->second;
    for (std::map<int, Client*>::iterator it = clients.begin(); it != clients.end(); ++it)
        delete it->second;
    for (std::vector<pollfd>::iterator it = fds.begin(); it != fds.end(); ++it)
        close(it->fd);
}

/**
 * @brief Accepte un nouveau client sur une connexion serveur
 * @param connect Référence au serveur acceptant la connexion
 */
void Webserv::acceptNewClient(const Serveur &connect)
{
    std::cout << "Tentative d'accepter un nouveau client depuis le serveur fd = "
              << connect.getfd() << std::endl;
    sockaddr_in serveuraddr = connect.getAddress();
    socklen_t addrlen = sizeof(serveuraddr);
    int client_fd = accept(connect.getfd(), (sockaddr *)&serveuraddr, &addrlen);
    if (client_fd < 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            std::cout << "Aucune connexion client en attente" << std::endl;
            return;
        }
        std::cerr << "Erreur accept(): " << strerror(errno) << std::endl;
        throw (std::runtime_error("Error: accept client failed"));
    }
    std::cout << "Nouveau client accepté avec fd = " << client_fd << std::endl;
    Client *newclient = new Client(client_fd);
    pollfd newfd;
    newfd.fd = client_fd;
    newfd.events = POLLIN;
    fds.push_back(newfd);
    clients[client_fd] = newclient;
    std::cout << "client ajouter avec fd = " << client_fd << std::endl;
}

/**
 * @brief Boucle principale du serveur
 */
void Webserv::run()
{
    while (true)
    {
        int ret = poll(fds.data(), fds.size(), -1);
        if (ret < 0)
            throw(std::runtime_error("poll failed"));
        for (std::vector<pollfd>::iterator it = fds.begin(); it != fds.end(); it++)
        {
            if (serveurs.count(it->fd))
            {
                if (it->revents & POLLIN)
                    acceptNewClient(*serveurs[it->fd]);
            }
            if (clients.count(it->fd) && it->revents & POLLIN)
            {
                try {
                    Request req(it->fd);
                    Response resp;
                    std::string path = req.getPath();
                    if (path.find(".css") != std::string::npos) {
                        std::string file_path = "." + path;
                        std::ifstream css_file(file_path.c_str());
                        if (css_file.is_open()) {
                            std::stringstream buffer;
                            buffer << css_file.rdbuf();
                            resp.setStatusCode("200 OK");
                            resp.setContentType("text/css");
                            resp.setContent(buffer.str());
                            css_file.close();
                        } else {
                            resp.setStatusCode("404 Not Found");
                            resp.setContentType("text/css");
                            resp.setContent("/* CSS file not found */");
                        }
                    } else {
                        resp.setStatusCode("404 Not Found");
                        resp.loadErrorPage("./config/html/404.html");
                    }
                    std::string response_str = resp.build();
                    send(it->fd, response_str.c_str(), response_str.length(), 0);
                    close(it->fd);
                    delete clients[it->fd];
                    clients.erase(it->fd);
                    it->fd = -1;
                } catch (std::exception &e) {
                    std::cerr << "Erreur lors du traitement de la requête: " << e.what() << std::endl;
                    close(it->fd);
                    delete clients[it->fd];
                    clients.erase(it->fd);
                    it->fd = -1;
                }
            } 
        }
        for (size_t i = 0; i < fds.size(); ++i) {
            if (fds[i].fd == -1) {
                fds.erase(fds.begin() + i);
                --i;
            }
        }
    }
}

/**
 * @brief Charge la configuration depuis un fichier et crée les serveurs
 * @param filename Chemin vers le fichier de configuration
 */
void Webserv::storeServers(std::string &filename)
{
    deleteAndNull(rootConfig);
    try
    {
        std::vector<Token> tokens = tokenizeConfigFile(filename);
        std::vector<Token> filteredTokens = filterTokens(tokens, filename);
        rootConfig = parseTokens(filteredTokens, filename);
        validateAndBuildServers(rootConfig, filename);
    }
    catch (const std::exception &e)
    {
        deleteAndNull(rootConfig);
        throw std::runtime_error(e.what());
    }
}

/**
 * @brief Initialise les serveurs réels à partir de la configuration parsée
 */
void Webserv::launchServers()
{
    std::cout << "Lancement des serveurs..." << std::endl;
    for (std::map<int, Serveur*>::iterator it = serveurs.begin(); it != serveurs.end(); ++it)
        delete it->second;
    serveurs.clear();
    fds.clear();
    if (serveurs.empty())
    {
        Serveur *defaultServer = new Serveur();
        pollfd newfd;
        newfd.fd = defaultServer->getfd();
        newfd.events = POLLIN;
        fds.push_back(newfd);
        serveurs[defaultServer->getfd()] = defaultServer;
        std::cout << "Default server added on fd = " << defaultServer->getfd() << std::endl;
    }
    for (std::map<int, Serveur*>::iterator it = serveurs.begin(); it != serveurs.end(); ++it)
    {
        pollfd newfd;
        newfd.fd = it->first;
        newfd.events = POLLIN;
        fds.push_back(newfd);
        std::cout << "Server listening on fd = " << it->first << std::endl;
    }
}

/**
 * @brief Affiche la configuration
 * @param node Nœud à afficher
 * @param depth Profondeur d'indentation
 */
void Webserv::displayConfig(ConfigNode *node, int depth)
{
    if (!node)
        return;
    std::string indent(depth * 4, ' ');
    std::cout << indent << node->type;
    if (!node->value.empty())
        std::cout << " " << node->value;
    std::cout << " {" << std::endl;
    for (std::map<std::string, std::vector<std::string> >::const_iterator it = 
        node->directives.begin(); it != node->directives.end(); ++it)
    {
        std::cout << indent << "    " << it->first << " ";
        const std::vector<std::string> &values = it->second;
        for (size_t i = 0; i < values.size(); ++i)
            std::cout << values[i] << " ";
        std::cout << ";" << std::endl;
    }
    for (size_t i = 0; i < node->children.size(); ++i)
        displayConfig(node->children[i], depth + 1);
    std::cout << indent << "}" << std::endl;
}
