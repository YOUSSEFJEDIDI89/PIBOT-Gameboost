#!/data/data/com.termux/files/usr/bin/bash
# ============================================================
#  PIBOT Gameboost - مسرّع ألعاب حقيقي لتيرمكس (بدون روت)
#  + بوت ذكاء اصطناعي (Gemini / Claude / Custom API / Free)
# ============================================================

PIBOT_DIR="$(cd "$(dirname "$(readlink -f "$0")")" && pwd)"
export PIBOT_DIR
export PIBOT_LIB="$PIBOT_DIR/lib"
export PIBOT_CFG_DIR="$HOME/.pibot"
mkdir -p "$PIBOT_CFG_DIR"

source "$PIBOT_LIB/ui.sh"
source "$PIBOT_LIB/priv.sh"
source "$PIBOT_LIB/boost.sh"
source "$PIBOT_LIB/game.sh"
source "$PIBOT_LIB/bot.sh"

main_menu() {
    while true; do
        banner
        priv_status_line
        echo
        echo -e "  ${C}[1]${N} ⚡ تسريع سريع (بدون روت - تيرمكس فقط)"
        echo -e "  ${C}[2]${N} 🚀 تسريع عميق (ADB / Shizuku - تأثير حقيقي)"
        echo -e "  ${C}[3]${N} 🎮 تعزيز لعبة معيّنة (Game Mode + رفع FPS)"
        echo -e "  ${C}[4]${N} 📊 قياس FPS الحقيقي + حالة الشاشة"
        echo -e "  ${C}[5]${N} 🖥️  رفع تردد الشاشة (60 → 90/120Hz)"
        echo -e "  ${C}[6]${N} 🤖 بوت PIBOT الذكي (AI حقيقي)"
        echo -e "  ${C}[7]${N} 🔌 إعداد ADB اللاسلكي / Shizuku"
        echo -e "  ${C}[8]${N} ♻️  استرجاع الإعدادات الافتراضية"
        echo -e "  ${C}[9]${N} ℹ️  معلومات الجهاز"
        echo -e "  ${C}[0]${N} 🚪 خروج"
        echo
        read -r -p "  اختر رقماً ⇢ " ch
        case "$ch" in
            1) quick_boost ;;
            2) deep_boost ;;
            3) game_boost_menu ;;
            4) fps_meter ;;
            5) refresh_rate_menu ;;
            6) bot_main ;;
            7) adb_setup_menu ;;
            8) restore_defaults ;;
            9) device_info ;;
            0) echo -e "\n  ${G}إلى اللقاء! العب براحتك 🎮${N}\n"; exit 0 ;;
            *) warn "اختيار غير صحيح" ;;
        esac
        pause
    done
}

case "${1:-}" in
    bot)   bot_main; exit 0 ;;
    boost) quick_boost; exit 0 ;;
    deep)  deep_boost; exit 0 ;;
    fps)   fps_meter; exit 0 ;;
    *)     main_menu ;;
esac
