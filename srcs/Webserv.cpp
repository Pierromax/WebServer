/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cezou <cezou@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/05 19:04:30 by ple-guya          #+#    #+#             */
/*   Updated: 2025/04/08 17:01:00 by cezou            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Webserv.hpp"
#include "Utils.hpp"
#include "Server.hpp"
#include <sstream>
#include <csignal>

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
    serveurs[newfd.fd] = newServer;
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
    for (std::map<int, Server*>::iterator it = serveurs.begin(); it != serveurs.end(); ++it)
        delete it->second;
    for (std::map<int, Client*>::iterator it = clients.begin(); it != clients.end(); ++it)
        delete it->second;
    for (std::vector<pollfd>::iterator it = fds.begin(); it != fds.end(); ++it)
        close(it->fd);
}

/**
 * @brief Accepte un nouveau client sur une connexion serveur
 * @param server Référence au serveur acceptant la connexion
 */
void Webserv::acceptNewClient(const Server &server)
{
    std::cout << "Tentative d'accepter un nouveau client depuis le serveur fd = "
              << server.getfd() << std::endl;
    sockaddr_in serveuraddr = server.getAddress();
    socklen_t addrlen = sizeof(serveuraddr);
    int client_fd = accept(server.getfd(), (sockaddr *)&serveuraddr, &addrlen);
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
 * @brief Removes closed file descriptors from the poll array
 */
void Webserv::cleanInvalidFileDescriptors()
{
    for (size_t i = 0; i < fds.size(); ++i)
        if (fds[i].fd == -1)
        {
            fds.erase(fds.begin() + i);
            --i;
        }
}

/**
 * @brief Processes a client request and generates a response
 * @param client_fd Client file descriptor
 * @return Response string to send back to client
 */
std::string Webserv::processRequest(int client_fd)
{
    Request req(client_fd);
    Response resp;
    std::string path = req.getPath();
    
    if (path.find(".css") != std::string::npos)
    {
        std::string file_path = "." + path;
        std::ifstream css_file(file_path.c_str());
        if (css_file.is_open())
        {
            std::stringstream buffer;
            buffer << css_file.rdbuf();
            resp.setStatusCode("200 OK");
            resp.setContentType("text/css");
            resp.setContent(buffer.str());
            css_file.close();
        }
        else
        {
            resp.setStatusCode("404 Not Found");
            resp.setContentType("text/css");
            resp.setContent("/* CSS file not found */");
        }
    }
    else
    {
        resp.setStatusCode("404 Not Found");
        resp.loadErrorPage("./config/html/404.html");
    }
    
    return resp.build();
}

/**
 * @brief Closes client connection and cleans up resources
 * @param clientFd Client file descriptor to clean up
 * @param it Iterator pointing to the client's pollfd entry
 */
void Webserv::closeClientConnection(int clientFd, std::vector<pollfd>::iterator &it)
{
    close(clientFd);
    delete clients[clientFd];
    clients.erase(clientFd);
    it->fd = -1;
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
                continue;
            std::cerr << "Poll error: " << strerror(errno) << std::endl;
            break;
        }
        
        bool clientProcessed = false;
        for (std::vector<pollfd>::iterator it = fds.begin(); it != fds.end() && !clientProcessed; ++it)
        {
            if (serveurs.count(it->fd) && (it->revents & POLLIN)) {
                acceptNewClient(*serveurs[it->fd]);
                break;
            }
            else if (clients.count(it->fd) && (it->revents & POLLIN))
            {
                try {
                    std::string response_str = processRequest(it->fd);
                    send(it->fd, response_str.c_str(), response_str.length(), 0);
                    closeClientConnection(it->fd, it);
                } catch (std::exception &e) {
                    std::cerr << "Error processing request: " << e.what() << std::endl;
                    closeClientConnection(it->fd, it);
                }
                clientProcessed = true;
            } 
        }
        cleanInvalidFileDescriptors();
    }
    std::cout << "Webserver shutting down" << std::endl;
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
    
    for (std::map<int, Server*>::iterator it = serveurs.begin(); it != serveurs.end(); ++it)
        delete it->second;
    serveurs.clear();
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
            serveurs[newfd.fd] = newServer;
            std::cout << "Server added on fd = " << newfd.fd 
                      << " (port " << ntohs(newServer->getAddress().sin_port) << ")" << std::endl;
        }
    }
    
    // Si aucun serveur n'a été créé, utiliser un serveur par défaut
    if (serveurs.empty())
    {
        Server *defaultServer = new Server();
        pollfd newfd;
        newfd.fd = defaultServer->getfd();
        newfd.events = POLLIN;
        fds.push_back(newfd);
        serveurs[newfd.fd] = defaultServer;
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
