const { spawnSync } = require("node:child_process");
const fs = require("node:fs");
const path = require("node:path");

const root = path.resolve(__dirname, "..");
const pkg = JSON.parse(fs.readFileSync(path.join(root, "package.json"), "utf8"));
const repo = "abhraneeldhar7/xommit";

const platformArg = process.argv.find(arg => arg.startsWith("--platform="));
const platform = platformArg ? platformArg.split("=")[1] : "windows";

if (platform !== "windows" && platform !== "linux") {
    console.error(`Invalid platform: ${platform}. Use --platform=windows or --platform=linux`);
    process.exit(1);
}

const srcDir = path.join(root, "src");
const allSources = fs
    .readdirSync(srcDir)
    .filter((name) => name.endsWith(".cpp"))
    .sort()
    .map((name) => path.join("src", name));

// Filter sources based on platform
const sources = allSources.filter(src => {
    const basename = path.basename(src);
    if (platform === "windows") {
        return !basename.endsWith("_linux.cpp");
    } else {
        // For linux, use _linux.cpp variants instead of base ones
        if (basename.endsWith("_linux.cpp")) {
            return true;
        }
        // Exclude windows-specific files
        return !["http.cpp", "git.cpp", "ui.cpp", "util.cpp", "update.cpp"].includes(basename);
    }
});

const outDir = path.join(root, "bin");
fs.mkdirSync(outDir, { recursive: true });

const isWindows = platform === "windows";
const exeName = isWindows ? "xommit.exe" : "xommit";
const assetName = exeName;

let args = [
    "-std=c++20",
    "-O2",
    "-Wall",
    "-Wextra",
    `-DXOMMIT_VERSION="${pkg.version}"`,
    `-DXOMMIT_REPO="${repo}"`,
    `-DXOMMIT_ASSET="${assetName}"`,
    `-DXOMMIT_HOMEPAGE="${pkg.homepage}"`,
    "-Iinclude",
    ...sources,
    "-o",
    path.join("bin", exeName),
];

if (isWindows) {
    args.unshift(
        "-municode",
        "-DUNICODE",
        "-D_UNICODE"
    );
    args.push(
        "-static",
        "-static-libgcc",
        "-static-libstdc++",
        "-lwinhttp"
    );
} else {
    // Linux: static link libcurl and dependencies
    args.push(
        "-static",
        "-static-libgcc",
        "-static-libstdc++",
        "-lcurl",
        "-lpthread"
    );
}

const result = spawnSync("g++", args, { cwd: root, stdio: "inherit" });

if (result.error) {
    console.error(`build failed: ${result.error.message}`);
    process.exit(1);
}
if (result.status !== 0) {
    process.exit(result.status ?? 1);
}

console.log(`built bin/${exeName} ${pkg.version} (${platform})`);