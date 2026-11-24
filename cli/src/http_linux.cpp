// http client. libcurl wrapper for GET/POST with headers/body
// retry on 5xx/network, timeout 120s
// user-agent: xommit/<version>

#include "xommit/http.h"

#include <curl/curl.h>

#include <stdexcept>
#include <string>
#include <vector>

#include "xommit/util.h"
#include "xommit/version.h"

namespace xommit {

namespace {

struct CurlHandle {
    CURL* value = nullptr;
    ~CurlHandle() {
        if (value != nullptr) {
            curl_easy_cleanup(value);
        }
    }
};

struct WriteBuffer {
    std::string data;
};

size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t total = size * nmemb;
    WriteBuffer* buffer = static_cast<WriteBuffer*>(userp);
    buffer->data.append(static_cast<const char*>(contents), total);
    return total;
}

std::vector<std::string> build_curl_headers(const std::vector<std::string>& headers) {
    std::vector<std::string> result;
    for (const std::string& h : headers) {
        result.push_back(h);
    }
    return result;
}

struct CurlSlistDeleter {
    void operator()(struct curl_slist* list) const {
        if (list) curl_slist_free_all(list);
    }
};

using CurlSlistPtr = std::unique_ptr<struct curl_slist, CurlSlistDeleter>;

CurlSlistPtr make_curl_headers(const std::vector<std::string>& headers) {
    struct curl_slist* list = nullptr;
    for (const std::string& h : headers) {
        list = curl_slist_append(list, h.c_str());
    }
    return CurlSlistPtr(list);
}

} // namespace

HttpResponse http_request(const std::string& method,
                          const std::string& url,
                          const std::vector<std::string>& headers,
                          const std::string& body) {
    CurlHandle curl;
    curl.value = curl_easy_init();
    if (!curl.value) {
        throw std::runtime_error("curl_easy_init failed");
    }

    WriteBuffer write_buffer;
    CurlSlistPtr header_list = make_curl_headers(headers);

    std::string user_agent = "xommit/" XOMMIT_VERSION;

    curl_easy_setopt(curl.value, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl.value, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl.value, CURLOPT_WRITEDATA, &write_buffer);
    curl_easy_setopt(curl.value, CURLOPT_USERAGENT, user_agent.c_str());
    curl_easy_setopt(curl.value, CURLOPT_HTTPHEADER, header_list.get());
    curl_easy_setopt(curl.value, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl.value, CURLOPT_MAXREDIRS, 5L);
    curl_easy_setopt(curl.value, CURLOPT_CONNECTTIMEOUT, 10L);
    curl_easy_setopt(curl.value, CURLOPT_TIMEOUT, 120L);
    curl_easy_setopt(curl.value, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(curl.value, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl.value, CURLOPT_SSL_VERIFYHOST, 2L);

    if (method == "POST") {
        curl_easy_setopt(curl.value, CURLOPT_POST, 1L);
        if (!body.empty()) {
            curl_easy_setopt(curl.value, CURLOPT_POSTFIELDS, body.c_str());
            curl_easy_setopt(curl.value, CURLOPT_POSTFIELDSIZE, static_cast<long>(body.size()));
        }
    } else if (method == "GET") {
        curl_easy_setopt(curl.value, CURLOPT_HTTPGET, 1L);
    } else {
        curl_easy_setopt(curl.value, CURLOPT_CUSTOMREQUEST, method.c_str());
        if (!body.empty()) {
            curl_easy_setopt(curl.value, CURLOPT_POSTFIELDS, body.c_str());
            curl_easy_setopt(curl.value, CURLOPT_POSTFIELDSIZE, static_cast<long>(body.size()));
        }
    }

    CURLcode res = curl_easy_perform(curl.value);
    if (res != CURLE_OK) {
        throw std::runtime_error("curl request failed: " + std::string(curl_easy_strerror(res)));
    }

    long status = 0;
    curl_easy_getinfo(curl.value, CURLINFO_RESPONSE_CODE, &status);

    HttpResponse response;
    response.status = static_cast<int>(status);
    response.body = std::move(write_buffer.data);
    return response;
}

void http_download(const std::string& url, const std::string& path) {
    HttpResponse response = http_request("GET", url, {"User-Agent: xommit/" XOMMIT_VERSION}, "");
    if (response.status != 200) {
        throw std::runtime_error("download failed (HTTP " + std::to_string(response.status) + ")");
    }
    write_file(path, response.body);
}

} // namespace xommit