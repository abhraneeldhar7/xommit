#!/usr/bin/env node

const { spawnSync } = require("node:child_process");
const fs = require("node:fs");
const https = require("node:https");
const os = require("node:os");
const path = require("node:path");

const pkg = require("../package.json");
const repo = "abhraneeldhar7/xommit";
const version = pkg.version;

function compareVersions(left, right) {
  const a = left.split(".").map((part) => Number.parseInt(part, 10) || 0);
  const b = right.split(".").map((part) => Number.parseInt(part, 10) || 0);
  const count = Math.max(a.length, b.length);
  for (let i = 0; i < count; i++) {
    const x = a[i] || 0;
    const y = b[i] || 0;
    if (x !== y) {
      return x < y ? -1 : 1;
    }
  }
  return 0;
}

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
        response.pipe(file);
        file.on("finish", () => file.close(resolve));
        file.on("error", reject);
      },
    );
    request.on("error", reject);
  });
}

async function ensureBinary() {
  const local = process.env.LOCALAPPDATA || path.join(os.homedir(), "AppData", "Local");
  const dir = path.join(local, "xommit", "bin");
  const exe = path.join(dir, "xommit.exe");
  const stamp = path.join(dir, "version.txt");

  let installed = "";
  try {
    installed = fs.readFileSync(stamp, "utf8").trim();
  } catch {
    installed = "";
  }

  if (fs.existsSync(exe) && installed && compareVersions(installed, version) >= 0) {
    return exe;
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
  fs.writeFileSync(stamp, `${version}\n`);
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
