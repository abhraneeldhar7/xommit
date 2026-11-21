const { spawnSync } = require("node:child_process");
const fs = require("node:fs");
const path = require("node:path");

const root = path.resolve(__dirname, "..");
const pkg = JSON.parse(fs.readFileSync(path.join(root, "package.json"), "utf8"));
const repo = "abhraneeldhar7/xommit";

const srcDir = path.join(root, "src");
const sources = fs
  .readdirSync(srcDir)
  .filter((name) => name.endsWith(".cpp"))
  .sort()
  .map((name) => path.join("src", name));

const outDir = path.join(root, "bin");
fs.mkdirSync(outDir, { recursive: true });

const args = [
  "-std=c++20",
  "-O2",
  "-Wall",
  "-Wextra",
  "-municode",
  "-DUNICODE",
  "-D_UNICODE",
  `-DXOMMIT_VERSION="${pkg.version}"`,
  `-DXOMMIT_REPO="${repo}"`,
  `-DXOMMIT_ASSET="xommit.exe"`,
  "-Iinclude",
  ...sources,
  "-o",
  path.join("bin", "xommit.exe"),
  "-static",
  "-static-libgcc",
  "-static-libstdc++",
  "-lwinhttp",
];

const result = spawnSync("g++", args, { cwd: root, stdio: "inherit" });

if (result.error) {
  console.error(`build failed: ${result.error.message}`);
  process.exit(1);
}
if (result.status !== 0) {
  process.exit(result.status ?? 1);
}

console.log(`built bin/xommit.exe ${pkg.version}`);
