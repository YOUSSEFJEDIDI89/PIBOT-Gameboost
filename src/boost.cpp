#include "boost.hpp"
#include "json.hpp"
#include "priv.hpp"
#include "util.hpp"

#include <cstdlib>
#include <iostream>
#include <unistd.h>

using namespace ui;

static int sh(const std::string& c) {
    int rc;
    execCapture(c, &rc);
    return rc;
}
static int pv(const std::string& c) {
    int rc;
    runPriv(c, &rc);
    return rc;
}

// ---------------- تسريع سريع (تيرمكس فقط) ----------------
void quickBoost() {
    banner();
    std::cout << "  " << M << "⚡ التسريع السريع (بدون روت)" << N << "\n\n";
    long before = ramFreeMB();

    step("تنظيف ملفات تيرمكس المؤقتة", [] {
        const char* p = getenv("PREFIX");
        std::string pre = p ? p : "";
        if (!pre.empty()) sh("rm -rf " + shellQuote(pre + "/tmp") + "/* 2>/dev/null");
        sh("rm -rf " + shellQuote(homeDir() + "/.cache") + "/* 2>/dev/null");
        return 0;
    });
    step("مزامنة الذاكرة", [] { return sh("sync"); });
    step("تثبيت أولوية تيرمكس (منع الخنق أثناء اللعب)", [] { return sh("termux-wake-lock"); });
    step("محاولة قتل عمليات الخلفية", [] { return sh("am kill-all"); });

    if (hasPriv()) {
        step("تنظيف كاش كل التطبيقات (حقيقي)", [] { return pv("pm trim-caches 512G"); });
        step("قتل كل عمليات الخلفية (حقيقي)", [] { return pv("am kill-all"); });
    }

    sleep(1);
    long after = ramFreeMB();
    std::cout << "\n";
    ok("الرام المتاحة قبل: " + std::string(Y) + std::to_string(before) + " MB" + N +
       " ← بعد: " + G + std::to_string(after) + " MB" + N);
    if (!hasPriv())
        warn("بدون ADB/Shizuku التنظيف محدود. فعّل الخيار [7] لنتيجة أقوى بكثير.");
    info("نصيحة: أغلق التطبيقات قبل اللعب، وفعّل وضع الطيران إذا كانت اللعبة أوفلاين.");
}

// ---------------- تسريع عميق ----------------
void deepBoost() {
    banner();
    std::cout << "  " << M << "🚀 التسريع العميق (تأثير حقيقي على مستوى النظام)" << N << "\n\n";
    if (!needPrivOrExplain()) return;

    long before = ramFreeMB();

    step("تنظيف كاش جميع التطبيقات", [] { return pv("pm trim-caches 999G"); });
    step("قتل جميع عمليات الخلفية", [] { return pv("am kill-all"); });
    step("تسريع الأنيميشن x0.5 (استجابة أسرع)", [] {
        pv("settings put global window_animation_scale 0.5");
        pv("settings put global transition_animation_scale 0.5");
        return pv("settings put global animator_duration_scale 0.5");
    });
    step("تفعيل وضع الأداء الثابت (منع خفض التردد)", [] {
        return pv("cmd power set-fixed-performance-mode-enabled true");
    });
    step("تفعيل تجميد تطبيقات الخلفية", [] {
        return pv("settings put global cached_apps_freezer enabled");
    });
    step("إيقاف مسح الواي فاي الخلفي (يقلل اللاق)", [] {
        pv("settings put global wifi_scan_always_enabled 0");
        return pv("settings put global ble_scan_always_enabled 0");
    });

    sleep(1);
    long after = ramFreeMB();
    std::cout << "\n";
    ok("اكتمل التسريع العميق! الرام: " + std::string(Y) + std::to_string(before) + " MB" + N +
       " ← " + G + std::to_string(after) + " MB" + N);
    info("لأفضل نتيجة: شغّل أيضاً [5] لرفع تردد الشاشة، و[3] لتعزيز لعبتك.");
    warn("لإرجاع كل شيء كما كان: الخيار [8].");
}

// ---------------- استرجاع الإعدادات ----------------
void restoreDefaults() {
    banner();
    std::cout << "  " << M << "♻️  استرجاع الإعدادات الافتراضية" << N << "\n\n";
    if (!needPrivOrExplain()) return;

    step("إرجاع سرعة الأنيميشن 1.0", [] {
        pv("settings put global window_animation_scale 1.0");
        pv("settings put global transition_animation_scale 1.0");
        return pv("settings put global animator_duration_scale 1.0");
    });
    step("إيقاف وضع الأداء الثابت", [] {
        return pv("cmd power set-fixed-performance-mode-enabled false");
    });
    step("إرجاع تردد الشاشة للتلقائي", [] {
        pv("settings delete system min_refresh_rate");
        return pv("settings delete system peak_refresh_rate");
    });
    step("إرجاع مسح الواي فاي", [] {
        return pv("settings put global wifi_scan_always_enabled 1");
    });
    sh("termux-wake-unlock");
    ok("تم إرجاع كل شيء للوضع الافتراضي.");
}

// ---------------- معلومات الجهاز ----------------
void deviceInfo() {
    banner();
    std::cout << "  " << M << "ℹ️  معلومات الجهاز" << N << "\n\n";
    std::cout << "  " << C << "الجهاز:" << N << "        " << getProp("ro.product.brand")
              << " " << getProp("ro.product.model") << "\n";
    std::cout << "  " << C << "أندرويد:" << N << "       " << getProp("ro.build.version.release")
              << " (SDK " << getProp("ro.build.version.sdk") << ")\n";
    std::cout << "  " << C << "المعالج:" << N << "       " << getProp("ro.board.platform")
              << " - " << trim(execCapture("nproc")) << " أنوية\n";
    std::cout << "  " << C << "الرام الكلية:" << N << "  "
              << trim(execCapture("awk '/MemTotal/ {printf \"%d MB\", $2/1024}' /proc/meminfo")) << "\n";
    std::cout << "  " << C << "الرام المتاحة:" << N << " " << ramFreeMB() << " MB\n";

    if (cmdExists("termux-battery-status")) {
        std::string bat = execCapture("timeout 5 termux-battery-status");
        auto j = mj::parse(bat);
        if (j && j->get("percentage")) {
            long pct = (long)j->get("percentage")->num;
            long temp = j->get("temperature") ? (long)j->get("temperature")->num : 0;
            std::cout << "  " << C << "البطارية:" << N << "      " << pct << "% - حرارة "
                      << temp << "°C\n";
        }
    }
    if (hasPriv()) {
        std::string rr = trim(runPriv(
            "dumpsys display 2>/dev/null | grep -m1 -oE 'renderFrameRate[= ]+[0-9.]+' | grep -oE '[0-9.]+' | head -1"));
        if (!rr.empty()) {
            auto dot = rr.find('.');
            std::cout << "  " << C << "تردد الشاشة الحالي:" << N << " " << G
                      << rr.substr(0, dot) << " Hz" << N << "\n";
        }
    }
}
