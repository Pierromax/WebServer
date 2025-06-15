/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cviegas <cviegas@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/05 19:04:30 by ple-guya          #+#    #+#             */
/*   Updated: 2025/06/15 15:12:13 by cviegas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Webserv.hpp"
#include "Utils.hpp"
#include "Server.hpp"
#include <sstream>
#include <csignal>
#include <cerrno> // For errno

// Variable globale pour contrôler la boucle principale
volatile sig_atomic_t g_running = 1;

/**
 * @brief Signal handler for SIGINT (Ctrl+C)
 * @param sig Signal number
 */
void signalHandler(int sig)
{
    (void)sig;
    std::cout << std::endl << "Received SIGINT, shutting down..." << std::endl;
    g_running = 0;
}

/**
 * @brief Constructeur par défaut
 */
Webserv::Webserv() : rootConfig(NULL)
{
    Server *newServer = new Server();
    pollfd newfd;
    newfd.fd = newServer->getfd();
    newfd.events = POLLIN;
    newfd.revents = 0;
    fds.push_back(newfd);
    servers[newfd.fd] = newServer;
    std::cout << "Serveur ajouté à la map avec fd = " << newfd.fd << std::endl;
}

/**
 * @brief Constructeur avec fichier de configuration
 * @param config_filename Chemin vers le fichier de configuration
 */
Webserv::Webserv(std::string config_filename) : rootConfig(NULL)
{
    fds.reserve(20);

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
        this->servers = rhs.servers;
    }
    return (*this);
}

/**
 * @brief Destructeur
 */
Webserv::~Webserv()
{
    deleteAndNull(rootConfig);
    for (std::map<int, Server*>::iterator it = servers.begin(); it != servers.end(); ++it)
        delete it->second;
    servers.clear();
    for (std::map<int, Client*>::iterator it = clients.begin(); it != clients.end(); ++it)
        delete it->second;
    clients.clear();
    for (std::vector<pollfd>::iterator it = fds.begin(); it != fds.end(); ++it)
        close(it->fd);
    fds.clear();
}

/**
 * @brief Accepte un nouveau client sur une connexion serveur
 * @param server Pointeur vers le serveur acceptant la connexion
 */
void Webserv::acceptNewClient(Server *server)
{
    if (!server) return; // Safety check
    std::cout << "enter handle acceptnew client" << std::endl;
    
    sockaddr_in serveuraddr = server->getAddress();
    socklen_t addrlen = sizeof(serveuraddr);
    int client_fd = accept(server->getfd(), (sockaddr *)&serveuraddr, &addrlen);
    
    if (client_fd < 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            return;
        }
        std::cerr << "Erreur accept(): " << strerror(errno) << std::endl;
        throw std::runtime_error("Error: accept client failed");
    }
    
    std::cout << "Nouveau client accepté avec fd = " << client_fd << std::endl;
    Client *newclient = new Client(client_fd, server); // Pass server pointer
    pollfd newfd;
    std::cout << "client accepted" << std::endl;
    newfd.fd = client_fd;
    newfd.events = POLLIN;
    newfd.revents = 0;
    fds.push_back(newfd);
    clients[client_fd] = newclient;
}

/**
 * @brief Removes closed file descriptors from the poll array
 */
void Webserv::cleanInvalidFileDescriptors()
{
    for (size_t i = 0; i < fds.size(); ++i){
        if (fds[i].fd == -1)
        {
            fds.erase(fds.begin() + i);
            --i;
        }
    }
}

/**
 * @brief Closes client connection and cleans up resources
 * @param clientFd Client file descriptor to clean up
 */
void Webserv::closeClientConnection(int clientFd)
{
    if (clients.count(clientFd) == 0)
        return;
    std::cout << "connection close with fd = " << clientFd << std::endl; 
    close(clientFd);
    delete clients[clientFd];
    clients.erase(clientFd);

    for (std::vector<pollfd>::iterator pfd_it = fds.begin(); pfd_it != fds.end(); ++pfd_it)
    {
        if (pfd_it->fd == clientFd)
        {
            //std::cout << "[DEBUG] Marking pollfd as -1 for fd = " << clientFd << std::endl;
            pfd_it->fd = -1;
            break;
        }
    }
}

/**
 * @brief Main server loop
 */
void Webserv::run()
{
    std::cout << "Webserver running." << std::endl;
    g_running = 1;
    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT, signalHandler);
    
    while (g_running)
    {
        int ret = poll(fds.data(), fds.size(), 1000);
        if (ret < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }
            std::cerr << "Poll error: " << strerror(errno) << std::endl;
            break;
        }
        cleanInvalidFileDescriptors();
        for (std::vector<pollfd>::iterator it = fds.begin(); it != fds.end(); it++)
        {
            if (it->fd == -1)
                continue;
            if (it->revents == 0)  // Pas d'événements sur ce fd
                continue;
            if (servers.count(it->fd))
                handleServers(*it);
            else if (clients.count(it->fd))
                handleClients(*it);
        }
    }
    std::cout << "Webserver shutting down" << std::endl;
}

void Webserv::handleServers(pollfd &it)
{

    if (it.revents & POLLERR)
    {
        std::cerr << "Error on server fd = " << it.fd << std::endl;
        return;
    }
    if (it.revents & POLLIN)
    {
        try {
            acceptNewClient(servers[it.fd]);
        } catch (const std::exception &e){
            std::cerr << "Error accepting new client: " << e.what() << std::endl;
        }
    }
}

void Webserv::handleClients(pollfd &it)
{
    bool closeConn = false;

    if (it.revents & (POLLERR | POLLHUP))
    {
        std::cerr << "Error or HUP on client fd = " << it.fd << std::endl;
        closeConn = true;
    }
    if (it.revents & POLLIN)
    {
        clients[it.fd]->prepareRequest();
        if (clients[it.fd]->getState() == WRITING)
        {
            clients[it.fd]->prepareResponse();
            setPollEvent(it.fd, POLLOUT);
        }
    }
    else if (it.revents & POLLOUT && clients[it.fd]->response && !closeConn)
    {
        clients[it.fd]->sendResponse();
        if (clients[it.fd]->response->getConnectionType() == "keep-alive")
            setPollEvent(it.fd, POLLIN);
        else
            closeConn = true;
    }
    if (closeConn || clients[it.fd]->isTimeout())
        closeClientConnection(it.fd);
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
        validateServers(rootConfig, filename);
    }
    catch (const std::exception &e)
    {
        deleteAndNull(rootConfig);
        throw std::runtime_error(e.what());
    }
}

/**
 * @brief Initialise les serveurs à partir de la configuration parsée
 */
void Webserv::launchServers()
{
    std::cout << "Lancement des serveurs..." << std::endl;
    
    for (std::map<int, Server*>::iterator it = servers.begin(); it != servers.end(); ++it)
        delete it->second;
    servers.clear();    
    fds.clear();
    if (rootConfig && !rootConfig->children.empty())
    {
        for (size_t i = 0; i < rootConfig->children.size(); ++i)
        {
            ConfigNode *serverNode = rootConfig->children[i];
            if (serverNode->type != "server")
                continue;

            Server *newServer = new Server(serverNode);
            pollfd newfd;
            newfd.fd = newServer->getfd();
            newfd.events = POLLIN;
            newfd.revents = 0;
            fds.push_back(newfd);
            servers[newfd.fd] = newServer;
            std::cout << "Server added on fd = " << newfd.fd 
                      << " (port " << ntohs(newServer->getAddress().sin_port) << ")" << std::endl;
        }
    }
    
    if (servers.empty())
    {
        Server *defaultServer = new Server();
        pollfd newfd;
        newfd.fd = defaultServer->getfd();
        newfd.events = POLLIN;
        newfd.revents = 0;
        fds.push_back(newfd);
        servers[newfd.fd] = defaultServer;
        std::cout << "Default server added on fd = " << newfd.fd << std::endl;
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
    if (node->type == "root")
        std::cout << "\nParsed Configuration:" << std::endl;
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
    for (std::map<std::string, std::string>::const_iterator it = 
        node->cgiHandlers.begin(); it != node->cgiHandlers.end(); ++it)
    {
        std::cout << indent << "    cgi " << it->first << " " << it->second << " ;" << std::endl;
    }
    for (size_t i = 0; i < node->children.size(); ++i)
        displayConfig(node->children[i], depth + 1);
    std::cout << indent << "}" << std::endl;
}

void Webserv::setPollEvent(int fd, short events)
{
    for (size_t i = 0; i < fds.size(); ++i)
    {
        if (fds[i].fd == fd)
        {
            fds[i].events = events;
            break;
        }
    }
}
