import { existsSync, readFileSync } from "node:fs"
import { resolve } from "node:path"

type PackageJson = {
    version: string
}

export function getCliVersion() {
    const candidates = [
        resolve(process.cwd(), "../cli/package.json"),
        resolve(process.cwd(), "cli/package.json"),
    ]
    const packageJsonPath = candidates.find((candidate) => existsSync(candidate))
    if (!packageJsonPath) {
        throw new Error("cannot find cli/package.json")
    }
    const { version }: PackageJson = JSON.parse(readFileSync(packageJsonPath, "utf8"))
    return version
}
