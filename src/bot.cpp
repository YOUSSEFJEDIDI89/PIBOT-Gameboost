// 🤖 بوت PIBOT الذكي — AI حقيقي 100% عبر API
// المزودون: Google AI Studio (Gemini) / Claude / Base URL مخصص (OpenAI-متوافق) / مجاني بدون مفتاح
#include "bot.hpp"
#include "http.hpp"
#include "json.hpp"
#include "priv.hpp"
#include "util.hpp"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <sys/stat.h>
#include <vector>

using namespace ui;

static const char* SYSTEM_PROMPT =
    "أنت PIBOT، خبير محترف في تسريع هواتف أندرويد وتحسين FPS في الألعاب بدون روت، تعمل داخل Termux.\n"
    "- تجيب بالعربية بشكل مختصر وعملي ومباشر.\n"
    "- عندك أداة PIBOT Gameboost على جهاز المستخدم تنفّذ أوامر shell حقيقية عبر ADB/Shizuku بدون روت.\n"
    "- عندما يطلب المستخدم تحسيناً يمكن تنفيذه بأمر، ضع الأوامر داخل كتلة كود تبدأ بـ ```run وتنتهي بـ ``` "
    "وسيُعرض على المستخدم تنفيذها بموافقته.\n"
    "- الأوامر التي تحتاج صلاحيات نظام (settings, cmd, pm, am, dumpsys, device_config) اكتبها بادئة بكلمة priv "
    "مثل: priv settings put global window_animation_scale 0.5\n"
    "- كن صادقاً: لا تعد بزيادات FPS خيالية، واشرح ماذا يفعل كل أمر بسطر واحد.\n"
    "- لا تقترح أبداً أوامر تحذف بيانات المستخدم أو تضر الجهاز.";

// ---------------- الإعدادات ----------------
struct BotCfg {
    std::string provider   = "free";
    std::string geminiKey, geminiModel = "gemini-2.5-flash";
    std::string claudeKey, claudeModel = "claude-sonnet-4-20250514";
    std::string customUrl, customKey, customModel = "gpt-4o-mini";
    std::string freeModel = "openai";
};

static std::string cfgPath() { return cfgDir() + "/bot.conf"; }

static std::string unquote(std::string v) {
    v = trim(v);
    if (v.size() >= 2 && v.front() == '"' && v.back() == '"')
        v = v.substr(1, v.size() - 2);
    return v;
}

static BotCfg loadCfg() {
    BotCfg c;
    std::ifstream f(cfgPath());
    std::string line;
    while (std::getline(f, line)) {
        size_t eq = line.find('=');
        if (eq == std::string::npos) continue;
        std::string k = trim(line.substr(0, eq));
        std::string v = unquote(line.substr(eq + 1));
        if      (k == "BOT_PROVIDER")    c.provider = v;
        else if (k == "GEMINI_API_KEY")  c.geminiKey = v;
        else if (k == "GEMINI_MODEL"   && !v.empty()) c.geminiModel = v;
        else if (k == "CLAUDE_API_KEY")  c.claudeKey = v;
        else if (k == "CLAUDE_MODEL"   && !v.empty()) c.claudeModel = v;
        else if (k == "CUSTOM_BASE_URL") c.customUrl = v;
        else if (k == "CUSTOM_API_KEY")  c.customKey = v;
        else if (k == "CUSTOM_MODEL"   && !v.empty()) c.customModel = v;
        else if (k == "FREE_MODEL"     && !v.empty()) c.freeModel = v;
    }
    return c;
}

static void saveCfg(const BotCfg& c) {
    std::ofstream f(cfgPath());
    f << "BOT_PROVIDER=\""    << c.provider    << "\"\n"
      << "GEMINI_API_KEY=\""  << c.geminiKey   << "\"\n"
      << "GEMINI_MODEL=\""    << c.geminiModel << "\"\n"
      << "CLAUDE_API_KEY=\""  << c.claudeKey   << "\"\n"
      << "CLAUDE_MODEL=\""    << c.claudeModel << "\"\n"
      << "CUSTOM_BASE_URL=\"" << c.customUrl   << "\"\n"
      << "CUSTOM_API_KEY=\""  << c.customKey   << "\"\n"
      << "CUSTOM_MODEL=\""    << c.customModel << "\"\n"
      << "FREE_MODEL=\""      << c.freeModel   << "\"\n";
    chmod(cfgPath().c_str(), 0600);
}

void botProviderMenu() {
    BotCfg c = loadCfg();
    banner();
    std::cout << "  " << M << "🤖 اختيار مزود الذكاء الاصطناعي" << N << "\n\n";
    std::cout << "  الحالي: " << G << c.provider << N << "\n\n";
    std::cout << "  " << C << "[1]" << N << " 🟢 مجاني بدون مفتاح (Pollinations AI)\n"
              << "  " << C << "[2]" << N << " 🔷 Google AI Studio — Gemini (مفتاح مجاني من aistudio.google.com)\n"
              << "  " << C << "[3]" << N << " 🟠 Claude (Anthropic API)\n"
              << "  " << C << "[4]" << N << " 🔧 Base URL مخصص (OpenRouter, Groq, Ollama...)\n"
              << "  " << C << "[0]" << N << " رجوع\n\n";
    std::string p = ask("اختر ⇢ ");
    if (p == "1") {
        c.provider = "free";
        ok("تم اختيار المزود المجاني — يعمل فوراً بدون أي مفتاح.");
    } else if (p == "2") {
        c.provider = "gemini";
        info("احصل على مفتاح مجاني من: https://aistudio.google.com/apikey");
        c.geminiKey = ask("أدخل GEMINI API KEY ⇢ ");
        std::string m = ask("الموديل [" + c.geminiModel + "] ⇢ ");
        if (!m.empty()) c.geminiModel = m;
    } else if (p == "3") {
        c.provider = "claude";
        info("المفتاح من: https://console.anthropic.com");
        c.claudeKey = ask("أدخل CLAUDE API KEY ⇢ ");
        std::string m = ask("الموديل [" + c.claudeModel + "] ⇢ ");
        if (!m.empty()) c.claudeModel = m;
    } else if (p == "4") {
        c.provider = "custom";
        c.customUrl = ask("Base URL (مثال https://openrouter.ai/api/v1) ⇢ ");
        while (!c.customUrl.empty() && c.customUrl.back() == '/') c.customUrl.pop_back();
        c.customKey = ask("API KEY (اتركه فارغاً إن لم يلزم) ⇢ ");
        std::string m = ask("الموديل [" + c.customModel + "] ⇢ ");
        if (!m.empty()) c.customModel = m;
    } else {
        return;
    }
    saveCfg(c);
    ok("تم حفظ الإعدادات.");
}

// ---------------- بناء الطلبات واستدعاء API ----------------
struct Msg {
    std::string role;    // "user" | "assistant"
    std::string content;
};

// ترجع {نجاح، نص}
static std::pair<bool, std::string> callApi(const BotCfg& c, const std::vector<Msg>& hist) {
    std::string body, url;
    std::vector<std::string> headers;
    std::string prov = c.provider;

    if (prov == "gemini") {
        if (c.geminiKey.empty())
            return {false, "لا يوجد مفتاح Gemini. استخدم /provider"};
        std::ostringstream b;
        b << "{\"systemInstruction\":{\"parts\":[{\"text\":\"" << mj::escape(SYSTEM_PROMPT)
          << "\"}]},\"contents\":[";
        for (size_t i = 0; i < hist.size(); ++i) {
            if (i) b << ",";
            b << "{\"role\":\"" << (hist[i].role == "assistant" ? "model" : "user")
              << "\",\"parts\":[{\"text\":\"" << mj::escape(hist[i].content) << "\"}]}";
        }
        b << "],\"generationConfig\":{\"maxOutputTokens\":2048}}";
        body = b.str();
        url = "https://generativelanguage.googleapis.com/v1beta/models/" + c.geminiModel +
              ":generateContent";
        headers.push_back("x-goog-api-key: " + c.geminiKey);
    } else if (prov == "claude") {
        if (c.claudeKey.empty())
            return {false, "لا يوجد مفتاح Claude. استخدم /provider"};
        std::ostringstream b;
        b << "{\"model\":\"" << mj::escape(c.claudeModel)
          << "\",\"max_tokens\":2048,\"system\":\"" << mj::escape(SYSTEM_PROMPT)
          << "\",\"messages\":[";
        for (size_t i = 0; i < hist.size(); ++i) {
            if (i) b << ",";
            b << "{\"role\":\"" << hist[i].role << "\",\"content\":\""
              << mj::escape(hist[i].content) << "\"}";
        }
        b << "]}";
        body = b.str();
        url = "https://api.anthropic.com/v1/messages";
        headers.push_back("x-api-key: " + c.claudeKey);
        headers.push_back("anthropic-version: 2023-06-01");
    } else { // custom | free  (OpenAI-متوافق)
        std::string model;
        if (prov == "free") {
            url = "https://text.pollinations.ai/openai";
            model = c.freeModel;
        } else {
            if (c.customUrl.empty())
                return {false, "لم تضبط Base URL. استخدم /provider"};
            url = c.customUrl + "/chat/completions";
            model = c.customModel;
            if (!c.customKey.empty())
                headers.push_back("Authorization: Bearer " + c.customKey);
        }
        std::ostringstream b;
        b << "{\"model\":\"" << mj::escape(model) << "\",\"messages\":[{\"role\":\"system\",\"content\":\""
          << mj::escape(SYSTEM_PROMPT) << "\"}";
        for (auto& m : hist)
            b << ",{\"role\":\"" << m.role << "\",\"content\":\"" << mj::escape(m.content) << "\"}";
        b << "]}";
        body = b.str();
    }

    std::string resp = httpPostJson(url, headers, body);
    if (resp.empty())
        return {false, "فشل الاتصال — تحقق من الإنترنت."};

    auto j = mj::parse(resp);
    if (!j) {
        // بعض المزودين المجانيين قد يرجعون نصاً خاماً
        if (prov == "free" && !trim(resp).empty()) return {true, resp};
        return {false, "رد غير صالح من المزود."};
    }

    // رسالة خطأ من المزود؟
    if (auto e = j->get("error")) {
        std::string msg;
        if (e->type == mj::Value::OBJ && e->get("message")) msg = e->get("message")->asStr();
        else if (e->type == mj::Value::STR) msg = e->asStr();
        return {false, "خطأ من المزود: " + (msg.empty() ? resp.substr(0, 200) : msg)};
    }

    std::string text;
    if (prov == "gemini") {
        if (auto cand = j->get("candidates"))
            if (auto c0 = cand->at(0))
                if (auto ct = c0->get("content"))
                    if (auto parts = ct->get("parts"))
                        if (auto p0 = parts->at(0))
                            if (auto t = p0->get("text")) text = t->asStr();
    } else if (prov == "claude") {
        if (auto ct = j->get("content"))
            if (auto c0 = ct->at(0))
                if (auto t = c0->get("text")) text = t->asStr();
    } else {
        if (auto ch = j->get("choices"))
            if (auto c0 = ch->at(0))
                if (auto msg = c0->get("message"))
                    if (auto t = msg->get("content")) text = t->asStr();
    }

    if (text.empty()) return {false, "رد فارغ من المزود."};
    return {true, text};
}

// ---------------- تنفيذ أوامر البوت (بموافقة المستخدم فقط) ----------------
static std::vector<std::string> extractRunLines(const std::string& reply) {
    std::vector<std::string> lines;
    std::istringstream ss(reply);
    std::string line;
    bool in = false;
    while (std::getline(ss, line)) {
        std::string t = trim(line);
        if (!in && t.rfind("```run", 0) == 0) { in = true; continue; }
        if (in && t.rfind("```", 0) == 0)     { in = false; continue; }
        if (in && !t.empty()) lines.push_back(t);
    }
    return lines;
}

static void offerRunBlocks(const std::string& reply) {
    auto lines = extractRunLines(reply);
    if (lines.empty()) return;

    std::cout << "\n  " << Y << "⚡ البوت يقترح تنفيذ الأوامر التالية على جهازك:" << N << "\n";
    std::cout << B << "  ┌──────────────────────────────────────────" << N << "\n";
    for (auto& l : lines) std::cout << B << "  │ " << N << l << "\n";
    std::cout << B << "  └──────────────────────────────────────────" << N << "\n";

    if (!confirm("تنفيذ هذه الأوامر؟")) {
        info("تم الإلغاء — لم يُنفّذ شيء.");
        return;
    }
    std::cout << "\n";
    for (auto& l : lines) {
        std::string out;
        int rc = 0;
        if (l.rfind("priv ", 0) == 0) {
            out = runPriv(l.substr(5), &rc);
        } else {
            out = execCapture(l, &rc);
        }
        std::cout << "  " << (rc == 0 ? G : Y) << "$" << N << " " << l << "\n";
        if (!trim(out).empty()) {
            std::istringstream ss(out);
            std::string ol;
            int shown = 0;
            while (std::getline(ss, ol) && shown++ < 10)
                std::cout << "    " << ol << "\n";
        }
    }
    ok("تم تنفيذ أوامر البوت.");
}

// ---------------- واجهة الدردشة ----------------
void botMain() {
    BotCfg cfg = loadCfg();
    banner();
    std::cout << "  " << M << "🤖 بوت PIBOT الذكي" << N << "  " << B << "|" << N
              << "  المزود: " << G << cfg.provider << N << "\n";
    std::cout << B << "  ──────────────────────────────────────────────" << N << "\n";
    std::cout << "  الأوامر: " << C << "/provider" << N << " تغيير المزود  " << C << "/new" << N
              << " محادثة جديدة  " << C << "/exit" << N << " خروج\n";
    std::cout << "  اسأله: كيف أزيد FPS في ببجي؟ / سرّع هاتفي / لماذا يسخن جهازي؟\n\n";

    std::vector<Msg> hist;

    while (true) {
        std::cout << "\n  " << G << "أنت ⇢ " << N;
        std::cout.flush();
        std::string msg;
        if (!std::getline(std::cin, msg)) break;
        msg = trim(msg);
        if (msg.empty()) continue;
        if (msg == "/exit" || msg == "/q") break;
        if (msg == "/new") {
            hist.clear();
            ok("بدأنا محادثة جديدة.");
            continue;
        }
        if (msg == "/provider") {
            botProviderMenu();
            cfg = loadCfg();
            std::cout << "\n  المزود الآن: " << G << cfg.provider << N << "\n";
            continue;
        }
        if (msg == "/help") {
            info("/provider تغيير المزود | /new محادثة جديدة | /exit خروج");
            continue;
        }

        hist.push_back({"user", msg});
        std::cout << "  " << C << "PIBOT يفكر..." << N << "\r" << std::flush;

        auto [okFlag, reply] = callApi(cfg, hist);
        std::cout << "\r\033[K";

        if (!okFlag) {
            err(reply);
            hist.pop_back(); // إزالة الرسالة الفاشلة
            continue;
        }

        std::cout << "  " << M << "🤖 PIBOT ⇢" << N << "\n";
        {
            std::istringstream ss(reply);
            std::string l;
            while (std::getline(ss, l)) std::cout << "  " << l << "\n";
        }

        hist.push_back({"assistant", reply});
        offerRunBlocks(reply);

        // الاحتفاظ بآخر 20 رسالة فقط
        if (hist.size() > 20)
            hist.erase(hist.begin(), hist.end() - 20);
    }
}
