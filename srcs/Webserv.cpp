/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ple-guya <ple-guya@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/05 19:04:30 by ple-guya          #+#    #+#             */
/*   Updated: 2025/04/18 19:07:25 by ple-guya         ###   ########.fr       */
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
    for (std::map<int, Client*>::iterator it = clients.begin(); it != clients.end(); ++it)
        delete it->second;
    for (std::vector<pollfd>::iterator it = fds.begin(); it != fds.end(); ++it)
        close(it->fd);
}

/**
 * @brief Accepte un nouveau client sur une connexion serveur
 * @param server Pointeur vers le serveur acceptant la connexion
 */
void Webserv::acceptNewClient(Server *server)
{
    if (!server) return; // Safety check

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
    
    newfd.fd = client_fd;
    newfd.events = POLLIN;
    newfd.revents = 0;
    fds.push_back(newfd);
    clients[client_fd] = newclient;
    std::cout << "client ajouter avec fd = " << client_fd << std::endl;
}

/**
 * @brief Removes closed file descriptors from the poll array
 */
void Webserv::cleanInvalidFileDescriptors()
{
    for (size_t i = 0; i < fds.size(); ++i)
    {
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
    std::cout << "Closing connection for fd = " << clientFd << std::endl;
    close(clientFd);
    delete clients[clientFd];
    clients.erase(clientFd);

    // Mark the corresponding pollfd in the main fds vector for removal
    for (std::vector<pollfd>::iterator pfd_it = fds.begin(); pfd_it != fds.end(); ++pfd_it)
    {
        if (pfd_it->fd == clientFd)
        {
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
        for (std::vector<pollfd>::iterator it = fds.begin(); it != fds.end(); ++it)
        {
            if (servers.count(it->fd))
            {
                active_servers.push_back(*it);
            }
            else if (clients.count(it->fd))
            {
                active_clients.push_back(*it);
            }
        }
        try {
            handleServers();
            handleClients();
        }
        catch (std::exception &e) {
            std::cerr << "Error processing request: " << e.what() << std::endl;
        }
    }
    std::cout << "Webserver shutting down" << std::endl;
}

void Webserv::handleServers()
{
    std::vector<pollfd>::iterator it;
    for (it = active_servers.begin(); it != active_servers.end(); it++)
    {
        if (it->revents & POLLERR)
        {
            std::cerr << "Error on server fd = " << it->fd << std::endl;
            continue;
        }
        try {
            acceptNewClient(servers[it->fd]); // Pass Server*
        } catch (const std::exception &e){
            std::cerr << "Error accepting new client: " << e.what() << std::endl;
        }
    }
    active_servers.clear();
}

void Webserv::handleClients()
{
    std::vector<pollfd>::iterator it;
    
    for (it = active_clients.begin(); it != active_clients.end(); ++it)
    {
        int currentFd = it->fd;
        bool closeConn = false;

        if (it->revents & (POLLERR | POLLHUP))
        {
            std::cerr << "Error or HUP on client fd = " << currentFd << std::endl;
            closeConn = true;
        }
        else if (it->revents & POLLIN)
        {
            Client* client = clients[currentFd]; // Get client object
            if (!client) continue; // Should not happen, but safety check
            Server* server = client->getServer(); // Get associated server
            if (!server) continue; // Should not happen

            Request req(currentFd); // Read request data
            ssize_t bytesRead = req.getBytesRead();

            if (bytesRead <= 0)
            {
                if (bytesRead == 0)
                {
                    std::cout << "Client fd = " << currentFd << " disconnected." << std::endl;
                    closeConn = true;
                }
                else
                {
                    if (errno != EAGAIN && errno != EWOULDBLOCK)
                    {
                        std::cerr << "Recv error on client fd = " << currentFd << ": " << strerror(errno) << std::endl;
                        closeConn = true;
                    }
                }
            }
            else
            {
                if (!req.isEmptyInput())
                {
                    try {
                        if (req.getStatusCode() != GOOD_REQUEST)
                        {
                            std::cerr << "Bad request from fd = " << currentFd << " (" << req.getStatusCode() << ")" << std::endl;
                            Response resp(req, server); // Pass server pointer
                            std::string to_send = resp.build();
                            if (send(currentFd, to_send.c_str(), to_send.length(), 0) < 0)
                                std::cerr << "Error sending error response to fd = " << currentFd << ": " << strerror(errno) << std::endl;
                            closeConn = true;
                        }
                        else
                        {
                            Response resp(req, server); // Pass server pointer
                            std::string to_send = resp.build();
                            ssize_t bytes_sent = send(currentFd, to_send.c_str(), to_send.length(), 0);
                            if (bytes_sent < 0)
                            {
                                std::cerr << "Error sending response to fd = " << currentFd << ": " << strerror(errno) << std::endl;
                                closeConn = true;
                            } else
                                std::cout << "Response sent to fd = " << currentFd << "." << std::endl;
                        }
                    } catch (const std::exception &e) {
                        std::cerr << "Exception processing client fd = " << currentFd << ": " << e.what() << std::endl;
                        closeConn = true;
                    }
                }
            }
        }
        if (closeConn)
            closeClientConnection(currentFd);
    }
    cleanInvalidFileDescriptors();
    active_clients.clear();
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
    for (size_t i = 0; i < node->children.size(); ++i)
        displayConfig(node->children[i], depth + 1);
    std::cout << indent << "}" << std::endl;
}
