// priv.hpp - طبقة الصلاحيات: Shizuku / ADB لاسلكي / روت (بدون روت في الغالب)
#pragma once
#include <string>

enum class PrivMode { None, Shizuku, Adb, Root };

PrivMode detectPriv(bool refresh = false);
// تنفيذ أمر بصلاحيات shell وإرجاع مخرجاته
std::string runPriv(const std::string& cmd, int* rc = nullptr);
bool hasPriv();
void privStatusLine();
bool needPrivOrExplain();
void adbSetupMenu();
