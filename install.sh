#!/data/data/com.termux/files/usr/bin/bash
# مثبّت PIBOT Gameboost لتيرمكس

set -e
DIR="$(cd "$(dirname "$0")" && pwd)"

echo -e "\033[1;35m"
echo "  ╔══════════════════════════════════════╗"
echo "  ║   تثبيت PIBOT Gameboost ⚡           ║"
echo "  ╚══════════════════════════════════════╝"
echo -e "\033[0m"

if [ -z "$PREFIX" ] || ! command -v pkg >/dev/null 2>&1; then
    echo "  [!] يبدو أنك لست داخل Termux — سأكمل التثبيت المحلي فقط."
    PREFIX="${PREFIX:-$HOME/.local}"
    mkdir -p "$PREFIX/bin"
else
    echo "  [i] تحديث الحزم وتثبيت المتطلبات..."
    pkg update -y >/dev/null 2>&1 || true
    pkg install -y curl jq ncurses-utils termux-api >/dev/null 2>&1 || pkg install -y curl jq
    echo "  [?] هل تريد تثبيت android-tools (ADB) للحصول على التأثير الكامل بدون روت؟ (موصى به)"
    read -r -p "      (y/n) ⇢ " adb_yn
    if [ "$adb_yn" = "y" ] || [ "$adb_yn" = "Y" ]; then
        pkg install -y android-tools >/dev/null 2>&1 && echo "  [✔] تم تثبيت ADB" || echo "  [!] تعذر تثبيت ADB"
    fi
fi

chmod +x "$DIR/pibot.sh" "$DIR/lib/"*.sh

# إنشاء أمر pibot
cat > "$PREFIX/bin/pibot" <<EOF
#!/usr/bin/env bash
exec bash "$DIR/pibot.sh" "\$@"
EOF
chmod +x "$PREFIX/bin/pibot"

echo
echo -e "  \033[1;32m[✔] تم التثبيت بنجاح!\033[0m"
echo
echo "  التشغيل:"
echo "    pibot          ← القائمة الرئيسية"
echo "    pibot bot      ← البوت الذكي مباشرة"
echo "    pibot boost    ← تسريع سريع"
echo "    pibot deep     ← تسريع عميق"
echo "    pibot fps      ← قياس FPS"
echo
