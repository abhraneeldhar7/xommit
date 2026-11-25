import { Check, X } from "lucide-react"
import Gradient from "../../assets/gradient.jpg"
import MeImg from "../../assets/me.png"
import { Marquee } from "../ui/marquee"

type CommitItemProps = {
    message: string
    username: string
    committedAt: string
    hash: string
    status?: "none" | "tick" | "cross"
    statusCount?: string
}

function CommitItem({ message, username, committedAt, hash, status = "none", statusCount }: CommitItemProps) {
    return (
        <div className="flex flex-col gap-1 border-b bg-background p-4 text-sm hover:bg-muted transition-all duration-default">

            <p className="truncate font-[400] text-[1.1rem] leading-[1.1em]">
                {message}
            </p>

            <div className="flex items-center gap-1.5 text-xs text-muted-foreground font-[300] leading-[1.1em]">
                <img
                    src={MeImg.src}
                    alt={username}
                    className="h-4 w-4 rounded-full object-cover"
                />
                <span>{username}</span>
                <span>committed {committedAt}</span>

                {status !== "none" && (
                    <>
                        <span className="text-md">·</span>
                        <span className="inline-flex items-center gap-1">
                            {status === "tick" && (
                                <Check className="h-3.5 w-3.5 text-blue-500" strokeWidth={2.5} />
                            )}
                            {status === "cross" && (
                                <X className="h-3.5 w-3.5 text-red-500" strokeWidth={2.5} />
                            )}
                            {statusCount && (
                                <span>{statusCount}</span>
                            )}
                        </span>
                    </>
                )}
            </div>
        </div>
    )
}

const commits: CommitItemProps[] = [
    {
        message: "feat(auth): add token refresh flow",
        username: "abhraneeldhar7",
        committedAt: "2 hours ago",
        hash: "a1b2c3d",
        status: "tick",
        statusCount: "12 / 12",
    },
    {
        message: "fix(payments): handle stripe webhook retries",
        username: "abhraneeldhar7",
        committedAt: "5 hours ago",
        hash: "e4f5g6h",
        status: "tick",
        statusCount: "8 / 8",
    },
    {
        message: "refactor(cache): pool redis connections",
        username: "abhraneeldhar7",
        committedAt: "yesterday",
        hash: "i7j8k9l",
        status: "cross",
        statusCount: "2 / 3",
    },
    {
        message: "chore(deps): bump astro to 7.3.2",
        username: "abhraneeldhar7",
        committedAt: "2 days ago",
        hash: "m0n1o2p",
        status: "tick",
        statusCount: "5 / 5",
    },
    {
        message: "docs(api): document webhook payloads",
        username: "abhraneeldhar7",
        committedAt: "3 days ago",
        hash: "q3r4s5t",
    },
]

export default function GithubRepoCommits() {
    return (<div className="my-30 site-max flex gap-10 md:flex-row flex-col">

        <div className="flex-1 sticky top-[100px] md:self-start">
            <h1 className="text-4xl">So your repo looks this good</h1>
            <p className="md:text-md text-muted-foreground mt-4 leading-[1.8em]">Lorem ipsum dolor sit amet consectetur adipisicing elit. Impedit quibusdam neque iusto nobis ipsam animi voluptate natus numquam quas, ut optio accusamus aliquid maxime perferendis dicta reprehenderit.</p>
        </div>

        <div className="flex-1 aspect-square">
            <div className="rounded-md h-full w-full relative overflow-hidden">
                <img src={Gradient.src} className="w-full h-full top-0 left-0 absolute object-cover z-[-1]" />

                <div className="w-full h-full p-5 md:p-15 flex items-center">
                    <div className="w-full h-full rounded-md bg-background overflow-hidden">
                        <Marquee vertical className="[--duration:15s] p-0 [--gap:0px]">
                            {commits.map((commit) => (
                                <CommitItem key={commit.hash} {...commit} />
                            ))}
                        </Marquee>
                    </div>
                </div>

            </div>
        </div>
    </div>)
}
