// util.hpp - أدوات مساعدة + واجهة ملونة
#pragma once
#include <functional>
#include <string>
#include <vector>

// تنفيذ أمر والتقاط مخرجاته
std::string execCapture(const std::string& cmd, int* rc = nullptr);
bool cmdExists(const std::string& c);
std::string shellQuote(const std::string& s);
std::string trim(const std::string& s);
std::string homeDir();
std::string cfgDir();      // ~/.pibot (يُنشأ تلقائياً)
long ramFreeMB();
std::string getProp(const std::string& key);

namespace ui {
// ألوان ANSI
extern const char *R, *G, *Y, *B, *M, *C, *N;

void banner();
void ok(const std::string& s);
void warn(const std::string& s);
void err(const std::string& s);
void info(const std::string& s);
void pause();
std::string ask(const std::string& prompt);
bool confirm(const std::string& prompt);
// تنفيذ خطوة مع مؤشر ورسالة نجاح/تخطي
void step(const std::string& msg, const std::function<int()>& fn);
} // namespace ui
