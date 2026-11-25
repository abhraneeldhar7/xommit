import { cn } from "@/lib/utils";
import LogoSpace from "./LogoSpace"

export default function XommitLogo({ className }: { className?: string }) {
    return (
        <h2
            className={cn("font-mono font-normal text-[9px] sm:text-[20px] mx-auto sm:mx-0 w-fit tracking-normal leading-[1em] ", className)}
        >
            <span className="text-destructive">██╗<LogoSpace /><LogoSpace />██╗</span
            ><span
            ><LogoSpace /><LogoSpace />██████╗<LogoSpace /><LogoSpace
                />███╗<LogoSpace /><LogoSpace /><LogoSpace />███╗<LogoSpace
                />███╗<LogoSpace /><LogoSpace /><LogoSpace />███╗<LogoSpace
                />██╗<LogoSpace />████████╗</span
            ><br />
            <span className="text-destructive">╚██╗██╔╝</span><span
            ><LogoSpace />██╔═══██╗<LogoSpace />████╗<LogoSpace
                />████║<LogoSpace />████╗<LogoSpace />████║<LogoSpace
                />██║<LogoSpace />╚══██╔══╝</span
            ><br />
            <span className="text-destructive"><LogoSpace />╚███╔╝<LogoSpace /></span
            ><span
            ><LogoSpace />██║<LogoSpace /><LogoSpace /><LogoSpace
                />██║<LogoSpace />██╔████╔██║<LogoSpace
                />██╔████╔██║<LogoSpace />██║<LogoSpace /><LogoSpace
                /><LogoSpace /><LogoSpace />██║<LogoSpace /><LogoSpace
                /><LogoSpace /></span
            ><br />
            <span className="text-destructive"><LogoSpace />██╔██╗<LogoSpace /></span
            ><span
            ><LogoSpace />██║<LogoSpace /><LogoSpace /><LogoSpace
                />██║<LogoSpace />██║╚██╔╝██║<LogoSpace
                />██║╚██╔╝██║<LogoSpace />██║<LogoSpace /><LogoSpace
                /><LogoSpace /><LogoSpace />██║<LogoSpace /><LogoSpace
                /><LogoSpace /></span
            ><br />
            <span className="text-destructive">██╔╝<LogoSpace />██╗</span><span
            ><LogoSpace />╚██████╔╝<LogoSpace />██║<LogoSpace
                />╚═╝<LogoSpace />██║<LogoSpace />██║<LogoSpace
                />╚═╝<LogoSpace />██║<LogoSpace />██║<LogoSpace /><LogoSpace
                /><LogoSpace /><LogoSpace />██║<LogoSpace /><LogoSpace
                /><LogoSpace /></span
            ><br />
            <span className="text-destructive">╚═╝<LogoSpace /><LogoSpace />╚═╝</span
            ><span
            ><LogoSpace /><LogoSpace />╚═════╝<LogoSpace /><LogoSpace
                />╚═╝<LogoSpace /><LogoSpace /><LogoSpace /><LogoSpace
                /><LogoSpace />╚═╝<LogoSpace />╚═╝<LogoSpace /><LogoSpace
                /><LogoSpace /><LogoSpace /><LogoSpace />╚═╝<LogoSpace
                />╚═╝<LogoSpace /><LogoSpace /><LogoSpace /><LogoSpace
                />╚═╝<LogoSpace /><LogoSpace /><LogoSpace /></span
            >
        </h2>
    )
}