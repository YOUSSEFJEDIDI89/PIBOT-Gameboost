#include "game.hpp"
#include "priv.hpp"
#include "util.hpp"

#include <csignal>
#include <iostream>
#include <regex>
#include <sstream>
#include <unistd.h>
#include <vector>

using namespace ui;

static int pv(const std::string& c) {
    int rc;
    runPriv(c, &rc);
    return rc;
}

// التطبيق النشط حالياً
static std::string focusedPkg() {
    std::string out = runPriv("dumpsys window 2>/dev/null | grep -m1 -E 'mCurrentFocus|mFocusedApp'");
    std::smatch m;
    static const std::regex re("([A-Za-z0-9_.]+)/[A-Za-z0-9_.]+");
    if (std::regex_search(out, m, re)) return m[1];
    return "";
}

static std::string pickGamePkg() {
    std::cout << "  " << C << "[1]" << N << " كتابة اسم الحزمة يدوياً (مثال: com.tencent.ig)\n"
              << "  " << C << "[2]" << N << " اختيار من الألعاب المثبتة الشائعة\n";
    std::string m = ask("اختر ⇢ ");
    if (m == "2") {
        info("جاري البحث عن ألعاب مثبتة...");
        std::string out = runPriv(
            "pm list packages 2>/dev/null | sed 's/package://' | grep -Ei "
            "'tencent|pubg|garena|freefire|dts.freefire|activision|callofduty|mobile.legends|"
            "mobilelegends|supercell|clash|roblox|minecraft|mojang|efootball|konami|ea.gp|fifa|"
            "genshin|miHoYo|hoyoverse|riotgames|wildrift|carxtech|outfit7|gameloft|nexon|netease' | head -20");
        std::vector<std::string> games;
        std::istringstream ss(out);
        std::string line;
        while (std::getline(ss, line)) {
            line = trim(line);
            if (!line.empty()) games.push_back(line);
        }
        if (games.empty()) {
            warn("لم أجد ألعاباً معروفة، اكتب الحزمة يدوياً.");
        } else {
            for (size_t i = 0; i < games.size(); ++i)
                std::cout << "   " << Y << "[" << (i + 1) << "]" << N << " " << games[i] << "\n";
            std::string gi = ask("رقم اللعبة ⇢ ");
            try {
                size_t idx = std::stoul(gi);
                if (idx >= 1 && idx <= games.size()) return games[idx - 1];
            } catch (...) {}
        }
    }
    return ask("اسم الحزمة ⇢ ");
}

// ---------------- تعزيز لعبة ----------------
void gameBoostMenu() {
    banner();
    std::cout << "  " << M << "🎮 تعزيز لعبة معيّنة" << N << "\n\n";
    if (!needPrivOrExplain()) return;

    std::string pkg = pickGamePkg();
    if (pkg.empty()) { err("لم يتم اختيار لعبة"); return; }

    std::string check = runPriv("pm list packages 2>/dev/null | grep -x 'package:" + pkg + "'");
    if (trim(check).empty()) warn("لم أتأكد من وجود " + pkg + " — سأكمل على أي حال.");

    std::cout << "\n";
    info("اللعبة المستهدفة: " + std::string(G) + pkg + N);
    std::cout << "\n";

    step("تفعيل Game Mode: Performance (أندرويد 12+)",
         [&] { return pv("cmd game mode performance " + pkg); });
    step("طلب فتح حد FPS من النظام",
         [&] { return pv("device_config put game_overlay " + pkg + " 'mode=2,fps=120:mode=3,fps=120'"); });
    step("إيقاف تحسين البطارية لهذه اللعبة (منع الخنق)",
         [&] { return pv("dumpsys deviceidle whitelist +" + pkg); });
    step("قتل الخلفية لتفريغ الرام", [] { return pv("am kill-all"); });

    std::cout << "\n";
    if (confirm("هل تريد إعادة ترجمة اللعبة لأقصى أداء؟ يستغرق دقائق")) {
        info("جاري الترجمة الكاملة (pm compile speed)... اصبر، هذا يحسّن الأداء فعلياً");
        int rc = pv("pm compile -m speed -f " + pkg);
        if (rc == 0) ok("تمت الترجمة بنجاح!");
        else warn("لم تكتمل الترجمة");
    }
    std::cout << "\n";
    ok("تم تعزيز " + std::string(G) + pkg + N + " — شغّل اللعبة الآن وقس النتيجة من الخيار [4]");
}

// ---------------- قياس FPS الحقيقي ----------------
static volatile sig_atomic_t g_stop = 0;
static void onInt(int) { g_stop = 1; }

static long grepFirstNum(const std::string& text, const std::string& key) {
    size_t pos = text.find(key);
    if (pos == std::string::npos) return -1;
    pos += key.size();
    while (pos < text.size() && !isdigit((unsigned char)text[pos])) ++pos;
    long v = 0;
    bool any = false;
    while (pos < text.size() && isdigit((unsigned char)text[pos])) {
        v = v * 10 + (text[pos] - '0');
        ++pos;
        any = true;
    }
    return any ? v : -1;
}

void fpsMeter() {
    banner();
    std::cout << "  " << M << "📊 قياس FPS الحقيقي" << N << "\n\n";
    if (!needPrivOrExplain()) return;

    std::string rr = trim(runPriv(
        "dumpsys display 2>/dev/null | grep -m1 -oE 'renderFrameRate[= ]+[0-9.]+' | grep -oE '[0-9.]+' | head -1"));
    if (!rr.empty())
        info("تردد الشاشة الحالي: " + std::string(G) + rr.substr(0, rr.find('.')) + " Hz" + N +
             " (هذا هو سقف الـFPS)");

    std::string pkg = focusedPkg();
    if (!pkg.empty() && pkg != "com.termux") {
        info("التطبيق النشط: " + pkg);
        if (!confirm("قياس هذا التطبيق؟"))
            pkg = ask("اسم الحزمة ⇢ ");
    } else {
        pkg = ask("اسم حزمة اللعبة (شغّلها بنافذة منقسمة) ⇢ ");
    }
    if (pkg.empty()) { err("لا توجد حزمة"); return; }

    std::cout << "\n";
    info("القياس المباشر: قراءة كل 5 ثوانٍ — Ctrl+C للإيقاف");
    std::cout << "\n";

    g_stop = 0;
    struct sigaction sa{}, old{};
    sa.sa_handler = onInt;
    sigaction(SIGINT, &sa, &old);

    while (!g_stop) {
        runPriv("dumpsys gfxinfo " + pkg + " reset");
        for (int i = 0; i < 5 && !g_stop; ++i) sleep(1);
        if (g_stop) break;

        std::string out = runPriv("dumpsys gfxinfo " + pkg);
        long total = grepFirstNum(out, "Total frames rendered");
        long janky = grepFirstNum(out, "Janky frames");
        if (total <= 0) {
            warn("اللعبة لا ترسم إطارات الآن (تأكد أنها مفتوحة على الشاشة)");
        } else {
            long fps = total / 5;
            long jp = (janky > 0) ? janky * 100 / total : 0;
            const char* col = G;
            if (fps < 45) col = Y;
            if (fps < 25) col = R;
            std::cout << "  🎯 FPS: " << col << fps << N << "  |  إطارات متقطعة: " << jp
                      << "%  |  إجمالي: " << total << " إطار/5ث\n";
        }
    }
    sigaction(SIGINT, &old, nullptr);
    std::cout << "\n";
    info("تم إيقاف القياس.");
}

// ---------------- رفع تردد الشاشة ----------------
void refreshRateMenu() {
    banner();
    std::cout << "  " << M << "🖥️  رفع تردد الشاشة (أكبر زيادة حقيقية للـFPS)" << N << "\n\n";
    if (!needPrivOrExplain()) return;

    info("الترددات المدعومة في شاشتك:");
    std::string modes = runPriv(
        "dumpsys display 2>/dev/null | grep -oE 'fps=[0-9]+' | grep -oE '[0-9]+' | sort -run | head -6");
    if (!trim(modes).empty()) {
        std::istringstream ss(modes);
        std::string m;
        while (std::getline(ss, m)) {
            m = trim(m);
            if (!m.empty()) std::cout << "     " << G << "●" << N << " " << m << " Hz\n";
        }
    } else {
        warn("لم أستطع قراءة الأوضاع — شاشتك غالباً 60/90/120");
    }
    std::cout << "\n";

    std::string hz = ask("أدخل التردد المطلوب تثبيته (مثال 90 أو 120) ⇢ ");
    if (hz.empty() || hz.find_first_not_of("0123456789") != std::string::npos) {
        err("قيمة غير صحيحة");
        return;
    }

    pv("settings put system peak_refresh_rate " + hz);
    pv("settings put system min_refresh_rate " + hz);
    pv("settings put secure refresh_rate_mode 2");

    std::string now = trim(runPriv(
        "dumpsys display 2>/dev/null | grep -m1 -oE 'renderFrameRate[= ]+[0-9.]+' | grep -oE '[0-9.]+' | head -1"));
    std::cout << "\n";
    if (!now.empty())
        ok("تم! تردد الشاشة الآن: " + std::string(G) + now.substr(0, now.find('.')) + " Hz" + N +
           " (كان النظام يخفضه تلقائياً)");
    else
        ok("تم إرسال الأوامر. تحقق من الإعدادات ← الشاشة.");
    warn("بعض الألعاب تقفل نفسها على 60 من الداخل — ارفع الإعداد من داخل اللعبة أيضاً.");
    info("تثبيت التردد الأعلى يستهلك بطارية أكثر — للإرجاع استخدم الخيار [8].");
}
