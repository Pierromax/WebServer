/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIsHandling.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cviegas <cviegas@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/04 10:00:00 by cezou             #+#    #+#             */
/*   Updated: 2025/06/09 15:29:31 by cviegas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGISHANDLING_HPP
#define CGISHANDLING_HPP

#include <string>
#include <map>
#include <vector>
#include <unistd.h>

struct ConfigNode;

/**
 * @brief Handles CGI execution for specific file extensions
 * 
 * This class manages the execution of CGI scripts through fork/execve
 * and pipes for communication with the web server.
 */
class CGIsHandling
{
private:
    std::map<std::string, std::string> cgiHandlers;
    std::string scriptPath;
    std::map<std::string, std::string> envVars;

public:
    CGIsHandling();
    CGIsHandling(const CGIsHandling &other);
    CGIsHandling &operator=(const CGIsHandling &other);
    ~CGIsHandling();

    CGIsHandling(const std::string &ext, const std::string &path);
    CGIsHandling(const std::map<std::string, std::string> &handlers);
    
    void addCgiHandler(const std::string &ext, const std::string &path);
    void setScriptPath(const std::string &path);
    void setEnvironmentVariable(const std::string &key, const std::string &value);
    std::string execute();
    std::string getCgiPath(const std::string &filePath) const;
    bool isValidExtension(const std::string &filePath) const;
    
    static CGIsHandling* findCgiHandler(const std::string &filePath, ConfigNode* contextNode);

private:
    void createPipes(int pipefdOut[2], int pipefdIn[2], int pipefdErr[2]);
    void closeAllPipes(int pipefdOut[2], int pipefdIn[2], int pipefdErr[2]);
    void executeChildProcess(int pipefdOut[2], int pipefdIn[2], int pipefdErr[2], const std::string &cgiPath);
    void setupChildRedirection(int pipefdOut[2], int pipefdIn[2], int pipefdErr[2]);
    std::vector<char*> prepareEnvironment();
    void cleanupEnvironment(std::vector<char*>& envp);
    std::string handleParentProcess(pid_t pid, int pipefdOut[2], int pipefdIn[2], int pipefdErr[2]);
    void writeRequestBody(int pipefd);
    std::string readProcessOutput(int pipefdOut, int pipefdErr, std::string& errorOutput);
    std::string formatFinalOutput(const std::string& output, const std::string& errorOutput);
};

#endif