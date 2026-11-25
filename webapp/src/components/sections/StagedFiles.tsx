import { ChevronDown, FileIcon, LockIcon, SmileIcon } from "lucide-react"
import Gradient from "../../assets/gradient.jpg"
import { cn } from "@/lib/utils"

function FileItem({ filename, delay = 0, animate = true }: { filename: string; delay?: number; animate?: boolean }) {
    return (
        <div className="flex gap-2 items-center">
            <FileIcon size={17} className="opacity-80" />
            <p
                className={cn(
                    "text-md truncate w-full leading-[1.1em] text-muted-foreground",
                    animate && "shimmer font-[500]"
                )}
                style={{ animationDelay: `${delay}ms` }}
            >
                {filename}
            </p>
        </div>
    )
}

const stagedFiles = [
    "src/auth/token_refresh.rs",
    "services/payment/stripe_webhook.py",
    "internal/cache/redis_pool.go",
    "db/migrations/0042_add_user_preferences.sql",
    "pkg/logger/structured_logging.rs",
]

const changesFiles = [
    "src/auth/token_refresh.rs",
    "services/payment/stripe_webhook.py",
    "internal/cache/redis_pool.go",
    "tests/auth/token_refresh_test.rs",
    "docs/api/payment_webhook.md",
]

export default function StagedFiles() {
    return (
        <div className="my-15 site-max flex gap-10 md:flex-row flex-col">

            <div className="flex-1 sticky top-[100px] md:self-start">
                <h1 className="text-4xl">How it works</h1>
                <p className="md:text-md text-muted-foreground mt-4">Lorem ipsum dolor sit amet consectetur adipisicing elit. Impedit quibusdam neque iusto nobis ipsam animi voluptate natus numquam quas, ut optio accusamus aliquid maxime perferendis dicta reprehenderit fuga pariatur minus? lorem10</p>
            </div>

            <div className="flex-1 aspect-square">
                <div className="rounded-md h-full w-full relative overflow-hidden flex flex-col flex-1">
                    <img src={Gradient.src} className="w-full h-full top-0 left-0 absolute object-cover z-[-1] " />

                    <div className="w-full h-full flex items-center p-5 md:p-15">

                        <div className="w-full h-fit rounded-md bg-background/70 p-3 px-5">

                            <p className="">Staged <ChevronDown size={17} className="inline opacity-70" /></p>
                            <div className="mt-2 space-y-2.5">
                                {stagedFiles.map((file, i) => (
                                    <FileItem key={i} filename={file} delay={i * 100} />
                                ))}
                            </div>

                            <p className="mt-5">Changes <ChevronDown size={17} className="inline opacity-70" /></p>
                            <div className="mt-2 space-y-2.5 relative">

                                <div className="blur-[4px] space-y-2 opacity-60">
                                    {changesFiles.map((file, i) => (
                                        <FileItem key={i} filename={file} delay={i * 100} animate={false} />
                                    ))}
                                </div>
                                <div className="absolute top-0 left-0 flex items-center justify-center w-full h-full">
                                    <p className="text-center text-lg font-">
                                        Untouched <LockIcon size={18} strokeWidth={2.5} className="inline mb-1 ml-1 opacity-90"/>
                                    </p>
                                </div>
                            </div>


                        </div>

                    </div>

                </div>
            </div>

        </div>
    )
}