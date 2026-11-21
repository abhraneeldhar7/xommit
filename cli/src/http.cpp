#include "xommit/http.h"

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0A00
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <winhttp.h>

#include <stdexcept>

#include "xommit/util.h"
#include "xommit/version.h"

namespace xommit {

namespace {

struct InternetHandle {
    HINTERNET value = nullptr;
    ~InternetHandle() {
        if (value != nullptr) {
            WinHttpCloseHandle(value);
        }
    }
};

std::wstring request_path(const URL_COMPONENTS& components) {
    std::wstring path(components.lpszUrlPath, components.dwUrlPathLength);
    if (components.dwExtraInfoLength > 0) {
        path.append(components.lpszExtraInfo, components.dwExtraInfoLength);
    }
    return path;
}

void throw_last_error(const std::string& action) {
    DWORD code = GetLastError();
    throw std::runtime_error(action + " failed: " + windows_error(code));
}

}

HttpResponse http_request(const std::string& method,
                          const std::string& url,
                          const std::vector<std::string>& headers,
                          const std::string& body) {
    std::wstring wide_url = utf8_to_wide(url);
    URL_COMPONENTS components{};
    components.dwStructSize = sizeof(components);
    components.dwSchemeLength = static_cast<DWORD>(-1);
    components.dwHostNameLength = static_cast<DWORD>(-1);
    components.dwUrlPathLength = static_cast<DWORD>(-1);
    components.dwExtraInfoLength = static_cast<DWORD>(-1);

    if (!WinHttpCrackUrl(wide_url.c_str(), static_cast<DWORD>(wide_url.size()), 0, &components)) {
        throw_last_error("parsing URL");
    }

    std::wstring host(components.lpszHostName, components.dwHostNameLength);
    std::wstring path = request_path(components);
    INTERNET_PORT port = components.nPort;
    bool secure = components.nScheme == INTERNET_SCHEME_HTTPS;

    std::wstring user_agent = utf8_to_wide("xommit/" XOMMIT_VERSION);
    InternetHandle session;
    session.value = WinHttpOpen(user_agent.c_str(), WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY,
                                WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (session.value == nullptr) {
        session.value = WinHttpOpen(user_agent.c_str(), WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                                    WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    }
    if (session.value == nullptr) {
        throw_last_error("opening HTTP session");
    }

    WinHttpSetTimeouts(session.value, 10000, 10000, 30000, 60000);

    InternetHandle connection;
    connection.value = WinHttpConnect(session.value, host.c_str(), port, 0);
    if (connection.value == nullptr) {
        throw_last_error("connecting to host");
    }

    InternetHandle request;
    request.value = WinHttpOpenRequest(connection.value, utf8_to_wide(method).c_str(), path.c_str(),
                                       nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES,
                                       secure ? WINHTTP_FLAG_SECURE : 0);
    if (request.value == nullptr) {
        throw_last_error("opening request");
    }

    std::wstring header_block;
    for (const std::string& header : headers) {
        header_block += utf8_to_wide(header);
        header_block += L"\r\n";
    }
    if (!header_block.empty()) {
        WinHttpAddRequestHeaders(request.value, header_block.c_str(),
                                 static_cast<DWORD>(header_block.size()),
                                 WINHTTP_ADDREQ_FLAG_ADD | WINHTTP_ADDREQ_FLAG_REPLACE);
    }

    LPVOID body_pointer = body.empty() ? WINHTTP_NO_REQUEST_DATA
                                       : const_cast<char*>(body.data());
    if (!WinHttpSendRequest(request.value, WINHTTP_NO_ADDITIONAL_HEADERS, 0, body_pointer,
                            static_cast<DWORD>(body.size()), static_cast<DWORD>(body.size()), 0)) {
        throw_last_error("sending request");
    }

    if (!WinHttpReceiveResponse(request.value, nullptr)) {
        throw_last_error("receiving response");
    }

    DWORD status = 0;
    DWORD status_size = sizeof(status);
    if (!WinHttpQueryHeaders(request.value, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                             WINHTTP_HEADER_NAME_BY_INDEX, &status, &status_size,
                             WINHTTP_NO_HEADER_INDEX)) {
        throw_last_error("reading status code");
    }

    std::string response_body;
    while (true) {
        DWORD available = 0;
        if (!WinHttpQueryDataAvailable(request.value, &available)) {
            throw_last_error("reading response");
        }
        if (available == 0) {
            break;
        }
        std::string chunk(available, '\0');
        DWORD read = 0;
        if (!WinHttpReadData(request.value, chunk.data(), available, &read)) {
            throw_last_error("reading response");
        }
        response_body.append(chunk.data(), read);
    }

    HttpResponse response;
    response.status = static_cast<int>(status);
    response.body = std::move(response_body);
    return response;
}

void http_download(const std::string& url, const std::string& path) {
    HttpResponse response = http_request("GET", url, {"User-Agent: xommit/" XOMMIT_VERSION}, "");
    if (response.status != 200) {
        throw std::runtime_error("download failed (HTTP " + std::to_string(response.status) + ")");
    }
    write_file(path, response.body);
}

}
