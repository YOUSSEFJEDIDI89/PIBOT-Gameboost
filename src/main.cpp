// ============================================================
//  PIBOT Gameboost v2.0 (C++) - مسرّع ألعاب حقيقي لتيرمكس بدون روت
//  + بوت ذكاء اصطناعي (Gemini / Claude / Custom API / Free)
// ============================================================
#include "boost.hpp"
#include "bot.hpp"
#include "game.hpp"
#include "priv.hpp"
#include "util.hpp"

#include <cstring>
#include <iostream>

using namespace ui;

static void mainMenu() {
    while (true) {
        banner();
        privStatusLine();
        std::cout << "\n"
            << "  " << C << "[1]" << N << " ⚡ تسريع سريع (بدون روت - تيرمكس فقط)\n"
            << "  " << C << "[2]" << N << " 🚀 تسريع عميق (ADB / Shizuku - تأثير حقيقي)\n"
            << "  " << C << "[3]" << N << " 🎮 تعزيز لعبة معيّنة (Game Mode + رفع FPS)\n"
            << "  " << C << "[4]" << N << " 📊 قياس FPS الحقيقي + حالة الشاشة\n"
            << "  " << C << "[5]" << N << " 🖥️  رفع تردد الشاشة (60 → 90/120Hz)\n"
            << "  " << C << "[6]" << N << " 🤖 بوت PIBOT الذكي (AI حقيقي)\n"
            << "  " << C << "[7]" << N << " 🔌 إعداد ADB اللاسلكي / Shizuku\n"
            << "  " << C << "[8]" << N << " ♻️  استرجاع الإعدادات الافتراضية\n"
            << "  " << C << "[9]" << N << " ℹ️  معلومات الجهاز\n"
            << "  " << C << "[0]" << N << " 🚪 خروج\n\n";

        std::string ch = ask("اختر رقماً ⇢ ");
        if      (ch == "1") quickBoost();
        else if (ch == "2") deepBoost();
        else if (ch == "3") gameBoostMenu();
        else if (ch == "4") fpsMeter();
        else if (ch == "5") refreshRateMenu();
        else if (ch == "6") botMain();
        else if (ch == "7") adbSetupMenu();
        else if (ch == "8") restoreDefaults();
        else if (ch == "9") deviceInfo();
        else if (ch == "0" || std::cin.eof()) {
            std::cout << "\n  " << G << "إلى اللقاء! العب براحتك 🎮" << N << "\n\n";
            return;
        }
        else warn("اختيار غير صحيح");
        if (std::cin.eof()) return;
        pause();
    }
}

int main(int argc, char** argv) {
    if (argc > 1) {
        std::string a = argv[1];
        if (a == "bot")   { botMain();        return 0; }
        if (a == "boost") { quickBoost();     return 0; }
        if (a == "deep")  { deepBoost();      return 0; }
        if (a == "fps")   { fpsMeter();       return 0; }
        if (a == "--help" || a == "-h") {
            std::cout << "pibot [bot|boost|deep|fps]\n";
            return 0;
        }
    }
    mainMenu();
    return 0;
}
