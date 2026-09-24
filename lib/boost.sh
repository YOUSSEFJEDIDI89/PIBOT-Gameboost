#!/data/data/com.termux/files/usr/bin/bash
# محرك التسريع

ram_free_mb() {
    awk '/MemAvailable/ {printf "%d", $2/1024}' /proc/meminfo 2>/dev/null || echo "?"
}

# ---------------- تسريع سريع (تيرمكس فقط، بدون أي صلاحيات) ----------------
quick_boost() {
    banner
    echo -e "  ${M}⚡ التسريع السريع (بدون روت)${N}\n"
    local before after
    before=$(ram_free_mb)

    spinner_run "تنظيف ملفات تيرمكس المؤقتة" bash -c 'rm -rf "$PREFIX/tmp/"* "$HOME/.cache/"* 2>/dev/null; true'
    spinner_run "مزامنة الذاكرة وإسقاط الكاش الداخلي" bash -c 'sync; true'
    spinner_run "تثبيت أولوية تيرمكس (منع الخنق أثناء اللعب)" termux-wake-lock
    spinner_run "محاولة قتل عمليات الخلفية" am kill-all
    # تنظيف كاش التطبيقات إن توفرت صلاحيات
    if has_priv; then
        spinner_run "تنظيف كاش كل التطبيقات (حقيقي)" run_priv pm trim-caches 512G
        spinner_run "قتل كل عمليات الخلفية (حقيقي)" run_priv am kill-all
    fi

    sleep 1
    after=$(ram_free_mb)
    echo
    ok "الرام المتاحة قبل: ${Y}${before} MB${N} ← بعد: ${G}${after} MB${N}"
    if ! has_priv; then
        warn "بدون ADB/Shizuku التنظيف محدود. فعّل الخيار [7] لنتيجة أقوى بكثير."
    fi
    info "نصيحة: أغلق التطبيقات من الزر المربع قبل اللعب، وفعّل وضع الطيران إذا كانت اللعبة أوفلاين."
}

# ---------------- تسريع عميق (صلاحيات shell حقيقية) ----------------
deep_boost() {
    banner
    echo -e "  ${M}🚀 التسريع العميق (تأثير حقيقي على مستوى النظام)${N}\n"
    need_priv_or_explain || return 1

    local before after
    before=$(ram_free_mb)

    spinner_run "تنظيف كاش جميع التطبيقات" run_priv pm trim-caches 999G
    spinner_run "قتل جميع عمليات الخلفية" run_priv am kill-all
    spinner_run "تسريع الأنيميشن x0.5 (استجابة أسرع)" bash -c '
        run_priv settings put global window_animation_scale 0.5
        run_priv settings put global transition_animation_scale 0.5
        run_priv settings put global animator_duration_scale 0.5'
    spinner_run "تفعيل وضع الأداء الثابت (منع خفض التردد)" run_priv cmd power set-fixed-performance-mode-enabled true
    spinner_run "تفعيل تجميد تطبيقات الخلفية" run_priv settings put global cached_apps_freezer enabled
    spinner_run "إيقاف مسح الواي فاي الخلفي (يقلل اللاق)" bash -c '
        run_priv settings put global wifi_scan_always_enabled 0
        run_priv settings put global ble_scan_always_enabled 0'
    spinner_run "تعطيل الحد من عمليات الخلفية الزائدة" run_priv settings put global activity_starts_logging_enabled 0

    sleep 1
    after=$(ram_free_mb)
    echo
    ok "اكتمل التسريع العميق! الرام: ${Y}${before} MB${N} ← ${G}${after} MB${N}"
    info "لأفضل نتيجة: شغّل أيضاً الخيار [5] لرفع تردد الشاشة، و[3] لتعزيز لعبتك."
    warn "لإرجاع كل شيء كما كان: الخيار [8]."
}

# ---------------- استرجاع الإعدادات ----------------
restore_defaults() {
    banner
    echo -e "  ${M}♻️  استرجاع الإعدادات الافتراضية${N}\n"
    need_priv_or_explain || return 1
    spinner_run "إرجاع سرعة الأنيميشن 1.0" bash -c '
        run_priv settings put global window_animation_scale 1.0
        run_priv settings put global transition_animation_scale 1.0
        run_priv settings put global animator_duration_scale 1.0'
    spinner_run "إيقاف وضع الأداء الثابت" run_priv cmd power set-fixed-performance-mode-enabled false
    spinner_run "إرجاع تردد الشاشة للتلقائي" bash -c '
        run_priv settings delete system min_refresh_rate
        run_priv settings delete system peak_refresh_rate'
    spinner_run "إرجاع مسح الواي فاي" run_priv settings put global wifi_scan_always_enabled 1
    termux-wake-unlock 2>/dev/null
    ok "تم إرجاع كل شيء للوضع الافتراضي."
}

# ---------------- معلومات الجهاز ----------------
device_info() {
    banner
    echo -e "  ${M}ℹ️  معلومات الجهاز${N}\n"
    echo -e "  ${C}الجهاز:${N}     $(getprop ro.product.brand 2>/dev/null) $(getprop ro.product.model 2>/dev/null)"
    echo -e "  ${C}أندرويد:${N}    $(getprop ro.build.version.release 2>/dev/null) (SDK $(getprop ro.build.version.sdk 2>/dev/null))"
    echo -e "  ${C}المعالج:${N}    $(getprop ro.board.platform 2>/dev/null) - $(nproc 2>/dev/null) أنوية"
    echo -e "  ${C}الرام الكلية:${N} $(awk '/MemTotal/ {printf "%d MB", $2/1024}' /proc/meminfo 2>/dev/null)"
    echo -e "  ${C}الرام المتاحة:${N} $(ram_free_mb) MB"
    if command -v termux-battery-status >/dev/null 2>&1; then
        local bat
        bat=$(timeout 5 termux-battery-status 2>/dev/null)
        if [ -n "$bat" ]; then
            echo -e "  ${C}البطارية:${N}   $(echo "$bat" | jq -r '.percentage')% - حرارة $(echo "$bat" | jq -r '.temperature | floor')°C"
        fi
    fi
    if has_priv; then
        local rr
        rr=$(run_priv "dumpsys display 2>/dev/null | grep -m1 -oE 'renderFrameRate[= ]+[0-9.]+' | grep -oE '[0-9.]+'" | head -1)
        [ -n "$rr" ] && echo -e "  ${C}تردد الشاشة الحالي:${N} ${G}${rr%%.*} Hz${N}"
    fi
}
