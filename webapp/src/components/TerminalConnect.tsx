import { cn } from "@/lib/utils"
import { icons } from "@/lib/icons"
import XommitLogo from "./XommitLogo"

type Provider = {
    id: string
    label: string
    recommended: boolean
}

const providers: Provider[] = [
    { id: "google", label: "Google AI Studio", recommended: true },
    { id: "vercel", label: "Vercel AI Gateway", recommended: false },
]

export default function TerminalConnect() {
    return (
        <div className="bg-[#101010] shadow-md border aspect-[5/3] w-full flex flex-col rounded-[5px] overflow-hidden">
            <div className="w-full h-7 flex justify-end items-center">
                <div className="w-fit h-full p-[6px] px-[10px] hover:bg-muted flex items-center transition-all duration-fast"><icons.Minus className="h-3 w-3" /></div>
                <div className="w-fit h-full p-[6px] px-[10px] hover:bg-muted flex items-center transition-all duration-fast"><icons.Square className="h-2.5 w-2.5" /></div>
                <div className="w-fit h-full p-[6px] px-[8px] hover:bg-[red] flex items-center transition-all duration-fast"><icons.Close className="h-3 w-3" /></div>
            </div>

            <div className="p-3 px-5 space-y-5">
                <p className="text-sm">
                    <span className="text-foreground/90">C:\projects\palantir{">"}</span>
                    <span className="text-destructive mx-1">$</span>
                    xommit --connect
                </p>

                <XommitLogo className="text-[8px] sm:text-[10px] leading-[1em]" />

                <div className="text-sm">
                    <p>Choose a provider:</p>
                    {providers.map((provider, index) => {
                        return (
                            <p key={index} className="text-green-500">
                                <span className="inline-block w-4">{index === 0 ? ">" : ""}</span>
                                {index + 1}. {provider.label}
                                {provider.recommended && <span className="ml-2 text-dim text-xs">Recommended</span>}
                            </p>
                        )
                    })}
                </div>

                <p className="text-sm text-muted-foreground">
                    <span className="text-destructive mx-1">$</span> Paste your API key...
                </p>
            </div>
        </div>
    )
}