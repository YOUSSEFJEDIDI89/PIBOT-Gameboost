#!/data/data/com.termux/files/usr/bin/bash
# واجهة وألوان

R='\033[1;31m'; G='\033[1;32m'; Y='\033[1;33m'
B='\033[1;34m'; M='\033[1;35m'; C='\033[1;36m'; N='\033[0m'
export R G Y B M C N

banner() {
    clear 2>/dev/null
    echo -e "${M}"
    cat <<'EOF'
   ██████╗ ██╗██████╗  ██████╗ ████████╗
   ██╔══██╗██║██╔══██╗██╔═══██╗╚══██╔══╝
   ██████╔╝██║██████╔╝██║   ██║   ██║
   ██╔═══╝ ██║██╔══██╗██║   ██║   ██║
   ██║     ██║██████╔╝╚██████╔╝   ██║
   ╚═╝     ╚═╝╚═════╝  ╚═════╝    ╚═╝
EOF
    echo -e "${C}        G A M E B O O S T  ⚡  v1.0${N}"
    echo -e "${Y}   مسرّع ألعاب حقيقي بدون روت + بوت ذكاء اصطناعي${N}"
    echo -e "${B}  ──────────────────────────────────────────────${N}"
}

ok()   { echo -e "  ${G}[✔]${N} $*"; }
warn() { echo -e "  ${Y}[!]${N} $*"; }
err()  { echo -e "  ${R}[✘]${N} $*"; }
info() { echo -e "  ${C}[i]${N} $*"; }

pause() {
    echo
    read -r -p "  اضغط Enter للمتابعة..." _
}

spinner_run() {
    # spinner_run "رسالة" command...
    local msg="$1"; shift
    echo -ne "  ${C}[…]${N} $msg "
    "$@" >/dev/null 2>&1 &
    local pid=$!
    local sp='⠋⠙⠹⠸⠼⠴⠦⠧⠇⠏'
    local i=0
    while kill -0 $pid 2>/dev/null; do
        printf '\b%s' "${sp:i++%10:1}"
        sleep 0.1
    done
    wait $pid
    local rc=$?
    if [ $rc -eq 0 ]; then
        printf '\b'; echo -e "\r  ${G}[✔]${N} $msg"
    else
        printf '\b'; echo -e "\r  ${Y}[~]${N} $msg (تخطّي)"
    fi
    return 0
}
