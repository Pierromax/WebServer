/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIsHandling.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cviegas <cviegas@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/04 10:00:00 by cezou             #+#    #+#             */
/*   Updated: 2025/06/12 17:20:49 by cviegas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CGIsHandling.hpp"
#include "Webserv.hpp"
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <cstdlib>
#include <iostream>
#include <sstream>

/**
 * @brief Default constructor
 */
CGIsHandling::CGIsHandling() : scriptPath("")
{
}

/**
 * @brief Copy constructor
 * @param other Object to copy
 */
CGIsHandling::CGIsHandling(const CGIsHandling &other)
{
    *this = other;
}

/**
 * @brief Assignment operator
 * @param other Object to assign from
 * @return Reference to this object
 */
CGIsHandling &CGIsHandling::operator=(const CGIsHandling &other)
{
    if (this != &other)
    {
        cgiHandlers = other.cgiHandlers;
        scriptPath = other.scriptPath;
        envVars = other.envVars;
    }
    return *this;
}

/**
 * @brief Destructor
 */
CGIsHandling::~CGIsHandling()
{
}

/**
 * @brief Constructor with extension and CGI path
 * @param ext File extension to handle
 * @param path Path to CGI executable
 */
CGIsHandling::CGIsHandling(const std::string &ext, const std::string &path) : scriptPath("")
{
    cgiHandlers[ext] = path;
}

/**
 * @brief Constructor with map of handlers
 * @param handlers Map of extension to CGI path
 */
CGIsHandling::CGIsHandling(const std::map<std::string, std::string> &handlers) : cgiHandlers(handlers), scriptPath("")
{
}

/**
 * @brief Adds a CGI handler for an extension
 * @param ext File extension to handle
 * @param path Path to CGI executable
 */
void CGIsHandling::addCgiHandler(const std::string &ext, const std::string &path)
{
    cgiHandlers[ext] = path;
}

/**
 * @brief Sets the script path to execute
 * @param path Path to the script file
 */
void CGIsHandling::setScriptPath(const std::string &path)
{
    scriptPath = path;
}

/**
 * @brief Sets an environment variable for CGI execution
 * @param key Environment variable name
 * @param value Environment variable value
 */
void CGIsHandling::setEnvironmentVariable(const std::string &key, const std::string &value)
{
    envVars[key] = value;
}

/**
 * @brief Gets the CGI path for a given file
 * @param filePath Path to the file
 * @return CGI executable path or empty string if not found
 */
std::string CGIsHandling::getCgiPath(const std::string &filePath) const
{
    size_t dotPos = filePath.rfind('.');
    if (dotPos == std::string::npos)
        return "";
    std::string ext = filePath.substr(dotPos + 1);
    std::map<std::string, std::string>::const_iterator it = cgiHandlers.find(ext);
    if (it != cgiHandlers.end())
        return it->second;
    return "";
}

/**
 * @brief Executes the CGI script and returns output
 * @return CGI script output as string
 */
std::string CGIsHandling::execute()
{
    int pipefdOut[2];
    int pipefdIn[2];
    int pipefdErr[2];
    pid_t pid;
    std::string cgiPath = getCgiPath(scriptPath);
    if (cgiPath.empty())
        throw std::runtime_error("No CGI handler found for file: " + scriptPath);
    createPipes(pipefdOut, pipefdIn, pipefdErr);
    pid = fork();
    if (pid == -1)
    {
        closeAllPipes(pipefdOut, pipefdIn, pipefdErr);
        throw std::runtime_error("Failed to fork for CGI execution");
    }
    if (pid == 0)
    {
        executeChildProcess(pipefdOut, pipefdIn, pipefdErr, cgiPath);
        exit(EXIT_FAILURE);
    }
    else
    {
        return handleParentProcess(pid, pipefdOut, pipefdIn, pipefdErr);
    }
}

/**
 * @brief Creates the three pipes needed for CGI communication
 * @param pipefdOut Pipe for stdout
 * @param pipefdIn Pipe for stdin
 * @param pipefdErr Pipe for stderr
 */
void CGIsHandling::createPipes(int pipefdOut[2], int pipefdIn[2], int pipefdErr[2])
{
    if (pipe(pipefdOut) == -1 || pipe(pipefdIn) == -1 || pipe(pipefdErr) == -1)
        throw std::runtime_error("Failed to create pipes for CGI");
}

/**
 * @brief Closes all pipe file descriptors
 * @param pipefdOut Pipe for stdout
 * @param pipefdIn Pipe for stdin
 * @param pipefdErr Pipe for stderr
 */
void CGIsHandling::closeAllPipes(int pipefdOut[2], int pipefdIn[2], int pipefdErr[2])
{
    close(pipefdOut[0]);
    close(pipefdOut[1]);
    close(pipefdIn[0]);
    close(pipefdIn[1]);
    close(pipefdErr[0]);
    close(pipefdErr[1]);
}

/**
 * @brief Handles the child process execution
 * @param pipefdOut Pipe for stdout
 * @param pipefdIn Pipe for stdin
 * @param pipefdErr Pipe for stderr
 * @param cgiPath Path to the CGI executable
 */
void CGIsHandling::executeChildProcess(int pipefdOut[2], int pipefdIn[2], int pipefdErr[2], const std::string &cgiPath)
{
    setupChildRedirection(pipefdOut, pipefdIn, pipefdErr);
    std::vector<char*> envp = prepareEnvironment();
    char *args[] = {const_cast<char*>(cgiPath.c_str()), 
                   const_cast<char*>(scriptPath.c_str()), NULL};
    execve(cgiPath.c_str(), args, &envp[0]);
    std::cerr << "CGI execution failed: " << strerror(errno) << std::endl;
    cleanupEnvironment(envp);
    exit(EXIT_FAILURE);
}

/**
 * @brief Sets up pipe redirection for child process
 * @param pipefdOut Pipe for stdout
 * @param pipefdIn Pipe for stdin
 * @param pipefdErr Pipe for stderr
 */
void CGIsHandling::setupChildRedirection(int pipefdOut[2], int pipefdIn[2], int pipefdErr[2])
{
    close(pipefdOut[0]);
    close(pipefdIn[1]);
    close(pipefdErr[0]);
    
    if (dup2(pipefdOut[1], STDOUT_FILENO) == -1 || 
        dup2(pipefdIn[0], STDIN_FILENO) == -1 ||
        dup2(pipefdErr[1], STDERR_FILENO) == -1)
        exit(EXIT_FAILURE);
        
    close(pipefdOut[1]);
    close(pipefdIn[0]);
    close(pipefdErr[1]);
}

/**
 * @brief Prepares environment variables for CGI execution
 * @return Vector of C-style environment strings
 */
std::vector<char*> CGIsHandling::prepareEnvironment()
{
    std::vector<char*> envp;
    
    for (std::map<std::string, std::string>::iterator it = envVars.begin(); it != envVars.end(); ++it)
    {
        if (it->first == "REQUEST_BODY")
            continue;
        std::string envVar = it->first + "=" + it->second;
        char* envStr = new char[envVar.length() + 1];
        std::strcpy(envStr, envVar.c_str());
        envp.push_back(envStr);
    }
    envp.push_back(NULL);
    
    return envp;
}

/**
 * @brief Cleans up environment variable memory
 * @param envp Vector of environment strings to clean
 */
void CGIsHandling::cleanupEnvironment(std::vector<char*>& envp)
{
    for (size_t i = 0; i < envp.size(); ++i)
        delete[] envp[i];
}

/**
 * @brief Handles parent process communication and output collection
 * @param pid Child process ID
 * @param pipefdOut Pipe for stdout
 * @param pipefdIn Pipe for stdin
 * @param pipefdErr Pipe for stderr
 * @return Formatted CGI output
 */
std::string CGIsHandling::handleParentProcess(pid_t pid, int pipefdOut[2], int pipefdIn[2], int pipefdErr[2])
{
    close(pipefdOut[1]);
    close(pipefdIn[0]);
    close(pipefdErr[1]);
    
    writeRequestBody(pipefdIn[1]);
    close(pipefdIn[1]);
    
    std::string errorOutput;
    std::string output = readProcessOutput(pipefdOut[0], pipefdErr[0], errorOutput);
    
    close(pipefdOut[0]);
    close(pipefdErr[0]);
    waitpid(pid, NULL, 0);
    
    return formatFinalOutput(output, errorOutput);
}

/**
 * @brief Writes POST request body to CGI stdin
 * @param pipefd File descriptor for writing
 */
void CGIsHandling::writeRequestBody(int pipefd)
{
    std::map<std::string, std::string>::iterator it = envVars.find("REQUEST_BODY");
    if (it != envVars.end() && !it->second.empty())
    {
        write(pipefd, it->second.c_str(), it->second.length());
    }
}

/**
 * @brief Reads output from both stdout and stderr pipes
 * @param pipefdOut Stdout pipe file descriptor
 * @param pipefdErr Stderr pipe file descriptor
 * @param errorOutput Reference to store stderr content
 * @return Stdout content
 */
std::string CGIsHandling::readProcessOutput(int pipefdOut, int pipefdErr, std::string& errorOutput)
{
    std::string output;
    char buffer[4096];
    char errorBuffer[4096];
    ssize_t bytesRead;
    
    while ((bytesRead = read(pipefdOut, buffer, sizeof(buffer) - 1)) > 0)
    {
        buffer[bytesRead] = '\0';
        output += buffer;
    }
    
    while ((bytesRead = read(pipefdErr, errorBuffer, sizeof(errorBuffer) - 1)) > 0)
    {
        errorBuffer[bytesRead] = '\0';
        errorOutput += errorBuffer;
    }
    
    return output;
}

/**
 * @brief Formats the final output combining stdout and stderr
 * @param output Main CGI output
 * @param errorOutput Error output from stderr
 * @return Formatted final output
 */
std::string CGIsHandling::formatFinalOutput(const std::string& output, const std::string& errorOutput)
{
    if (!errorOutput.empty() && !output.empty())
    {
        // Ajouter les erreurs à la fin du contenu principal
        std::string result = output;
        result += "\n<!-- CGI Debug Output -->\n";
        result += "<div style='background-color: #fff3cd; color: #856404; padding: 10px; margin-top: 20px; border-radius: 4px; font-family: monospace; font-size: 12px;'>";
        result += "<strong>Debug CGI:</strong><br><pre>" + errorOutput + "</pre></div>";
        return result;
    }
    else if (!errorOutput.empty() && output.empty())
    {
        // Si pas de sortie mais des erreurs, montrer seulement les erreurs
        std::string errorPage = "<html><head><title>CGI Error</title></head><body>";
        errorPage += "<div style='background-color: #f8d7da; color: #721c24; padding: 15px; border-radius: 4px;'>";
        errorPage += "<strong>Erreur CGI:</strong><br><pre>" + errorOutput + "</pre></div></body></html>";
        return errorPage;
    }
    
    return output;
}

/**
 * @brief Checks if file path has a supported extension
 * @param filePath Path to check
 * @return true if extension is supported
 */
bool CGIsHandling::isValidExtension(const std::string &filePath) const
{
    size_t dotPos = filePath.rfind('.');
    if (dotPos == std::string::npos)
        return false;
    std::string ext = filePath.substr(dotPos + 1);
    return cgiHandlers.find(ext) != cgiHandlers.end();
}

/**
 * @brief Finds appropriate CGI handler for file path
 * @param filePath Path to the file
 * @param contextNode Configuration context to search
 * @return CGI handler or NULL if none found
 */
CGIsHandling* CGIsHandling::findCgiHandler(const std::string &filePath, ConfigNode* contextNode)
{
    ConfigNode* searchNode = contextNode;
    std::map<std::string, std::string> allHandlers;
    while (searchNode)
    {
        for (std::map<std::string, std::string>::iterator it = searchNode->cgiHandlers.begin();
             it != searchNode->cgiHandlers.end(); ++it)
        {
            if (allHandlers.find(it->first) == allHandlers.end())
                allHandlers[it->first] = it->second;
        }
        searchNode = searchNode->parent;
    }
    if (allHandlers.empty())
        return NULL;
    CGIsHandling* handler = new CGIsHandling(allHandlers);
    if (handler->isValidExtension(filePath))
        return handler;
    delete handler;
    return NULL;
}