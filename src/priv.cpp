#include "priv.hpp"
#include "util.hpp"

#include <iostream>

static PrivMode g_mode = PrivMode::None;
static bool g_detected = false;

PrivMode detectPriv(bool refresh) {
    if (g_detected && !refresh) return g_mode;
    g_mode = PrivMode::None;
    if (cmdExists("rish")) {
        std::string id = execCapture("rish -c 'id' ");
        if (id.find("uid=2000") != std::string::npos) g_mode = PrivMode::Shizuku;
    }
    if (g_mode == PrivMode::None && cmdExists("adb")) {
        std::string st = execCapture("adb get-state");
        if (st.find("device") != std::string::npos) g_mode = PrivMode::Adb;
    }
    if (g_mode == PrivMode::None && cmdExists("su")) {
        std::string id = execCapture("su -c id");
        if (id.find("uid=0") != std::string::npos) g_mode = PrivMode::Root;
    }
    g_detected = true;
    return g_mode;
}

std::string runPriv(const std::string& cmd, int* rc) {
    switch (detectPriv()) {
        case PrivMode::Shizuku: return execCapture("rish -c " + shellQuote(cmd), rc);
        case PrivMode::Adb:     return execCapture("adb shell " + shellQuote(cmd), rc);
        case PrivMode::Root:    return execCapture("su -c " + shellQuote(cmd), rc);
        default:
            if (rc) *rc = 127;
            return "";
    }
}

bool hasPriv() { return detectPriv() != PrivMode::None; }

void privStatusLine() {
    using namespace ui;
    switch (detectPriv(true)) {
        case PrivMode::Shizuku:
            std::cout << "  " << G << "● صلاحيات: Shizuku متصل (تأثير كامل بدون روت)" << N << "\n";
            break;
        case PrivMode::Adb:
            std::cout << "  " << G << "● صلاحيات: ADB لاسلكي متصل (تأثير كامل بدون روت)" << N << "\n";
            break;
        case PrivMode::Root:
            std::cout << "  " << G << "● صلاحيات: روت (تأثير كامل)" << N << "\n";
            break;
        default:
            std::cout << "  " << Y << "● صلاحيات: تيرمكس فقط — فعّل ADB من الخيار [7] لتأثير أقوى" << N << "\n";
    }
}

bool needPrivOrExplain() {
    if (hasPriv()) return true;
    ui::err("هذه الميزة تحتاج ADB لاسلكي أو Shizuku (بدون روت).");
    ui::info("افتح الخيار [7] من القائمة وسأرشدك خطوة بخطوة (دقيقتان فقط).");
    return false;
}

// ---------------- معالج إعداد ADB اللاسلكي ----------------
static void adbWirelessWizard() {
    using namespace ui;
    if (!cmdExists("adb")) {
        step("تثبيت android-tools (adb)", [] {
            int rc;
            execCapture("pkg install -y android-tools", &rc);
            return rc;
        });
    }
    std::cout << "\n";
    info("الخطوات (لا تغلق تيرمكس، استخدم تقسيم الشاشة أو النافذة العائمة):");
    std::cout << "   " << Y << "1." << N << " الإعدادات ← خيارات المطور ← " << C << "التصحيح اللاسلكي" << N << " ← تفعيل\n";
    std::cout << "   " << Y << "2." << N << " اضغط " << C << "إقران الجهاز برمز إقران" << N << "\n";
    std::cout << "   " << Y << "3." << N << " سيظهر: عنوان IP:منفذ + رمز إقران من 6 أرقام\n\n";

    std::string pairAddr = ask("أدخل عنوان الإقران (مثال 192.168.1.5:37123) ⇢ ");
    std::string pairCode = ask("أدخل رمز الإقران (6 أرقام) ⇢ ");
    if (pairAddr.empty()) { err("لم تدخل العنوان"); return; }

    info("جاري الإقران...");
    std::string out = execCapture("adb pair " + shellQuote(pairAddr) + " " + shellQuote(pairCode));
    if (out.find("uccess") == std::string::npos) {
        err("فشل الإقران، تأكد من الرمز والعنوان وأعد المحاولة.");
        return;
    }
    ok("تم الإقران بنجاح!");

    std::cout << "\n";
    info("الآن ارجع لشاشة (التصحيح اللاسلكي) وانظر عنوان IP والمنفذ الرئيسي (مختلف عن منفذ الإقران)");
    std::string connAddr = ask("أدخل عنوان الاتصال (مثال 192.168.1.5:40567) ⇢ ");
    out = execCapture("adb connect " + shellQuote(connAddr));
    if (out.find("connected") != std::string::npos) {
        ok("تم الاتصال! أصبح لديك صلاحيات shell حقيقية بدون روت 🎉");
        detectPriv(true);
    } else {
        err("فشل الاتصال، تحقق من المنفذ.");
    }
}

static void shizukuWizard() {
    using namespace ui;
    std::cout << "\n";
    info("لتشغيل Shizuku بدون روت: افتح تطبيق Shizuku ← شغّله عبر (التصحيح اللاسلكي)");
    info("ثم من التطبيق: (استخدام Shizuku في تطبيقات الطرفية) ← صدّر ملفات rish إلى Termux");
    std::cout << "\n";
    if (cmdExists("rish")) {
        std::string id = execCapture("rish -c 'id'");
        if (id.find("uid=") != std::string::npos)
            ok("rish يعمل! Shizuku متصل.");
        else
            err("rish موجود لكن Shizuku غير مشغّل. شغّله من التطبيق أولاً.");
    } else {
        warn("ملف rish غير موجود في المسار. ضعه في $PREFIX/bin/rish مع rish_shizuku.dex");
    }
}

void adbSetupMenu() {
    using namespace ui;
    banner();
    std::cout << "  " << M << "🔌 إعداد الصلاحيات بدون روت" << N << "\n\n";
    privStatusLine();
    std::cout << "\n"
              << "  " << C << "[1]" << N << " إعداد ADB اللاسلكي (أندرويد 11+ ، بدون كمبيوتر)\n"
              << "  " << C << "[2]" << N << " ربط Shizuku (إن كان مثبتاً)\n"
              << "  " << C << "[3]" << N << " فحص الاتصال\n"
              << "  " << C << "[0]" << N << " رجوع\n\n";
    std::string c = ask("اختر ⇢ ");
    if (c == "1") adbWirelessWizard();
    else if (c == "2") shizukuWizard();
    else if (c == "3") {
        detectPriv(true);
        privStatusLine();
        if (hasPriv()) std::cout << "  " << trim(runPriv("getprop ro.product.model")) << "\n";
    }
}
