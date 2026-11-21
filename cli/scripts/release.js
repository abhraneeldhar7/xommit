const { execFileSync } = require("node:child_process");
const fs = require("node:fs");
const path = require("node:path");

const root = path.resolve(__dirname, "..");
const pkg = JSON.parse(fs.readFileSync(path.join(root, "package.json"), "utf8"));
const tag = `v${pkg.version}`;

function git(args) {
  return execFileSync("git", args, { cwd: root, encoding: "utf8" }).trim();
}

function fail(message) {
  console.error(`error: ${message}`);
  process.exit(1);
}

if (git(["status", "--porcelain"])) {
  fail("working tree is not clean, commit your changes first");
}

if (git(["tag", "--list", tag])) {
  fail(`tag ${tag} already exists`);
}

execFileSync("node", [path.join(root, "scripts", "build.js")], { stdio: "inherit" });

git(["tag", tag]);
git(["push", "origin", tag]);

console.log(`pushed ${tag}, the release workflow will build and publish`);
