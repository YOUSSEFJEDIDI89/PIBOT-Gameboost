#!/data/data/com.termux/files/usr/bin/bash
# 🤖 بوت PIBOT الذكي — AI حقيقي 100% عبر API
# المزودون: Google AI Studio (Gemini) / Claude / Base URL مخصص (OpenAI-متوافق) / مجاني بدون مفتاح

BOT_CFG="$PIBOT_CFG_DIR/bot.conf"
BOT_HIST="$PIBOT_CFG_DIR/history.json"

bot_load_cfg() {
    BOT_PROVIDER="free"
    GEMINI_API_KEY=""; GEMINI_MODEL="gemini-2.5-flash"
    CLAUDE_API_KEY=""; CLAUDE_MODEL="claude-sonnet-4-20250514"
    CUSTOM_BASE_URL=""; CUSTOM_API_KEY=""; CUSTOM_MODEL="gpt-4o-mini"
    FREE_MODEL="openai"
    [ -f "$BOT_CFG" ] && source "$BOT_CFG"
}

bot_save_cfg() {
    cat > "$BOT_CFG" <<EOF
BOT_PROVIDER="$BOT_PROVIDER"
GEMINI_API_KEY="$GEMINI_API_KEY"
GEMINI_MODEL="$GEMINI_MODEL"
CLAUDE_API_KEY="$CLAUDE_API_KEY"
CLAUDE_MODEL="$CLAUDE_MODEL"
CUSTOM_BASE_URL="$CUSTOM_BASE_URL"
CUSTOM_API_KEY="$CUSTOM_API_KEY"
CUSTOM_MODEL="$CUSTOM_MODEL"
FREE_MODEL="$FREE_MODEL"
EOF
    chmod 600 "$BOT_CFG"
}

BOT_SYSTEM_PROMPT='أنت PIBOT، خبير محترف في تسريع هواتف أندرويد وتحسين FPS في الألعاب بدون روت، تعمل داخل Termux.
- تجيب بالعربية بشكل مختصر وعملي ومباشر.
- عندك أداة PIBOT Gameboost على جهاز المستخدم تنفّذ أوامر shell حقيقية عبر ADB/Shizuku بدون روت.
- عندما يطلب المستخدم تحسيناً يمكن تنفيذه بأمر، ضع الأوامر داخل كتلة كود تبدأ بـ ```run وتنتهي بـ ``` وسيُعرض على المستخدم تنفيذها بموافقته.
- الأوامر التي تحتاج صلاحيات نظام (settings, cmd, pm, am, dumpsys, device_config) اكتبها بادئة بكلمة priv مثل: priv settings put global window_animation_scale 0.5
- كن صادقاً: لا تعد بزيادات FPS خيالية، واشرح ماذا يفعل كل أمر بسطر واحد.
- لا تقترح أبداً أوامر تحذف بيانات المستخدم أو تضر الجهاز.'

# ---------------- الإعدادات ----------------
bot_provider_menu() {
    bot_load_cfg
    banner
    echo -e "  ${M}🤖 اختيار مزود الذكاء الاصطناعي${N}\n"
    echo -e "  الحالي: ${G}$BOT_PROVIDER${N}\n"
    echo -e "  ${C}[1]${N} 🟢 مجاني بدون مفتاح (Pollinations AI)"
    echo -e "  ${C}[2]${N} 🔷 Google AI Studio — Gemini (مفتاح مجاني من aistudio.google.com)"
    echo -e "  ${C}[3]${N} 🟠 Claude (Anthropic API)"
    echo -e "  ${C}[4]${N} 🔧 Base URL مخصص (أي API متوافق مع OpenAI: OpenRouter, Groq, Ollama...)"
    echo -e "  ${C}[0]${N} رجوع"
    echo
    read -r -p "  اختر ⇢ " p
    case "$p" in
        1) BOT_PROVIDER="free"
           ok "تم اختيار المزود المجاني — يعمل فوراً بدون أي مفتاح." ;;
        2) BOT_PROVIDER="gemini"
           info "احصل على مفتاح مجاني من: ${C}https://aistudio.google.com/apikey${N}"
           read -r -p "  أدخل GEMINI API KEY ⇢ " GEMINI_API_KEY
           read -r -p "  الموديل [$GEMINI_MODEL] ⇢ " m; [ -n "$m" ] && GEMINI_MODEL="$m" ;;
        3) BOT_PROVIDER="claude"
           info "المفتاح من: ${C}https://console.anthropic.com${N}"
           read -r -p "  أدخل CLAUDE API KEY ⇢ " CLAUDE_API_KEY
           read -r -p "  الموديل [$CLAUDE_MODEL] ⇢ " m; [ -n "$m" ] && CLAUDE_MODEL="$m" ;;
        4) BOT_PROVIDER="custom"
           read -r -p "  Base URL (مثال https://openrouter.ai/api/v1) ⇢ " CUSTOM_BASE_URL
           CUSTOM_BASE_URL="${CUSTOM_BASE_URL%/}"
           read -r -p "  API KEY (اتركه فارغاً إن لم يلزم) ⇢ " CUSTOM_API_KEY
           read -r -p "  الموديل [$CUSTOM_MODEL] ⇢ " m; [ -n "$m" ] && CUSTOM_MODEL="$m" ;;
        *) return 0 ;;
    esac
    bot_save_cfg
    ok "تم حفظ الإعدادات."
}

# ---------------- بناء الطلبات ----------------
bot_call_api() {
    # $1 = ملف التاريخ (JSON: [{role,content},...])
    local hist="$1" resp
    case "$BOT_PROVIDER" in
        gemini)
            [ -z "$GEMINI_API_KEY" ] && { echo "__ERR__لا يوجد مفتاح Gemini. اذهب للإعدادات /provider"; return; }
            local body
            body=$(jq -n --arg sys "$BOT_SYSTEM_PROMPT" --slurpfile h "$hist" '{
                systemInstruction: {parts: [{text: $sys}]},
                contents: [$h[0][] | {role: (if .role=="assistant" then "model" else "user" end), parts: [{text: .content}]}],
                generationConfig: {maxOutputTokens: 2048}
            }')
            resp=$(curl -sS --max-time 90 \
                -H "Content-Type: application/json" \
                -H "x-goog-api-key: $GEMINI_API_KEY" \
                -d "$body" \
                "https://generativelanguage.googleapis.com/v1beta/models/${GEMINI_MODEL}:generateContent" 2>/dev/null)
            echo "$resp" | jq -r 'if .error then "__ERR__"+.error.message else (.candidates[0].content.parts[0].text // "__ERR__رد فارغ") end' 2>/dev/null \
                || echo "__ERR__فشل الاتصال بالإنترنت أو رد غير صالح"
            ;;
        claude)
            [ -z "$CLAUDE_API_KEY" ] && { echo "__ERR__لا يوجد مفتاح Claude. اذهب للإعدادات /provider"; return; }
            local body
            body=$(jq -n --arg sys "$BOT_SYSTEM_PROMPT" --arg model "$CLAUDE_MODEL" --slurpfile h "$hist" '{
                model: $model, max_tokens: 2048, system: $sys,
                messages: $h[0]
            }')
            resp=$(curl -sS --max-time 90 \
                -H "Content-Type: application/json" \
                -H "x-api-key: $CLAUDE_API_KEY" \
                -H "anthropic-version: 2023-06-01" \
                -d "$body" \
                "https://api.anthropic.com/v1/messages" 2>/dev/null)
            echo "$resp" | jq -r 'if .error then "__ERR__"+.error.message else (.content[0].text // "__ERR__رد فارغ") end' 2>/dev/null \
                || echo "__ERR__فشل الاتصال بالإنترنت أو رد غير صالح"
            ;;
        custom|free)
            local url model auth=()
            if [ "$BOT_PROVIDER" = "free" ]; then
                url="https://text.pollinations.ai/openai"
                model="$FREE_MODEL"
            else
                [ -z "$CUSTOM_BASE_URL" ] && { echo "__ERR__لم تضبط Base URL. اذهب للإعدادات /provider"; return; }
                url="$CUSTOM_BASE_URL/chat/completions"
                model="$CUSTOM_MODEL"
                [ -n "$CUSTOM_API_KEY" ] && auth=(-H "Authorization: Bearer $CUSTOM_API_KEY")
            fi
            local body
            body=$(jq -n --arg sys "$BOT_SYSTEM_PROMPT" --arg model "$model" --slurpfile h "$hist" '{
                model: $model,
                messages: ([{role:"system",content:$sys}] + $h[0])
            }')
            resp=$(curl -sS --max-time 90 \
                -H "Content-Type: application/json" "${auth[@]}" \
                -d "$body" "$url" 2>/dev/null)
            # pollinations قد يرجع نصاً خاماً أحياناً
            local parsed
            parsed=$(echo "$resp" | jq -r 'if type=="object" then (if .error then "__ERR__"+(.error.message // (.error|tostring)) else (.choices[0].message.content // "__ERR__رد فارغ") end) else . end' 2>/dev/null)
            if [ -n "$parsed" ]; then echo "$parsed"; else echo "${resp:-__ERR__لا يوجد رد}"; fi
            ;;
    esac
}

# ---------------- تنفيذ أوامر البوت (بموافقة المستخدم فقط) ----------------
bot_extract_and_run() {
    # يستخرج كتل ```run ... ``` من رد البوت
    local reply="$1"
    local blocks
    blocks=$(echo "$reply" | awk '/^```run/{f=1;next} /^```/{f=0;next} f')
    [ -z "$blocks" ] && return 0
    echo
    echo -e "  ${Y}⚡ البوت يقترح تنفيذ الأوامر التالية على جهازك:${N}"
    echo -e "${B}  ┌──────────────────────────────────────────${N}"
    echo "$blocks" | sed 's/^/  │ /'
    echo -e "${B}  └──────────────────────────────────────────${N}"
    read -r -p "  تنفيذ هذه الأوامر؟ (y/n) ⇢ " yn
    if [ "$yn" = "y" ] || [ "$yn" = "Y" ]; then
        echo
        local tmp="$PIBOT_CFG_DIR/.botcmd.sh"
        printf '%s\n' "$blocks" > "$tmp"
        ( source "$PIBOT_LIB/priv.sh"; source "$tmp" ) 2>&1 | sed 's/^/  /'
        rm -f "$tmp"
        ok "تم تنفيذ أوامر البوت."
    else
        info "تم الإلغاء — لم يُنفّذ شيء."
    fi
}

# ---------------- واجهة الدردشة ----------------
bot_main() {
    bot_load_cfg
    banner
    echo -e "  ${M}🤖 بوت PIBOT الذكي${N}  ${B}|${N}  المزود: ${G}$BOT_PROVIDER${N}"
    echo -e "  ${B}──────────────────────────────────────────────${N}"
    echo -e "  الأوامر: ${C}/provider${N} تغيير المزود  ${C}/new${N} محادثة جديدة  ${C}/exit${N} خروج"
    echo -e "  اسأله: كيف أزيد FPS في ببجي؟ / سرّع هاتفي / لماذا يسخن جهازي؟"
    echo

    echo "[]" > "$BOT_HIST"

    while true; do
        echo
        read -r -e -p "$(echo -e "  ${G}أنت ⇢ ${N}")" msg
        case "$msg" in
            ""|" ") continue ;;
            /exit|/q) break ;;
            /new) echo "[]" > "$BOT_HIST"; ok "بدأنا محادثة جديدة."; continue ;;
            /provider) bot_provider_menu; bot_load_cfg
                       echo -e "\n  المزود الآن: ${G}$BOT_PROVIDER${N}"; continue ;;
            /help) info "/provider تغيير المزود | /new محادثة جديدة | /exit خروج"; continue ;;
        esac

        # إضافة رسالة المستخدم للتاريخ
        jq --arg m "$msg" '. + [{role:"user", content:$m}]' "$BOT_HIST" > "$BOT_HIST.tmp" \
            && mv "$BOT_HIST.tmp" "$BOT_HIST"

        echo -ne "  ${C}PIBOT يفكر...${N}\r"
        local reply
        reply=$(bot_call_api "$BOT_HIST")
        printf '\r\033[K'

        if [ -z "$reply" ]; then
            err "لا يوجد رد — تحقق من الإنترنت."
            continue
        fi
        if [[ "$reply" == __ERR__* ]]; then
            err "خطأ من المزود: ${reply#__ERR__}"
            # إزالة رسالة المستخدم الفاشلة من التاريخ
            jq '.[:-1]' "$BOT_HIST" > "$BOT_HIST.tmp" && mv "$BOT_HIST.tmp" "$BOT_HIST"
            continue
        fi

        echo -e "  ${M}🤖 PIBOT ⇢${N}"
        echo "$reply" | sed 's/^/  /'

        # حفظ رد البوت في التاريخ
        jq --arg m "$reply" '. + [{role:"assistant", content:$m}]' "$BOT_HIST" > "$BOT_HIST.tmp" \
            && mv "$BOT_HIST.tmp" "$BOT_HIST"

        # عرض تنفيذ أي أوامر اقترحها البوت
        bot_extract_and_run "$reply"

        # الاحتفاظ بآخر 20 رسالة فقط
        jq 'if length > 20 then .[-20:] else . end' "$BOT_HIST" > "$BOT_HIST.tmp" \
            && mv "$BOT_HIST.tmp" "$BOT_HIST"
    done
}
