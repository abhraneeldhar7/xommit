import { useState } from "react"
import { cn, coptToClipboard } from "@/lib/utils"
import { icons } from "@/lib/icons"

type CopyCommandProps = {
    command: string
    prefix?: string
    className?: string
    fill?: boolean
}

export default function CopyCommand({ command, prefix = "$", className, fill = false }: CopyCommandProps) {
    const [copied, setCopied] = useState(false)

    return (
        <div className={cn("border rounded-md md:w-fit w-full p-[3px] flex gap-2 items-center", className)}>
            <p className={cn("truncate leading-[1.1em] px-3", fill ? "flex-1 min-w-0" : "md:w-70 w-full")}><span className="text-sm text-destructive mr-2">{prefix}</span>{command}</p>
            <button className={cn("bg-destructive px-3 py-2 rounded-sm transition-all duration-fast hover:bg-destructive/70 w-15 flex items-center justify-center relative overflow-hidden", copied && "hover:bg-destructive")} onClick={() => {
                coptToClipboard(command)
                setCopied(true)
                window.setTimeout(() => setCopied(false), 1000)
            }}>
                <div className={cn("flex items-center justify-center w-full transition-all duration-fast ease-out", copied && "translate-y-[-100%] opacity-0", !copied && "translate-y-0 opacity-100")}>
                    Copy
                </div>
                <div className={cn("absolute inset-0 flex items-center justify-center transition-all duration-fast ease-out", !copied && "translate-y-[100%] opacity-0", copied && "translate-y-0 opacity-100")}>
                    <icons.check className="h-4.5 w-4.5" strokeWidth={2} />
                </div>
            </button>
        </div>
    )
}
