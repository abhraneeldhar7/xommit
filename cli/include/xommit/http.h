// http client. winhttp wrapper
// http_request: GET/POST with headers/body -> HttpResponse
// http_download: streaming download to file
// user-agent: xommit/<version>, 120s timeout, retry on 5xx

#pragma once

#include <string>
#include <vector>

namespace xommit {

struct HttpResponse {
    int status = 0;
    std::string body;
};

HttpResponse http_request(const std::string& method,
                          const std::string& url,
                          const std::vector<std::string>& headers,
                          const std::string& body);

void http_download(const std::string& url, const std::string& path);

}
