#include "Response.hpp"
#include "Request.hpp"
#include "Response.hpp"

void Response::handleDeleteRequest(const Request &req)
{
    std::string path = req.getPath();
    ConfigNode* locationNode = findBestLocation(path, _server);

    if (!isMethodAllowed("DELETE", locationNode))
    {
        status_code = METHOD_NOT_ALLOWED;
        loadErrorPage("405", locationNode);
        return;
    }
    if (req.getStatusCode() == PAYLOAD_TOO_LARGE)
    {
        status_code = PAYLOAD_TOO_LARGE;
        loadErrorPage("413", locationNode);
        return;
    }
    (void)req;
}
