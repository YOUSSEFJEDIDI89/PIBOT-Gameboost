#!/data/data/com.termux/files/usr/bin/bash
# وحدة الألعاب: FPS حقيقي + Game Mode + تردد الشاشة

# الحصول على التطبيق النشط حالياً
focused_pkg() {
    run_priv "dumpsys window 2>/dev/null | grep -m1 -E 'mCurrentFocus|mFocusedApp'" \
        | grep -oE '[a-zA-Z0-9_.]+/[a-zA-Z0-9_.]+' | head -1 | cut -d/ -f1
}

pick_game_pkg() {
    echo -e "  ${C}[1]${N} كتابة اسم الحزمة يدوياً (مثال: com.tencent.ig)"
    echo -e "  ${C}[2]${N} اختيار من الألعاب المثبتة الشائعة"
    read -r -p "  اختر ⇢ " m
    if [ "$m" = "2" ]; then
        info "جاري البحث عن ألعاب مثبتة..."
        local games
        games=$(run_priv pm list packages 2>/dev/null | sed 's/package://' | grep -Ei \
'tencent|pubg|garena|freefire|dts.freefire|activision|callofduty|mobile.legends|mobilelegends|supercell|clash|roblox|minecraft|mojang|efootball|konami|ea.gp|fifa|genshin|miHoYo|hoyoverse|riotgames|wildrift|carxtech|outfit7|gameloft|nexon|netease' | head -20)
        if [ -z "$games" ]; then
            warn "لم أجد ألعاباً معروفة، اكتب الحزمة يدوياً."
        else
            local i=1
            local arr=()
            while IFS= read -r g; do
                echo -e "   ${Y}[$i]${N} $g"
                arr+=("$g"); i=$((i+1))
            done <<< "$games"
            read -r -p "  رقم اللعبة ⇢ " gi
            PKG="${arr[$((gi-1))]}"
            [ -n "$PKG" ] && return 0
        fi
    fi
    read -r -p "  اسم الحزمة ⇢ " PKG
    [ -n "$PKG" ]
}

# ---------------- تعزيز لعبة ----------------
game_boost_menu() {
    banner
    echo -e "  ${M}🎮 تعزيز لعبة معيّنة${N}\n"
    need_priv_or_explain || return 1
    PKG=""
    pick_game_pkg || { err "لم يتم اختيار لعبة"; return 1; }
    if ! run_priv pm list packages 2>/dev/null | grep -q "package:$PKG$"; then
        warn "لم أتأكد من وجود $PKG — سأكمل على أي حال."
    fi
    echo
    info "اللعبة المستهدفة: ${G}$PKG${N}"
    echo

    spinner_run "تفعيل Game Mode: Performance (أندرويد 12+)" run_priv cmd game mode performance "$PKG"
    spinner_run "طلب فتح حد FPS من النظام" run_priv device_config put game_overlay "$PKG" "mode=2,fps=120:mode=3,fps=120"
    spinner_run "إيقاف تحسين البطارية لهذه اللعبة (منع الخنق)" run_priv dumpsys deviceidle whitelist "+$PKG"
    spinner_run "قتل الخلفية لتفريغ الرام" run_priv am kill-all
    echo
    read -r -p "  هل تريد إعادة ترجمة اللعبة لأقصى أداء؟ يستغرق دقائق (y/n) ⇢ " comp
    if [ "$comp" = "y" ] || [ "$comp" = "Y" ]; then
        info "جاري الترجمة الكاملة (pm compile speed)... اصبر، هذا يحسّن الأداء فعلياً"
        run_priv pm compile -m speed -f "$PKG" && ok "تمت الترجمة بنجاح!" || warn "لم تكتمل الترجمة"
    fi
    echo
    ok "تم تعزيز ${G}$PKG${N} — شغّل اللعبة الآن وقس النتيجة من الخيار [4]"
}

# ---------------- قياس FPS الحقيقي ----------------
fps_meter() {
    banner
    echo -e "  ${M}📊 قياس FPS الحقيقي${N}\n"
    need_priv_or_explain || return 1

    local rr
    rr=$(run_priv "dumpsys display 2>/dev/null | grep -m1 -oE 'renderFrameRate[= ]+[0-9.]+' | grep -oE '[0-9.]+'" | head -1)
    [ -n "$rr" ] && info "تردد الشاشة الحالي: ${G}${rr%%.*} Hz${N} (هذا هو سقف الـFPS)"

    local pkg
    pkg=$(focused_pkg)
    if [ -n "$pkg" ] && [ "$pkg" != "com.termux" ]; then
        info "التطبيق النشط: $pkg"
        read -r -p "  قياس هذا التطبيق؟ (y) أم كتابة حزمة أخرى (n) ⇢ " yn
        [ "$yn" != "y" ] && read -r -p "  اسم الحزمة ⇢ " pkg
    else
        read -r -p "  اسم حزمة اللعبة (شغّلها بنافذة منقسمة) ⇢ " pkg
    fi
    [ -z "$pkg" ] && { err "لا توجد حزمة"; return 1; }

    echo
    info "القياس المباشر لمدة 5 ثوانٍ لكل قراءة — Ctrl+C للإيقاف"
    echo
    trap 'trap - INT; return 0' INT
    while true; do
        run_priv dumpsys gfxinfo "$pkg" reset >/dev/null 2>&1
        sleep 5
        local out total janky
        out=$(run_priv dumpsys gfxinfo "$pkg" 2>/dev/null)
        total=$(echo "$out" | grep -m1 "Total frames rendered" | grep -oE '[0-9]+' | head -1)
        janky=$(echo "$out" | grep -m1 "Janky frames" | grep -oE '[0-9]+' | head -1)
        if [ -z "$total" ] || [ "$total" = "0" ]; then
            warn "اللعبة لا ترسم إطارات الآن (تأكد أنها مفتوحة على الشاشة)"
        else
            local fps=$((total / 5))
            local jp=0
            [ "$total" -gt 0 ] && jp=$((janky * 100 / total))
            local col=$G
            [ "$fps" -lt 45 ] && col=$Y
            [ "$fps" -lt 25 ] && col=$R
            echo -e "  🎯 FPS: ${col}${fps}${N}  |  إطارات متقطعة: ${jp}%  |  إجمالي: ${total} إطار/5ث"
        fi
    done
    trap - INT
}

# ---------------- رفع تردد الشاشة ----------------
refresh_rate_menu() {
    banner
    echo -e "  ${M}🖥️  رفع تردد الشاشة (أكبر زيادة حقيقية للـFPS)${N}\n"
    need_priv_or_explain || return 1

    info "الترددات المدعومة في شاشتك:"
    local modes
    modes=$(run_priv "dumpsys display 2>/dev/null | grep -oE 'fps=[0-9]+' | grep -oE '[0-9]+' | sort -run" | head -6)
    if [ -n "$modes" ]; then
        echo "$modes" | while read -r m; do echo -e "     ${G}●${N} ${m} Hz"; done
    else
        warn "لم أستطع قراءة الأوضاع — شاشتك غالباً 60/90/120"
    fi
    echo
    read -r -p "  أدخل التردد المطلوب تثبيته (مثال 90 أو 120) ⇢ " hz
    case "$hz" in (*[!0-9]*|'') err "قيمة غير صحيحة"; return 1 ;; esac

    run_priv settings put system peak_refresh_rate "$hz"
    run_priv settings put system min_refresh_rate "$hz"
    run_priv settings put secure refresh_rate_mode 2
    run_priv settings put global oneplus_screen_refresh_rate 0 2>/dev/null

    local now
    now=$(run_priv "dumpsys display 2>/dev/null | grep -m1 -oE 'renderFrameRate[= ]+[0-9.]+' | grep -oE '[0-9.]+'" | head -1)
    echo
    if [ -n "$now" ]; then
        ok "تم! تردد الشاشة الآن: ${G}${now%%.*} Hz${N} (كان النظام يخفضه تلقائياً)"
    else
        ok "تم إرسال الأوامر. تحقق من الإعدادات ← الشاشة."
    fi
    warn "ملاحظة: بعض الألعاب تقفل نفسها على 60 من داخلها — ارفع الإعداد من داخل اللعبة أيضاً."
    info "تثبيت التردد على الأعلى يستهلك بطارية أكثر — للإرجاع استخدم الخيار [8]."
}
