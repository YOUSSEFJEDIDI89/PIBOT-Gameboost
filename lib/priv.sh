#!/data/data/com.termux/files/usr/bin/bash
# طبقة الصلاحيات: تنفيذ أوامر shell الحقيقية بدون روت
# الترتيب: Shizuku (rish) ← ADB اللاسلكي ← روت (إن وجد) ← فشل

PRIV_MODE="none"

detect_priv() {
    PRIV_MODE="none"
    if command -v rish >/dev/null 2>&1 && rish -c "id" 2>/dev/null | grep -q "uid=2000"; then
        PRIV_MODE="shizuku"
    elif command -v adb >/dev/null 2>&1 && adb get-state 2>/dev/null | grep -q "device"; then
        PRIV_MODE="adb"
    elif command -v su >/dev/null 2>&1 && su -c "id" 2>/dev/null | grep -q "uid=0"; then
        PRIV_MODE="root"
    fi
    export PRIV_MODE
}

# تنفيذ أمر بصلاحيات shell (أو روت إن وجد)
run_priv() {
    detect_priv
    case "$PRIV_MODE" in
        shizuku) rish -c "$*" 2>/dev/null ;;
        adb)     adb shell "$@" 2>/dev/null ;;
        root)    su -c "$*" 2>/dev/null ;;
        *)       return 127 ;;
    esac
}
export -f run_priv 2>/dev/null || true

# اختصار للبوت: priv <أمر>
priv() { run_priv "$@"; }
export -f priv 2>/dev/null || true

has_priv() {
    detect_priv
    [ "$PRIV_MODE" != "none" ]
}

priv_status_line() {
    detect_priv
    case "$PRIV_MODE" in
        shizuku) echo -e "  ${G}● صلاحيات: Shizuku متصل (تأثير كامل بدون روت)${N}" ;;
        adb)     echo -e "  ${G}● صلاحيات: ADB لاسلكي متصل (تأثير كامل بدون روت)${N}" ;;
        root)    echo -e "  ${G}● صلاحيات: روت (تأثير كامل)${N}" ;;
        *)       echo -e "  ${Y}● صلاحيات: تيرمكس فقط — فعّل ADB من الخيار [7] لتأثير أقوى${N}" ;;
    esac
}

need_priv_or_explain() {
    if has_priv; then return 0; fi
    err "هذه الميزة تحتاج ADB لاسلكي أو Shizuku (بدون روت)."
    info "افتح الخيار ${C}[7]${N} من القائمة وسأرشدك خطوة بخطوة (دقيقتان فقط)."
    return 1
}

# ---------------- إعداد ADB اللاسلكي ----------------
adb_setup_menu() {
    banner
    echo -e "  ${M}🔌 إعداد الصلاحيات بدون روت${N}\n"
    detect_priv
    priv_status_line
    echo
    echo -e "  ${C}[1]${N} إعداد ADB اللاسلكي (أندرويد 11+ ، بدون كمبيوتر)"
    echo -e "  ${C}[2]${N} ربط Shizuku (إن كان مثبتاً)"
    echo -e "  ${C}[3]${N} فحص الاتصال"
    echo -e "  ${C}[0]${N} رجوع"
    echo
    read -r -p "  اختر ⇢ " c
    case "$c" in
        1) adb_wireless_wizard ;;
        2) shizuku_wizard ;;
        3) detect_priv; priv_status_line
           has_priv && run_priv "getprop ro.product.model" | head -1 ;;
        *) return 0 ;;
    esac
}

adb_wireless_wizard() {
    if ! command -v adb >/dev/null 2>&1; then
        spinner_run "تثبيت android-tools (adb)" pkg install -y android-tools
    fi
    echo
    info "الخطوات (لا تغلق تيرمكس، استخدم تقسيم الشاشة أو النافذة العائمة):"
    echo -e "   ${Y}1.${N} الإعدادات ← خيارات المطور ← ${C}التصحيح اللاسلكي${N} ← تفعيل"
    echo -e "   ${Y}2.${N} اضغط ${C}إقران الجهاز برمز إقران${N}"
    echo -e "   ${Y}3.${N} سيظهر لك: عنوان IP:منفذ + رمز إقران من 6 أرقام"
    echo
    read -r -p "  أدخل عنوان الإقران (مثال 192.168.1.5:37123) ⇢ " pair_addr
    read -r -p "  أدخل رمز الإقران (6 أرقام) ⇢ " pair_code
    [ -z "$pair_addr" ] && { err "لم تدخل العنوان"; return 1; }
    echo
    info "جاري الإقران..."
    if adb pair "$pair_addr" "$pair_code" 2>&1 | grep -qi "success"; then
        ok "تم الإقران بنجاح!"
    else
        err "فشل الإقران، تأكد من الرمز والعنوان وأعد المحاولة."
        return 1
    fi
    echo
    info "الآن ارجع لشاشة (التصحيح اللاسلكي) وانظر ${C}عنوان IP والمنفذ${N} الرئيسي (منفذ مختلف عن الإقران)"
    read -r -p "  أدخل عنوان الاتصال (مثال 192.168.1.5:40567) ⇢ " conn_addr
    if adb connect "$conn_addr" 2>&1 | grep -qi "connected"; then
        ok "تم الاتصال! أصبح لديك صلاحيات shell حقيقية بدون روت 🎉"
        detect_priv
    else
        err "فشل الاتصال، تحقق من المنفذ."
    fi
}

shizuku_wizard() {
    echo
    info "لتشغيل Shizuku بدون روت: افتح تطبيق Shizuku ← شغّله عبر (التصحيح اللاسلكي)"
    info "ثم من داخل التطبيق: (استخدام Shizuku في تطبيقات الطرفية) ← صدّر ملفات rish إلى Termux"
    echo
    if command -v rish >/dev/null 2>&1; then
        if rish -c "id" 2>/dev/null | grep -q uid; then
            ok "rish يعمل! Shizuku متصل."
        else
            err "rish موجود لكن Shizuku غير مشغّل. شغّله من التطبيق أولاً."
        fi
    else
        warn "ملف rish غير موجود في المسار. ضعه في: $PREFIX/bin/rish مع rish_shizuku.dex"
    fi
}
