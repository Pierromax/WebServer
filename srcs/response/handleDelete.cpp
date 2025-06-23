#include "Response.hpp"
#include "Request.hpp"

void Response::handleDeleteRequest(const Request &req)
{
	std::string path = req.getPath();
	ConfigNode* locationNode = findBestLocation(path, _server);
	try
	{
		if (!isMethodAllowed("DELETE", locationNode))
			throw std::runtime_error(METHOD_NOT_ALLOWED);
		if (req.getStatusCode() == PAYLOAD_TOO_LARGE)
			throw std::runtime_error(PAYLOAD_TOO_LARGE);
		std::string filePath = buildFullPath(locationNode, path);
		if (filePath.empty())
			throw std::runtime_error(FILE_NOT_FOUND);
		struct stat path_stat;
		if (stat(filePath.c_str(), &path_stat) != 0)
			throw std::runtime_error(FILE_NOT_FOUND);
		if (S_ISDIR(path_stat.st_mode))
			throw std::runtime_error(METHOD_NOT_ALLOWED);
		if (access(filePath.c_str(), W_OK) != 0)
			throw std::runtime_error(FORBIDDEN);
		if (remove(filePath.c_str()) != 0)
			throw std::runtime_error(INTERNAL_ERROR);
		d_cout << GREEN "File deleted successfully: " B << filePath << R << std::endl;
		status_code = NO_CONTENT;
		setBody("");
		setContentType("text/html");
	}
	catch (const std::runtime_error& e)
	{
		status_code = e.what();
		loadErrorPage(status_code.substr(0, 3), locationNode);
	}
}
