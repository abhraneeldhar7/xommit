import MeTransparentImg from "../../assets/me_transparent_cropped.png"
import GmailLogo from "../../assets/gmail_logo.svg"
import TwitterLogo from "../../assets/twitter_logo.svg"
import GithubLogo from "../../assets/github_logo.svg"
import LinkedinLogo from "../../assets/linkedin_logo.svg"

export default function Footer() {

    return (<footer className="site-max flex flex-col gap-10">
        <div className="flex md:flex-row flex-col gap-6 justify-between">
            <div>
                {/* <h2 className="text-3xl">Other stuff I built</h2> */}
            </div>

            <nav aria-label="Social links" className="flex gap-1.5">
                <a href="https://github.com/abhraneeldhar7" target="_blank" rel="noopener" aria-label="abhraneeldhar7 on GitHub" className="transition-all duration-fast rounded-sm p-2 hover:bg-muted/50 flex items-center justify-center h-[35px] aspect-square overflow-hidden group">
                    <img src={GithubLogo.src} alt="GitHub" className="h-full w-full object-contain invert opacity-80 transition-all duration-fast group-hover:opacity-100" />
                </a>
                <a href="https://x.com/veryNeel" target="_blank" rel="noopener" aria-label="veryNeel on X" className="transition-all duration-fast rounded-sm p-2.5 hover:bg-muted/50 flex items-center justify-center h-[35px] aspect-square overflow-hidden">
                    <img src={TwitterLogo.src} alt="X" className="h-full w-full object-contain" />
                </a>
                <a href="https://www.linkedin.com/in/abhraneeldhar/" target="_blank" rel="noopener" aria-label="abhraneeldhar on LinkedIn" className="transition-all duration-fast rounded-sm p-2 hover:bg-muted/50 flex items-center justify-center h-[35px] aspect-square overflow-hidden group">
                    <img src={LinkedinLogo.src} alt="LinkedIn" className="h-full w-full object-contain grayscale transition-all duration-fast group-hover:grayscale-0" />
                </a>
                <a href="mailto:abhraneeldhar@gmail.com" aria-label="Email abhraneeldhar" className="transition-all duration-fast rounded-sm p-2 hover:bg-muted/50 flex items-center justify-center h-[35px] aspect-square overflow-hidden group">
                    <img src={GmailLogo.src} alt="Email" className="h-full w-full object-contain grayscale transition-all duration-fast group-hover:grayscale-0" />
                </a>
            </nav>
        </div>

        <div className="flex justify-center">
            <img src={MeTransparentImg.src} alt="Abhraneel Dhar, creator of xommit" className="w-full max-w-[300px] h-fit object-contain object-top" />
        </div>
    </footer>)
}