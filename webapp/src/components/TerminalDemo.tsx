import { useEffect, useState } from "react"
import { cn } from "@/lib/utils"
import { icons } from "@/lib/icons"
import XommitLogo from "./XommitLogo"

type Run = {
    command: string
    options: [string, string, string]
    selected: number
    hash: string
}

type Phase = "typing" | "generating" | "options" | "selected" | "committed"

const runs: Run[] = [
    {
        command: "fixed the webhook shit",
        options: [
            "fix(payments): retry failed stripe webhooks",
            "feat(payments): add webhook retry queue",
            "refactor(payments): simplify webhook handling",
        ],
        selected: 1,
        hash: "a1b2c3d",
    },
    {
        command: "auth token keeps dying",
        options: [
            "feat(auth): add silent token refresh",
            "fix(auth): refresh expired access tokens",
            "chore(auth): clean up token storage",
        ],
        selected: 2,
        hash: "e4f5a6b",
    },
    {
        command: "redis pool leaking",
        options: [
            "fix(cache): close idle redis connections",
            "refactor(cache): bound the redis pool",
            "perf(cache): reuse redis connections",
        ],
        selected: 2,
        hash: "c7d8e9f",
    },
    {
        command: "docs for the api",
        options: [
            "docs(api): document webhook payloads",
            "docs(api): add payment endpoint examples",
            "chore(api): tidy api reference",
        ],
        selected: 1,
        hash: "b1c2d3e",
    },
    {
        command: "bump astro finally",
        options: [
            "chore(deps): bump astro to 7.3.2",
            "build(deps): upgrade astro packages",
            "fix(deps): resolve astro peer warnings",
        ],
        selected: 2,
        hash: "f4a5b6c",
    },
]

const TYPING_MS = 50
const STEP_MS = 1500
const SELECTION_PHASE_MS = 500

const nextPhase: Record<Phase, Phase> = {
    typing: "generating",
    generating: "options",
    options: "selected",
    selected: "committed",
    committed: "typing",
}

export default function TerminalDemo() {
    const [runIndex, setRunIndex] = useState(0)
    const [phase, setPhase] = useState<Phase>("typing")
    const [typed, setTyped] = useState(0)
    const [displayedIndex, setDisplayedIndex] = useState(0)
    const [selectionAnimating, setSelectionAnimating] = useState(false)

    const run = runs[runIndex]

    useEffect(() => {
        if (phase === "typing") {
            if (typed < run.command.length) {
                const id = window.setTimeout(() => setTyped(typed + 1), TYPING_MS)
                return () => window.clearTimeout(id)
            }
            setPhase("generating")
            return
        }

        if (phase === "options" && !selectionAnimating) {
            setSelectionAnimating(true)
            setDisplayedIndex(0)
            const steps = run.selected
            let currentStep = 0
            const animate = () => {
                if (currentStep <= steps) {
                    setDisplayedIndex(currentStep)
                    currentStep++
                    if (currentStep <= steps) {
                        setTimeout(animate, SELECTION_PHASE_MS)
                    } else {
                        setSelectionAnimating(false)
                    }
                }
            }
            animate()
        }

        const id = window.setTimeout(() => {
            if (phase === "committed") {
                setRunIndex((runIndex + 1) % runs.length)
                setTyped(0)
            }
            setPhase(nextPhase[phase])
        }, STEP_MS)
        return () => window.clearTimeout(id)
    }, [phase, typed, runIndex, run.selected])

    const typing = phase === "typing"
    const command = typing ? run.command.slice(0, typed) : run.command
    const choice = run.options[run.selected]
    const activeIndex = phase === "options" ? displayedIndex : run.selected

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
                    {command}
                    {typing && <span className="animate-pulse">█</span>}
                </p>

                {(phase === "generating" || phase === "options" || phase === "selected") && (
                    <XommitLogo className="text-[8px] sm:text-[10px] leading-[1em]" />
                )}

                {phase === "generating" && <p className="shimmer font-[500] text-sm">Generating</p>}

                {(phase === "options" || phase === "selected") && (
                    <div className="text-sm">
                        <p>Choose a commit message:</p>
                        {run.options.map((option, index) => {
                            const active = index === activeIndex
                            return (
                                <p key={index} className={cn(active && "text-green-500")}>
                                    <span className="inline-block w-4">{active ? ">" : ""}</span>
                                    {index + 1}. {option}
                                </p>
                            )
                        })}
                    </div>
                )}

                {phase === "committed" && (
                    <div className="text-sm">
                        <p>[master {run.hash}] {choice}</p>
                        <p className="text-muted-foreground">1 file changed, 2 insertions(+), 1 deletion(-)</p>
                    </div>
                )}
            </div>
        </div>
    )
}
