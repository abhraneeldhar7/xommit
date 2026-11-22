#include <windows.h>

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "xommit/config.h"
#include "xommit/diff.h"
#include "xommit/git.h"
#include "xommit/ignore.h"
#include "xommit/json.h"
#include "xommit/prompt.h"
#include "xommit/util.h"

using namespace xommit;

static int failures = 0;
static int checks = 0;

static void check(const std::string& name, bool condition) {
    checks++;
    if (!condition) {
        failures++;
        std::cout << "FAIL: " << name << "\n";
    }
}

static void check_eq(const std::string& name, const std::string& actual, const std::string& expected) {
    checks++;
    if (actual != expected) {
        failures++;
        std::cout << "FAIL: " << name << "\n  expected: [" << expected << "]\n  actual:   [" << actual << "]\n";
    }
}

static void check_eq_int(const std::string& name, int actual, int expected) {
    checks++;
    if (actual != expected) {
        failures++;
        std::cout << "FAIL: " << name << " expected " << expected << " got " << actual << "\n";
    }
}

static void remove_tree(const std::filesystem::path& root) {
    std::error_code error;
    if (!std::filesystem::exists(root, error)) {
        return;
    }
    for (const auto& entry : std::filesystem::recursive_directory_iterator(
             root, std::filesystem::directory_options::skip_permission_denied, error)) {
        std::filesystem::permissions(entry.path(), std::filesystem::perms::owner_all,
                                     std::filesystem::perm_options::add, error);
    }
    std::filesystem::remove_all(root, error);
}

static std::filesystem::path unique_temp_dir(const std::string& name) {
    return std::filesystem::temp_directory_path() /
           (name + "-" + std::to_string(GetCurrentProcessId()) + "-" +
            std::to_string(GetTickCount64()));
}

static bool valid_utf8(const std::string& text) {
    size_t i = 0;
    while (i < text.size()) {
        unsigned char c = static_cast<unsigned char>(text[i]);
        int extra = 0;
        if (c < 0x80) extra = 0;
        else if ((c & 0xE0) == 0xC0) extra = 1;
        else if ((c & 0xF0) == 0xE0) extra = 2;
        else if ((c & 0xF8) == 0xF0) extra = 3;
        else return false;
        if (i + extra >= text.size()) return false;
        for (int j = 1; j <= extra; j++) {
            if ((static_cast<unsigned char>(text[i + j]) & 0xC0) != 0x80) return false;
        }
        i += extra + 1;
    }
    return true;
}

static void test_sanitize() {
    check_eq("sanitize plain", sanitize_message("fix: foo"), "fix: foo");
    check_eq("sanitize trailing period", sanitize_message("fix: foo."), "fix: foo");
    check_eq("sanitize multiple periods", sanitize_message("fix: done..."), "fix: done");
    check_eq("sanitize quotes", sanitize_message("\"fix: foo\""), "fix: foo");
    check_eq("sanitize newline", sanitize_message("fix:\nfoo"), "fix: foo");
    check_eq("sanitize spaces", sanitize_message("  fix:   foo  "), "fix: foo");
    check_eq("sanitize empty", sanitize_message(""), "");

    std::string long_message = "feat: " + std::string(80, 'a');
    std::string cut = sanitize_message(long_message);
    check("sanitize long length", cut.size() <= 72);

    std::string spaced = "feat: " + std::string(40, 'a') + " " + std::string(40, 'b');
    std::string spaced_cut = sanitize_message(spaced);
    check("sanitize long at space", spaced_cut.size() <= 72);
    check("sanitize long no trailing space", spaced_cut.empty() || spaced_cut.back() != ' ');

    std::string unicode = "fix: ";
    for (int i = 0; i < 40; i++) {
        unicode += "\xC3\xA9";
    }
    std::string unicode_cut = sanitize_message(unicode);
    check("sanitize unicode valid utf8", valid_utf8(unicode_cut));
    check("sanitize unicode length", unicode_cut.size() <= 72);
}

static void test_parse_options() {
    std::vector<std::string> options = parse_commit_options(
        "{\"commits\":[\"feat: a\",\"fix: b\",\"docs: c\",\"chore: d\"]}");
    check_eq_int("parse object count", static_cast<int>(options.size()), 3);
    check_eq("parse object first", options[0], "feat: a");

    options = parse_commit_options("[\"feat: a\",\"fix: b\"]");
    check_eq_int("parse array count", static_cast<int>(options.size()), 2);

    options = parse_commit_options("```json\n{\"commits\":[\"feat: a\",\"feat: a\",\"fix: b\"]}\n```");
    check_eq_int("parse dedupe", static_cast<int>(options.size()), 2);

    options = parse_commit_options("Here you go:\n1. feat: add thing\n2. fix: repair thing\n3) docs: explain thing\n");
    check_eq_int("parse fallback count", static_cast<int>(options.size()), 3);
    check_eq("parse fallback first", options[0], "feat: add thing");
    check_eq("parse fallback third", options[2], "docs: explain thing");

    options = parse_commit_options("- feat: a\n* fix: b\n+ docs: c\n");
    check_eq_int("parse bullets", static_cast<int>(options.size()), 3);

    options = parse_commit_options("```\nfeat: a\nfix: b\n```");
    check_eq_int("parse fences", static_cast<int>(options.size()), 2);

    options = parse_commit_options("");
    check("parse empty", options.empty());

    options = parse_commit_options("{\"commits\":[\"\",\"  \",\"fix: ok\"]}");
    check_eq_int("parse skip blanks", static_cast<int>(options.size()), 1);
    check_eq("parse skip blanks value", options[0], "fix: ok");

    options = parse_commit_options("{\"commits\":[\"fix: has \\\"quotes\\\"\"]}");
    check_eq("parse escaped quotes", options[0], "fix: has \"quotes\"");

    options = parse_commit_options("no json here at all");
    check_eq_int("parse prose one line", static_cast<int>(options.size()), 0);

    options = parse_commit_options("{\"commits\":[\"not a commit\",\"fix: real fix\"]}");
    check_eq_int("parse rejects non conventional", static_cast<int>(options.size()), 1);
    check_eq("parse rejects non conventional value", options[0], "fix: real fix");

    options = parse_commit_options(
        "{\"commits\":[\"docs: add usage and feat(math): add multiply\",\"feat: add multiply and document usage\"]}");
    check_eq_int("parse rejects double prefix", static_cast<int>(options.size()), 1);
    check_eq("parse rejects double prefix value", options[0], "feat: add multiply and document usage");
}

static void test_parse_diff() {
    std::string raw =
        "diff --git a/src/a.js b/src/a.js\n"
        "index 111..222 100644\n"
        "--- a/src/a.js\n"
        "+++ b/src/a.js\n"
        "@@ -1 +1 @@\n"
        "-old\n"
        "+new\n"
        "diff --git a/new.txt b/new.txt\n"
        "new file mode 100644\n"
        "index 000..333\n"
        "--- /dev/null\n"
        "+++ b/new.txt\n"
        "@@ -0,0 +1 @@\n"
        "+hello\n"
        "diff --git a/gone.txt b/gone.txt\n"
        "deleted file mode 100644\n"
        "index 444..000\n"
        "--- a/gone.txt\n"
        "+++ /dev/null\n"
        "@@ -1 +0,0 @@\n"
        "-bye\n";
    std::vector<DiffFile> files = parse_diff(raw);
    check_eq_int("diff file count", static_cast<int>(files.size()), 3);
    check_eq("diff first path", files[0].path, "src/a.js");
    check_eq("diff first body", files[0].body, "@@ -1 +1 @@\n-old\n+new\n");
    check_eq("diff new path", files[1].path, "new.txt");
    check_eq("diff deleted path", files[2].path, "gone.txt");

    raw =
        "diff --git a/old.txt b/new.txt\n"
        "similarity index 100%\n"
        "rename from old.txt\n"
        "rename to new.txt\n";
    files = parse_diff(raw);
    check_eq_int("diff rename count", static_cast<int>(files.size()), 1);
    check_eq("diff rename path", files[0].path, "new.txt");
    check_eq("diff rename body", files[0].body, "(no content changes)\n");

    raw =
        "diff --git a/image.png b/image.png\n"
        "index 111..222 100644\n"
        "Binary files a/image.png and b/image.png differ\n";
    files = parse_diff(raw);
    check_eq_int("diff binary count", static_cast<int>(files.size()), 1);
    check("diff binary flag", files[0].binary);
    check_eq("diff binary body", files[0].body, "(binary file)\n");

    raw =
        "diff --git a/with space.txt b/with space.txt\n"
        "index 111..222 100644\n"
        "--- a/with space.txt\n"
        "+++ b/with space.txt\n"
        "@@ -1 +1 @@\n"
        "-a\n"
        "+b\n";
    files = parse_diff(raw);
    check_eq_int("diff space count", static_cast<int>(files.size()), 1);
    check_eq("diff space path", files[0].path, "with space.txt");

    raw =
        "diff --git a/x b/y.bin b/x b/y.bin\n"
        "index 111..222 100644\n"
        "Binary files a/x b/y.bin and b/x b/y.bin differ\n";
    files = parse_diff(raw);
    check_eq_int("diff binary space count", static_cast<int>(files.size()), 1);
    check_eq("diff binary space path", files[0].path, "x b/y.bin");
}

static void test_diff_context() {
    std::vector<DiffFile> files;
    bool truncated = false;
    check_eq("context empty", build_diff_context(files, 1000, truncated), "");

    DiffFile file;
    file.path = "a.txt";
    file.body = "hello\n";
    files.push_back(file);
    check_eq("context one", build_diff_context(files, 1000, truncated), "<a.txt>\nhello\n</a.txt>\n\n");
    check("context one not truncated", !truncated);

    files.clear();
    file.path = "big.txt";
    file.body = std::string(5000, 'x');
    files.push_back(file);
    std::string output = build_diff_context(files, 500, truncated);
    check("context truncated flag", truncated);
    check("context truncated note", output.find("some diffs were truncated") != std::string::npos);

    bool threw = false;
    try {
        build_diff_context(files, 0, truncated);
    } catch (const std::exception&) {
        threw = true;
    }
    check("context zero budget throws", threw);

    files.clear();
    DiffFile binary;
    binary.path = "image.png";
    binary.body = "(binary file)\n";
    binary.binary = true;
    files.push_back(binary);
    check_eq("context binary", build_diff_context(files, 1000, truncated), "<image.png>\n(binary file)\n</image.png>\n\n");
}

static void test_budget() {
    ModelSpec model;
    model.context_window = 32000;
    model.max_output_tokens = 512;
    int budget = context_budget_chars(model, "system", "hint");
    check("budget positive", budget > 1000);
    check_eq_int("estimate tokens empty", estimate_tokens(""), 0);
    check_eq_int("estimate tokens four", estimate_tokens("abcd"), 1);
    check_eq_int("estimate tokens five", estimate_tokens("abcde"), 2);

    ModelSpec tiny;
    tiny.context_window = 1000;
    tiny.max_output_tokens = 512;
    bool threw = false;
    try {
        context_budget_chars(tiny, "system", "hint");
    } catch (const std::exception&) {
        threw = true;
    }
    check("budget tiny throws", threw);

    ModelSpec unknown;
    check("budget unknown positive", context_budget_chars(unknown, "", "") > 1000);
}

static void test_ignore() {
    std::filesystem::path root = unique_temp_dir("xommit-test-ignore");
    remove_tree(root);
    std::filesystem::create_directories(root);

    IgnoreRules rules(root.string());
    check("ignore package-lock", rules.is_ignored("package-lock.json"));
    check("ignore nested package-lock", rules.is_ignored("sub/package-lock.json"));
    check("ignore png", rules.is_ignored("assets/logo.png"));
    check("ignore dist", rules.is_ignored("dist/app.js"));
    check("ignore nested dist", rules.is_ignored("src/dist/app.js"));
    check("ignore node_modules", rules.is_ignored("node_modules/x/index.js"));
    check("ignore exe", rules.is_ignored("build/tool.exe"));
    check("do not ignore js", !rules.is_ignored("src/app.js"));
    check("do not ignore markdown", !rules.is_ignored("README.md"));

    std::ofstream ignore_file(root / ".xommitignore");
    ignore_file << "# custom\n";
    ignore_file << "*.log\n";
    ignore_file << "!important.log\n";
    ignore_file << "generated/\n";
    ignore_file << "docs/internal.md\n";
    ignore_file.close();

    IgnoreRules custom(root.string());
    check("custom log", custom.is_ignored("a.log"));
    check("custom unignore", !custom.is_ignored("important.log"));
    check("custom generated", custom.is_ignored("src/generated/x.js"));
    check("custom path", custom.is_ignored("docs/internal.md"));
    check("custom path not elsewhere", !custom.is_ignored("other/docs/internal.md"));
    check("custom unrelated", !custom.is_ignored("abc.txt"));

    remove_tree(root);
}

static void test_json() {
    Json root = Json::parse("{\"a\":1,\"b\":\"two\",\"c\":[true,null,3.5],\"d\":{\"e\":\"x\"}}");
    check("json object", root.is_object());
    check_eq_int("json size", static_cast<int>(root.size()), 4);
    check_eq("json string", root.get_string("b"), "two");
    check_eq_int("json number", static_cast<int>(root.get_number("a")), 1);
    check_eq("json nested", root.find("d")->get_string("e"), "x");

    Json parsed = Json::parse("\"line\\nbreak \\u00e9 \\ud83d\\ude00\"");
    check_eq("json unicode", parsed.string, "line\nbreak \xC3\xA9 \xF0\x9F\x98\x80");

    Json dumped = json_object({{"x", Json("a\"b")}, {"y", Json(2)}});
    check_eq("json dump", dumped.dump(), "{\"x\":\"a\\\"b\",\"y\":2}");

    bool threw = false;
    try {
        Json::parse("{bad}");
    } catch (const std::exception&) {
        threw = true;
    }
    check("json invalid throws", threw);

    Json array = json_array({Json("a"), Json(1), Json(true)});
    check_eq_int("json array size", static_cast<int>(array.size()), 3);
    check_eq("json array at", array.at(0).string, "a");
}

static void test_config() {
    std::filesystem::path root = unique_temp_dir("xommit-test-config");
    remove_tree(root);
    std::filesystem::create_directories(root);
    SetEnvironmentVariableW(L"APPDATA", root.wstring().c_str());

    bool threw = false;
    try {
        load_config();
    } catch (const std::exception&) {
        threw = true;
    }
    check("config missing throws", threw);

    Config config;
    config.provider = "vercel";
    config.api_key = "secret";
    config.model_id = "model/x";
    save_config(config);
    check("config exists", config_exists());
    Config loaded = load_config();
    check_eq("config provider", loaded.provider, "vercel");
    check_eq("config key", loaded.api_key, "secret");
    check_eq("config model", loaded.model_id, "model/x");

    write_file(config_path(), "{not json");
    threw = false;
    try {
        load_config();
    } catch (const std::exception& error) {
        threw = std::string(error.what()).find("corrupt") != std::string::npos;
    }
    check("config corrupt message", threw);

    write_file(config_path(), "{\"provider\":\"vercel\"}");
    threw = false;
    try {
        load_config();
    } catch (const std::exception& error) {
        threw = std::string(error.what()).find("incomplete") != std::string::npos;
    }
    check("config incomplete message", threw);

    reset_config();
    check("config reset", !config_exists());
    remove_tree(root);
}

static void test_git_commit() {
    std::filesystem::path root = unique_temp_dir("xommit-test-git");
    remove_tree(root);
    std::filesystem::create_directories(root);

    std::filesystem::path original = std::filesystem::current_path();
    std::filesystem::current_path(root);

    check_eq_int("git init", std::system("git init -q"), 0);
    check_eq_int("git config email", std::system("git config user.email test@example.com"), 0);
    check_eq_int("git config name", std::system("git config user.name Test"), 0);
    check_eq_int("git config signing", std::system("git config commit.gpgsign false"), 0);
    check_eq_int("git config line endings", std::system("git config core.autocrlf false"), 0);
    write_file("file.txt", "hello\n");
    check_eq_int("git add", std::system("git add file.txt"), 0);
    check_eq_int("git initial commit", std::system("git commit -q -m initial"), 0);

    write_file("file.txt", "hello world\n");
    check_eq_int("git add multiword", std::system("git add file.txt"), 0);
    std::string message = "fix: handle \"quoted\" path C:\\temp and caf\xC3\xA9";
    check_eq_int("git commit multiword exit", git_commit(message), 0);
    check_eq_int("git log multiword", std::system("git log -1 --pretty=%B > message.txt"), 0);
    check_eq("git commit multiword message", trim(read_file("message.txt")), message);

    write_file("file.txt", "hello again\n");
    check_eq_int("git add second", std::system("git add file.txt"), 0);
    std::string trailing = "fix: trailing slash C:\\";
    check_eq_int("git commit trailing slash exit", git_commit(trailing), 0);
    check_eq_int("git log trailing slash", std::system("git log -1 --pretty=%B > message.txt"), 0);
    check_eq("git commit trailing slash message", trim(read_file("message.txt")), trailing);

    std::filesystem::current_path(original);
    remove_tree(root);
}

static void test_prompt() {
    std::string system = system_prompt();
    check("prompt json contract", system.find("commits") != std::string::npos);
    check("prompt conventional types", system.find("feat") != std::string::npos);
    check("prompt diff trust", system.find("trust the diff") != std::string::npos);
    check_eq("user prompt hint", build_user_prompt("fix it", "<a>\nx\n</a>"), "User hint: fix it\n\n<a>\nx\n</a>");
    check_eq("user prompt no hint", build_user_prompt("", "ctx"), "User hint: (none)\n\nctx");
}

int main() {
    test_sanitize();
    test_parse_options();
    test_parse_diff();
    test_diff_context();
    test_budget();
    test_ignore();
    test_json();
    test_config();
    test_git_commit();
    test_prompt();

    std::cout << checks << " checks, " << failures << " failures\n";
    return failures == 0 ? 0 : 1;
}
