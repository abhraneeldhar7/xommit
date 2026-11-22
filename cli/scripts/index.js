#!/usr/bin/env node

const { spawnSync } = require("node:child_process");
const fs = require("node:fs");
const https = require("node:https");
const os = require("node:os");
const path = require("node:path");

const pkg = require("../package.json");
const repo = "abhraneeldhar7/xommit";
const version = pkg.version;

function download(url, target, redirects = 0) {
  return new Promise((resolve, reject) => {
    if (redirects > 5) {
      reject(new Error("too many redirects"));
      return;
    }
    const request = https.get(
      url,
      { headers: { "User-Agent": `xommit/${version}` } },
      (response) => {
        const status = response.statusCode || 0;
        if (status >= 300 && status < 400 && response.headers.location) {
          response.resume();
          const next = new URL(response.headers.location, url).toString();
          download(next, target, redirects + 1).then(resolve, reject);
          return;
        }
        if (status !== 200) {
          response.resume();
          reject(new Error(`HTTP ${status}`));
          return;
        }
        const file = fs.createWriteStream(target);
        response.on("error", reject);
        response.pipe(file);
        file.on("finish", () => file.close(resolve));
        file.on("error", reject);
      },
    );
    request.setTimeout(120000, () => {
      request.destroy(new Error("download timed out"));
    });
    request.on("error", reject);
  });
}

async function ensureBinary() {
  const local = process.env.LOCALAPPDATA || path.join(os.homedir(), "AppData", "Local");
  const dir = path.join(local, "xommit", "bin");
  const exe = path.join(dir, "xommit.exe");
  const cacheFile = path.join(dir, ".npm-version");

  if (fs.existsSync(exe)) {
    let cached = "";
    try { cached = fs.readFileSync(cacheFile, "utf8").trim(); } catch {}
    if (cached === version) {
      return exe;
    }
  }

  fs.mkdirSync(dir, { recursive: true });
  const temporary = path.join(dir, "xommit.download.exe");
  process.stderr.write(`downloading xommit ${version}...\n`);
  await download(
    `https://github.com/${repo}/releases/download/v${version}/xommit.exe`,
    temporary,
  );

  const size = fs.statSync(temporary).size;
  if (size < 100000) {
    fs.rmSync(temporary, { force: true });
    throw new Error("downloaded file looks invalid");
  }

  fs.renameSync(temporary, exe);
  fs.writeFileSync(cacheFile, version);
  return exe;
}

async function main() {
  const exe = await ensureBinary();
  const result = spawnSync(exe, process.argv.slice(2), { stdio: "inherit" });
  if (result.error) {
    throw new Error(`cannot run xommit: ${result.error.message}`);
  }
  process.exit(result.status ?? 1);
}

main().catch((error) => {
  console.error(`error: ${error.message}`);
  process.exit(1);
});