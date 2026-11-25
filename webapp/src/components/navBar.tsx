import { useEffect, useState } from "react"
import XommitLogo from "../assets/xommitlogo.png"
import npmLogo from "../assets/npm_logo.webp"
import githubLogo from "../assets/github_logo.svg"
import { cn } from "cn"

export default function NavBar() {
    const [compactNavbar, setCompactNavbar] = useState(false)

    useEffect(() => {
        const versionPill = document.getElementById("version-pill")
        if (!versionPill) return

        const observer = new IntersectionObserver(
            ([entry]) => {
                setCompactNavbar(!entry.isIntersecting)
            },
            { threshold: 0, rootMargin: "0px" }
        )

        observer.observe(versionPill)
        return () => observer.disconnect()
    }, [])

    return (
        <header className="fixed top-0 left-0 w-full z-5 bg-background p-4 group/navbar">

            <nav aria-label="Site links" className={cn("flex justify-between items-center transition-all duration-default ease-in-out w-full mx-auto max-w-[5000px]", compactNavbar && "max-w-[1200px] ")}>
                <div className="flex items-center gap-3">
                    <div className="rounded-xs overflow-hidden p-[2px] bg-destructive h-[27px] aspect-square grayscale group-hover/navbar:grayscale-0 transition-all duration-default ">
                        <img src={XommitLogo.src} alt="xommit" loading="eager" fetchPriority="high" decoding="sync" className="h-full w-full object-cover" />
                    </div>
                    <p className="text-sm text-muted-foreground leading-[0px]">3mb build</p>
                </div>

                <div className="flex gap-3 items-center">
                    <a href="https://github.com/abhraneeldhar7/xommit" target="_blank" rel="noopener" aria-label="xommit on GitHub" className="rounded-xs p-[7px] hover:bg-muted transition-all duration-default">
                        <img src={githubLogo.src} alt="GitHub" loading="eager" fetchPriority="high" className="h-[20px] object-contain w-fit invert" />
                    </a>

                    <a href="https://www.npmjs.com/package/@antkin/xommit" target="_blank" rel="noopener" aria-label="xommit on npm" className="rounded-xs p-[7px] pb-[5px] hover:bg-muted transition-all duration-default">
                        <img src={npmLogo.src} alt="npm" loading="eager" fetchPriority="high" className="h-[20px] object-contain w-fit grayscale group-hover/navbar:grayscale-0 transition-all duration-default" />
                    </a>
                </div>
            </nav>
        </header>
    )
}