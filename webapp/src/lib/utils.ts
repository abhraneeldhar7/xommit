export { cn } from "cn"

export async function coptToClipboard(str: string) {
    try {
        await navigator.clipboard.writeText(str);
    } catch {
        return;
    }
}