#include "util.hpp"

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <sys/stat.h>

std::string execCapture(const std::string& cmd, int* rc) {
    std::string out;
    FILE* fp = popen((cmd + " 2>/dev/null").c_str(), "r");
    if (!fp) {
        if (rc) *rc = -1;
        return out;
    }
    char buf[4096];
    size_t n;
    while ((n = fread(buf, 1, sizeof buf, fp)) > 0) out.append(buf, n);
    int st = pclose(fp);
    if (rc) *rc = (st == -1) ? -1 : WEXITSTATUS(st);
    return out;
}

bool cmdExists(const std::string& c) {
    int rc = 1;
    execCapture("command -v " + shellQuote(c), &rc);
    return rc == 0;
}

std::string shellQuote(const std::string& s) {
    std::string o = "'";
    for (char c : s) {
        if (c == '\'') o += "'\\''";
        else o += c;
    }
    o += "'";
    return o;
}

std::string trim(const std::string& s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a == std::string::npos) return "";
    size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}

std::string homeDir() {
    const char* h = getenv("HOME");
    return h ? h : "/data/data/com.termux/files/home";
}

std::string cfgDir() {
    std::string d = homeDir() + "/.pibot";
    mkdir(d.c_str(), 0700);
    return d;
}

long ramFreeMB() {
    std::ifstream f("/proc/meminfo");
    std::string k;
    long v;
    std::string unit;
    while (f >> k >> v >> unit) {
        if (k == "MemAvailable:") return v / 1024;
    }
    return -1;
}

std::string getProp(const std::string& key) {
    return trim(execCapture("getprop " + shellQuote(key)));
}

namespace ui {

const char* R = "\033[1;31m";
const char* G = "\033[1;32m";
const char* Y = "\033[1;33m";
const char* B = "\033[1;34m";
const char* M = "\033[1;35m";
const char* C = "\033[1;36m";
const char* N = "\033[0m";

void banner() {
    std::cout << "\033[2J\033[H";
    std::cout << M <<
        "   ██████╗ ██╗██████╗  ██████╗ ████████╗\n"
        "   ██╔══██╗██║██╔══██╗██╔═══██╗╚══██╔══╝\n"
        "   ██████╔╝██║██████╔╝██║   ██║   ██║\n"
        "   ██╔═══╝ ██║██╔══██╗██║   ██║   ██║\n"
        "   ██║     ██║██████╔╝╚██████╔╝   ██║\n"
        "   ╚═╝     ╚═╝╚═════╝  ╚═════╝    ╚═╝\n";
    std::cout << C << "        G A M E B O O S T  ⚡  v2.0 (C++)\n" << N;
    std::cout << Y << "   مسرّع ألعاب حقيقي بدون روت + بوت ذكاء اصطناعي\n" << N;
    std::cout << B << "  ──────────────────────────────────────────────\n" << N;
}

void ok(const std::string& s)   { std::cout << "  " << G << "[✔]" << N << " " << s << "\n"; }
void warn(const std::string& s) { std::cout << "  " << Y << "[!]" << N << " " << s << "\n"; }
void err(const std::string& s)  { std::cout << "  " << R << "[✘]" << N << " " << s << "\n"; }
void info(const std::string& s) { std::cout << "  " << C << "[i]" << N << " " << s << "\n"; }

void pause() {
    std::cout << "\n  اضغط Enter للمتابعة...";
    std::cout.flush();
    std::string _;
    std::getline(std::cin, _);
}

std::string ask(const std::string& prompt) {
    std::cout << "  " << prompt;
    std::cout.flush();
    std::string s;
    if (!std::getline(std::cin, s)) return "";
    return trim(s);
}

bool confirm(const std::string& prompt) {
    std::string a = ask(prompt + " (y/n) ⇢ ");
    return a == "y" || a == "Y" || a == "نعم";
}

void step(const std::string& msg, const std::function<int()>& fn) {
    std::cout << "  " << C << "[…]" << N << " " << msg << std::flush;
    int rc = fn();
    if (rc == 0)
        std::cout << "\r  " << G << "[✔]" << N << " " << msg << "        \n";
    else
        std::cout << "\r  " << Y << "[~]" << N << " " << msg << " (تخطّي)\n";
}

} // namespace ui
