import { ArrowUpRight } from "lucide-react"
import CopyCommand from "../CopyCommand"

export default function OtherWaysToInstall({ version }: { version: string }) {
    const releasesUrl = `https://github.com/abhraneeldhar7/xommit/releases/download/v${version}`
    return (<div id="other-install" className="scroll-mt-20 my-30 site-max flex flex-col gap-7">
        <h1 className="text-4xl">Other ways to install</h1>

        <div className="border-t border-l w-full grid md:grid-cols-3 grid-cols-1 group/install">
            <div className="p-4.5 border border-transparent border-r-border border-b-border transition-all duration-fast hover:border-border hover:bg-muted/50 group/item group-hover/install:opacity-60 group-hover/install:hover:opacity-100">
                <h2 className="text-3xl transition-colors duration-fast group-hover/install:text-muted-foreground group-hover/install:group-hover/item:text-foreground">Windows x64</h2>
                <p className="text-muted-foreground mt-2 transition-colors duration-fast group-hover/item:text-foreground/90 leading-[1.8em]">Fetches the latest binary, installs it for your user, and sets up your PATH so the command just works every time.</p>
                <CopyCommand
                    className="w-full mt-4"
                    fill
                    command="irm https://raw.githubusercontent.com/abhraneeldhar7/xommit/master/cli/scripts/install.ps1 | iex"
                />
            </div>

            <div className="p-4.5 border border-transparent border-r-border border-b-border transition-all duration-fast hover:border-border hover:bg-muted/50 group/item group-hover/install:opacity-60 group-hover/install:hover:opacity-100">
                <h2 className="text-3xl transition-colors duration-fast group-hover/install:text-muted-foreground group-hover/install:group-hover/item:text-foreground">Linux x64</h2>
                <p className="text-muted-foreground mt-2 transition-colors duration-fast group-hover/item:text-foreground/90 leading-[1.8em]">Pulls the latest build, installs it under your user directory, and reminds you to update your shell PATH for you.</p>
                <CopyCommand
                    className="w-full mt-4"
                    fill
                    command="curl -fsSL https://raw.githubusercontent.com/abhraneeldhar7/xommit/master/cli/scripts/install.sh | bash"
                />
            </div>

            <div className="p-4.5 border border-transparent border-r-border border-b-border transition-all duration-fast hover:border-border hover:bg-muted/50 group/item group-hover/install:opacity-60 group-hover/install:hover:opacity-100">
                <h2 className="text-3xl transition-colors duration-fast group-hover/install:text-muted-foreground group-hover/install:group-hover/item:text-foreground">Manual</h2>
                <p className="text-muted-foreground mt-2 transition-colors duration-fast group-hover/item:text-foreground/90 leading-[1.8em]">Download the build straight from GitHub Releases, then place it on your PATH and make it executable before running it in a terminal.</p>
                <div className="flex flex-col gap-2 mt-4">
                    <a className="hover:underline" href={`${releasesUrl}/xommit-windows-x64.exe`} target="_blank">Windows binary <ArrowUpRight className="inline" size={16}/></a>
                    <a className="hover:underline" href={`${releasesUrl}/xommit-linux-x64`} target="_blank">Linux binary <ArrowUpRight className="inline" size={16}/></a>
                </div>
            </div>
        </div>
    </div>)
}
