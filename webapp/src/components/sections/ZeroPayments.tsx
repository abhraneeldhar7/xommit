import Gradient from "../../assets/gradient.jpg"
import TerminalConnect from "@/components/TerminalConnect.tsx"


export default function ZeroPayments() {
    return (<div className="my-30 site-max flex gap-10 md:flex-row-reverse flex-col">

        <div className="flex-1 sticky top-[100px] md:self-start text-right">
            <h1 className="text-4xl">Zero Payment</h1>
            <p className="md:text-md text-muted-foreground mt-4 leading-[1.8em]">Chances are you have free credits sitting in a provider you already signed up for, and a commit message costs almost nothing to generate. Point xommit at those credits and let it handle committing for you. No monthly fee, nothing to pay me, and one less thing on your mind.</p>
        </div>

        <div className="flex-1">
            <div className="rounded-md h-full w-full relative overflow-hidden">
                <img src={Gradient.src} className="w-full h-full top-0 left-0 absolute object-cover z-[-1]" />

                <div className="h-full w-full p-5 md:p-10 flex items-center">
                    <TerminalConnect />
                </div>

            </div>
        </div>

    </div>)
}