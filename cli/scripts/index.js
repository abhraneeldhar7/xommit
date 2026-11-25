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
    const platform = os.platform(); // 'win32' | 'linux'
    const arch = os.arch(); // 'x64' | 'arm64'

    if (arch !== "x64") {
        throw new Error(`Unsupported architecture: ${arch}. Only x64 is supported.`);
    }

    let asset;
    let cacheDir;
    let exeName;

    if (platform === "win32") {
        asset = "xommit-windows-x64.exe";
        exeName = "xommit.exe";
        cacheDir = path.join(
            process.env.LOCALAPPDATA || path.join(os.homedir(), "AppData", "Local"),
            "xommit", "bin"
        );
    } else if (platform === "linux") {
        asset = "xommit-linux-x64";
        exeName = "xommit";
        cacheDir = path.join(
            process.env.XDG_BIN_HOME || path.join(os.homedir(), ".local", "bin")
        );
    } else {
        throw new Error(`Unsupported platform: ${platform}`);
    }

    const exe = path.join(cacheDir, exeName);
    const cacheFile = path.join(cacheDir, ".npm-version");

    if (fs.existsSync(exe)) {
        return exe;
    }

    fs.mkdirSync(cacheDir, { recursive: true });
    const temporary = path.join(cacheDir, `${exeName}.download`);
    process.stderr.write(`downloading xommit ${version}...\n`);

    await download(
        `https://github.com/${repo}/releases/download/v${version}/${asset}`,
        temporary,
    );

    const size = fs.statSync(temporary).size;
    if (size < 100000) {
        fs.rmSync(temporary, { force: true });
        throw new Error("downloaded file looks invalid");
    }

    fs.renameSync(temporary, exe);

    // Make executable on Linux
    if (platform === "linux") {
        fs.chmodSync(exe, 0o755);
    }

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