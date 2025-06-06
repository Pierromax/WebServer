/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIsHandling.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cezou <cezou@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/04 10:00:00 by cezou             #+#    #+#             */
/*   Updated: 2025/06/06 21:25:21 by cezou            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGISHANDLING_HPP
#define CGISHANDLING_HPP

#include <string>
#include <map>
#include <vector>

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
    std::string extension;
    std::string cgiPath;
    std::string scriptPath;
    std::map<std::string, std::string> envVars;

public:
    CGIsHandling();
    CGIsHandling(const CGIsHandling &other);
    CGIsHandling &operator=(const CGIsHandling &other);
    ~CGIsHandling();

    CGIsHandling(const std::string &ext, const std::string &path);
    
    void setScriptPath(const std::string &path);
    void setEnvironmentVariable(const std::string &key, const std::string &value);
    std::string execute();
    bool isValidExtension(const std::string &filePath) const;
    
    static CGIsHandling* findCgiHandler(const std::string &filePath, ConfigNode* contextNode);

private:
    void createPipes(int pipefdOut[2], int pipefdIn[2], int pipefdErr[2]);
    void closeAllPipes(int pipefdOut[2], int pipefdIn[2], int pipefdErr[2]);
    void executeChildProcess(int pipefdOut[2], int pipefdIn[2], int pipefdErr[2]);
    void setupChildRedirection(int pipefdOut[2], int pipefdIn[2], int pipefdErr[2]);
    std::vector<char*> prepareEnvironment();
    void cleanupEnvironment(std::vector<char*>& envp);
    std::string handleParentProcess(pid_t pid, int pipefdOut[2], int pipefdIn[2], int pipefdErr[2]);
    void writeRequestBody(int pipefd);
    std::string readProcessOutput(int pipefdOut, int pipefdErr, std::string& errorOutput);
    std::string formatFinalOutput(const std::string& output, const std::string& errorOutput);
};

#endif