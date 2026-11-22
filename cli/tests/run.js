#!/usr/bin/env node

const { spawnSync } = require("node:child_process");
const fs = require("node:fs");
const os = require("node:os");
const path = require("node:path");

const root = path.resolve(__dirname, "..");
const exe = path.join(root, "bin", "xommit.exe");
const pkg = require(path.join(root, "package.json"));
const live = process.argv.includes("--live");

let checks = 0;
let failures = 0;

function preview(text) {
  if (!text) {
    return "";
  }
  return text.length > 2000 ? `${text.slice(0, 2000)}\n...` : text;
}

function check(name, condition, detail = "") {
  checks++;
  if (condition) {
    return;
  }
  failures++;
  console.log(`FAIL ${name}`);
  if (detail) {
    console.log(preview(detail));
  }
}

function checkContains(name, text, needle) {
  check(name, text.includes(needle), `expected ${JSON.stringify(needle)} in:\n${text}`);
}

function hasAnsi(text) {
  return /\u001b\[/.test(text);
}

function build() {
  const result = spawnSync(process.execPath, [path.join(root, "scripts", "build.js")], {
    cwd: root,
    stdio: "inherit",
  });
  if (result.error) {
    throw new Error(`build failed: ${result.error.message}`);
  }
  if (result.status !== 0) {
    throw new Error(`build failed with exit code ${result.status}`);
  }
}

function buildUnitTests(workDir) {
  const sources = fs
    .readdirSync(path.join(root, "src"))
    .filter((name) => name.endsWith(".cpp") && name !== "main.cpp")
    .sort()
    .map((name) => path.join("src", name));

  const testExe = path.join(workDir, "xommit-tests.exe");
  const args = [
    "-std=c++20",
    "-O1",
    "-Wall",
    "-Wextra",
    "-DUNICODE",
    "-D_UNICODE",
    `-DXOMMIT_VERSION="${pkg.version}"`,
    "-I",
    "include",
    path.join("tests", "test_logic.cpp"),
    ...sources,
    "-o",
    testExe,
    "-static",
    "-static-libgcc",
    "-static-libstdc++",
    "-lwinhttp",
  ];

  const result = spawnSync(process.env.CXX || "g++", args, {
    cwd: root,
    stdio: "inherit",
  });
  if (result.error) {
    throw new Error(
      `cannot compile tests with ${process.env.CXX || "g++"}: ${result.error.message}\n` +
        "install MinGW g++ and put it on PATH",
    );
  }
  if (result.status !== 0) {
    throw new Error(`test compile failed with exit code ${result.status}`);
  }
  return testExe;
}

function runUnitTests(testExe) {
  console.log("\nunit tests");
  const result = spawnSync(testExe, [], { cwd: root, stdio: "inherit" });
  check("unit tests exit", result.status === 0, `exit code ${result.status}`);
}

function makeEnv(workDir) {
  return {
    ...process.env,
    APPDATA: path.join(workDir, "appdata"),
    LOCALAPPDATA: path.join(workDir, "localappdata"),
  };
}

function runCli(args, options = {}) {
  const result = spawnSync(exe, args, {
    cwd: options.cwd || root,
    env: options.env,
    input: options.input === undefined ? "" : options.input,
    encoding: "utf8",
    timeout: 30000,
  });
  if (result.error) {
    throw new Error(`xommit ${args.join(" ")} failed: ${result.error.message}`);
  }
  return {
    status: result.status,
    stdout: result.stdout || "",
    stderr: result.stderr || "",
  };
}

function git(args, cwd) {
  const result = spawnSync("git", args, { cwd, encoding: "utf8" });
  if (result.error) {
    throw new Error(`git is required: ${result.error.message}`);
  }
  if (result.status !== 0) {
    throw new Error(`git ${args.join(" ")} failed:\n${result.stderr}`);
  }
  return result.stdout || "";
}

function smokeTests(workDir) {
  console.log("\ncli smoke tests");

  const repo = path.join(workDir, "repo");
  const appData = path.join(workDir, "appdata");
  const configDir = path.join(appData, "xommit");
  const configPath = path.join(configDir, "config.json");
  const env = makeEnv(workDir);

  fs.mkdirSync(repo, { recursive: true });
  fs.mkdirSync(configDir, { recursive: true });

  git(["init", "-q"], repo);
  git(["config", "user.email", "test@example.com"], repo);
  git(["config", "user.name", "Test"], repo);
  git(["config", "commit.gpgsign", "false"], repo);
  git(["config", "core.autocrlf", "false"], repo);

  let result = runCli(["--version"], { env });
  check("version exit", result.status === 0, `status ${result.status}`);
  checkContains("version output", result.stdout, `xommit ${pkg.version}`);
  check("version no ansi", !hasAnsi(result.stdout + result.stderr));

  result = runCli(["-v"], { env });
  check("short version exit", result.status === 0, `status ${result.status}`);
  checkContains("short version output", result.stdout, `xommit ${pkg.version}`);

  for (const flag of ["--help", "-h"]) {
    result = runCli([flag], { env });
    check(`${flag} exit`, result.status === 0, `status ${result.status}`);
    for (const needle of ["--connect", "--model", "--reset", "--update", "--version", "--help"]) {
      checkContains(`${flag} shows ${needle}`, result.stdout, needle);
    }
    check(`${flag} no ansi`, !hasAnsi(result.stdout + result.stderr));
  }

  result = runCli(["--nope"], { env });
  check("unknown flag exit", result.status === 1, `status ${result.status}`);
  checkContains("unknown flag message", result.stdout, "Command not recognized");
  check("unknown flag no ansi", !hasAnsi(result.stdout + result.stderr));

  result = runCli(["--reset"], { env });
  check("reset without config exit", result.status === 0, `status ${result.status}`);
  checkContains("reset without config message", result.stdout, "deleted");

  result = runCli([], { cwd: workDir, env });
  check("outside repo exit", result.status === 1, `status ${result.status}`);
  checkContains("outside repo error", result.stderr, "not a git repository");

  result = runCli([], { cwd: repo, env });
  check("empty repo exit", result.status === 0, `status ${result.status}`);
  checkContains("empty repo message", result.stdout, "No files are staged");

  fs.writeFileSync(path.join(repo, "package-lock.json"), "{}\n");
  git(["add", "package-lock.json"], repo);
  result = runCli([], { cwd: repo, env });
  check("ignored only exit", result.status === 0, `status ${result.status}`);
  checkContains("ignored only message", result.stdout, "only ignored files are staged");

  fs.writeFileSync(path.join(repo, "app.txt"), "hello\n");
  git(["add", "app.txt"], repo);
  result = runCli([], { cwd: repo, env });
  check("missing config exit", result.status === 1, `status ${result.status}`);
  checkContains("missing config prompt", result.stdout, "Connect your AI provider to begin");
  checkContains("missing config provider list", result.stdout, "Choose a provider");
  checkContains("missing config cancel", result.stdout, "cancelled");

  fs.writeFileSync(configPath, "{not json");
  result = runCli([], { cwd: repo, env });
  check("corrupt config exit", result.status === 1, `status ${result.status}`);
  checkContains("corrupt config error", result.stderr, "config file is corrupt");
  checkContains("corrupt config hint", result.stderr, "--reset");

  fs.writeFileSync(configPath, "{\"provider\":\"vercel\"}");
  result = runCli([], { cwd: repo, env });
  check("incomplete config exit", result.status === 1, `status ${result.status}`);
  checkContains("incomplete config error", result.stderr, "config file is incomplete");

  fs.rmSync(configPath, { force: true });
  result = runCli(["--connect"], { env, input: "" });
  check("connect cancel exit", result.status === 1, `status ${result.status}`);
  checkContains("connect cancel message", result.stdout, "cancelled");

  result = runCli(["--connect"], { env, input: "1\n" });
  check("connect no key exit", result.status === 1, `status ${result.status}`);
  checkContains("connect no key error", result.stderr, "no API key provided");

  result = runCli(["--connect"], { env, input: "9\n" });
  check("connect invalid choice exit", result.status === 1, `status ${result.status}`);
  checkContains("connect invalid choice message", result.stdout, "cancelled");

  result = runCli(["--model"], { env, input: "" });
  check("model without config exit", result.status === 1, `status ${result.status}`);
  checkContains("model without config prompt", result.stdout, "Connect your AI provider first");
  checkContains("model without config cancel", result.stdout, "cancelled");

  fs.mkdirSync(configDir, { recursive: true });
  fs.writeFileSync(
    configPath,
    JSON.stringify({ provider: "vercel", api_key: "dummy", model_id: "dummy/model" }) + "\n",
  );
  result = runCli(["--reset"], { env });
  check("reset with config exit", result.status === 0, `status ${result.status}`);
  check("reset deletes config", !fs.existsSync(configPath));
}

function liveTests(workDir) {
  console.log("\nlive tests");

  const sourceConfig = path.join(process.env.APPDATA || "", "xommit", "config.json");
  check("live config exists", fs.existsSync(sourceConfig), `missing ${sourceConfig}\nrun xommit --connect first`);
  if (!fs.existsSync(sourceConfig)) {
    return;
  }

  const appData = path.join(workDir, "live-appdata");
  const configDir = path.join(appData, "xommit");
  const repo = path.join(workDir, "live-repo");
  fs.mkdirSync(configDir, { recursive: true });
  fs.mkdirSync(repo, { recursive: true });
  fs.copyFileSync(sourceConfig, path.join(configDir, "config.json"));

  const env = makeEnv(workDir);
  env.APPDATA = appData;

  git(["init", "-q"], repo);
  git(["config", "user.email", "test@example.com"], repo);
  git(["config", "user.name", "Test"], repo);
  git(["config", "commit.gpgsign", "false"], repo);
  git(["config", "core.autocrlf", "false"], repo);

  const file = path.join(repo, "app.js");
  fs.writeFileSync(file, "export function add(a, b) {\n  return a + b;\n}\n");
  git(["add", "app.js"], repo);
  git(["commit", "-q", "-m", "initial"], repo);

  fs.writeFileSync(
    file,
    "export function add(a, b) {\n" +
      "  if (typeof a !== \"number\" || typeof b !== \"number\") {\n" +
      "    throw new TypeError(\"add expects numbers\");\n" +
      "  }\n" +
      "  return a + b;\n" +
      "}\n",
  );
  git(["add", "app.js"], repo);

  const result = runCli([], { cwd: repo, env, input: "1\n" });
  check("live commit exit", result.status === 0, `status ${result.status}\nstdout:\n${result.stdout}\nstderr:\n${result.stderr}`);
  checkContains("live commit picker", result.stdout, "Choose a commit message");

  if (result.status !== 0) {
    return;
  }

  const message = git(["log", "-1", "--pretty=%B"], repo).trim();
  console.log(`live commit message: ${message}`);
  check("live message conventional", /^[a-z]+(\([^)]+\))?: .+$/.test(message), message);
  check("live message length", message.length <= 72, message);
  check("live message no trailing period", !message.endsWith("."), message);
  check("live message single line", !message.includes("\n"), message);
}

function main() {
  const workDir = fs.mkdtempSync(path.join(os.tmpdir(), "xommit-test-"));

  try {
    build();
    const testExe = buildUnitTests(workDir);
    runUnitTests(testExe);
    smokeTests(workDir);
    if (live) {
      liveTests(workDir);
    } else {
      console.log("\nlive tests skipped (pass --live to call the configured provider)");
    }
  } catch (error) {
    failures++;
    console.error(`error: ${error.message}`);
  } finally {
    fs.rmSync(workDir, { recursive: true, force: true, maxRetries: 3, retryDelay: 100 });
  }

  console.log(`\n${checks} checks, ${failures} failures`);
  process.exitCode = failures === 0 ? 0 : 1;
}

main();
