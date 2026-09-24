#!/data/data/com.termux/files/usr/bin/bash
# مثبّت PIBOT Gameboost — يبني نسخة C++ الأصلية (native) تلقائياً

set -e
DIR="$(cd "$(dirname "$0")" && pwd)"

echo -e "\033[1;35m"
echo "  ╔══════════════════════════════════════╗"
echo "  ║   تثبيت PIBOT Gameboost v2.0 (C++) ⚡ ║"
echo "  ╚══════════════════════════════════════╝"
echo -e "\033[0m"

IS_TERMUX=0
if [ -n "$PREFIX" ] && command -v pkg >/dev/null 2>&1; then
    IS_TERMUX=1
else
    PREFIX="${PREFIX:-$HOME/.local}"
    mkdir -p "$PREFIX/bin"
fi

if [ "$IS_TERMUX" = "1" ]; then
    echo "  [i] تثبيت المتطلبات (clang, cmake, libcurl)..."
    pkg update -y >/dev/null 2>&1 || true
    pkg install -y clang cmake make libcurl curl termux-api >/dev/null 2>&1 \
        || pkg install -y clang make curl
    echo "  [?] تثبيت android-tools (ADB) للتأثير الكامل بدون روت؟ (موصى به)"
    read -r -p "      (y/n) ⇢ " adb_yn
    if [ "$adb_yn" = "y" ] || [ "$adb_yn" = "Y" ]; then
        pkg install -y android-tools >/dev/null 2>&1 && echo "  [✔] تم تثبيت ADB" || echo "  [!] تعذر تثبيت ADB"
    fi
fi

# ---------- بناء النسخة الأصلية C++ ----------
BUILT=0
cd "$DIR"
if command -v cmake >/dev/null 2>&1; then
    echo "  [i] البناء عبر CMake..."
    cmake -B build -DCMAKE_BUILD_TYPE=Release >/dev/null && \
    cmake --build build -j"$(nproc 2>/dev/null || echo 2)" >/dev/null && \
    cp build/pibot "$PREFIX/bin/pibot" && BUILT=1
elif command -v c++ >/dev/null 2>&1 || command -v clang++ >/dev/null 2>&1 || command -v g++ >/dev/null 2>&1; then
    echo "  [i] البناء المباشر عبر المُصرّف..."
    CXX=$(command -v clang++ || command -v g++ || command -v c++)
    "$CXX" -std=c++17 -O2 src/*.cpp -o "$PREFIX/bin/pibot" && BUILT=1
fi

if [ "$BUILT" = "1" ]; then
    chmod +x "$PREFIX/bin/pibot"
    echo
    echo -e "  \033[1;32m[✔] تم بناء وتثبيت النسخة الأصلية C++ بنجاح!\033[0m"
else
    # بديل أخير: نسخة Bash
    echo "  [!] تعذر البناء — سيتم استخدام نسخة Bash الاحتياطية."
    if [ "$IS_TERMUX" = "1" ]; then pkg install -y jq curl >/dev/null 2>&1 || true; fi
    chmod +x "$DIR/pibot.sh" "$DIR/lib/"*.sh
    cat > "$PREFIX/bin/pibot" <<EOF
#!/usr/bin/env bash
exec bash "$DIR/pibot.sh" "\$@"
EOF
    chmod +x "$PREFIX/bin/pibot"
    echo -e "  \033[1;32m[✔] تم تثبيت نسخة Bash.\033[0m"
fi

echo
echo "  التشغيل:"
echo "    pibot          ← القائمة الرئيسية"
echo "    pibot bot      ← البوت الذكي مباشرة"
echo "    pibot boost    ← تسريع سريع"
echo "    pibot deep     ← تسريع عميق"
echo "    pibot fps      ← قياس FPS"
echo
