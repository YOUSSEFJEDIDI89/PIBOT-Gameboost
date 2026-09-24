#include "http.hpp"
#include "util.hpp"

#ifdef PIBOT_USE_LIBCURL
#include <curl/curl.h>

static size_t writeCb(char* ptr, size_t size, size_t nmemb, void* ud) {
    auto* s = static_cast<std::string*>(ud);
    s->append(ptr, size * nmemb);
    return size * nmemb;
}

std::string httpPostJson(const std::string& url,
                         const std::vector<std::string>& headers,
                         const std::string& body, long timeoutSec) {
    CURL* h = curl_easy_init();
    if (!h) return "";
    std::string out;
    struct curl_slist* hs = nullptr;
    hs = curl_slist_append(hs, "Content-Type: application/json");
    for (auto& hd : headers) hs = curl_slist_append(hs, hd.c_str());

    curl_easy_setopt(h, CURLOPT_URL, url.c_str());
    curl_easy_setopt(h, CURLOPT_HTTPHEADER, hs);
    curl_easy_setopt(h, CURLOPT_POSTFIELDS, body.c_str());
    curl_easy_setopt(h, CURLOPT_POSTFIELDSIZE, (long)body.size());
    curl_easy_setopt(h, CURLOPT_WRITEFUNCTION, writeCb);
    curl_easy_setopt(h, CURLOPT_WRITEDATA, &out);
    curl_easy_setopt(h, CURLOPT_TIMEOUT, timeoutSec);
    curl_easy_setopt(h, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(h, CURLOPT_USERAGENT, "PIBOT-Gameboost/2.0");

    CURLcode rc = curl_easy_perform(h);
    curl_slist_free_all(hs);
    curl_easy_cleanup(h);
    if (rc != CURLE_OK) return "";
    return out;
}

#else // بديل: استدعاء أمر curl (موجود دائماً في Termux)

#include <cstdio>
#include <fstream>
#include <unistd.h>

std::string httpPostJson(const std::string& url,
                         const std::vector<std::string>& headers,
                         const std::string& body, long timeoutSec) {
    // كتابة الجسم إلى ملف مؤقت لتفادي مشاكل التهريب
    std::string tmpl = cfgDir() + "/.httpXXXXXX";
    std::vector<char> path(tmpl.begin(), tmpl.end());
    path.push_back('\0');
    int fd = mkstemp(path.data());
    if (fd < 0) return "";
    {
        std::ofstream f(path.data(), std::ios::binary);
        f << body;
    }
    close(fd);

    std::string cmd = "curl -sS --max-time " + std::to_string(timeoutSec) +
                      " -H 'Content-Type: application/json'";
    for (auto& h : headers) cmd += " -H " + shellQuote(h);
    cmd += " --data-binary @" + shellQuote(path.data());
    cmd += " " + shellQuote(url);

    std::string out = execCapture(cmd);
    unlink(path.data());
    return out;
}

#endif
