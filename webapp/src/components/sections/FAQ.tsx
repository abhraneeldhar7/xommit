type FAQItemProps = {
    question: string
    answer: string
}

function FAQItem({ question, answer }: FAQItemProps) {
    return (
        <div className="p-4.5 border border-transparent border-r-border border-b-border transition-all duration-fast hover:border-border hover:bg-muted/50 group/item group-hover/faq:opacity-60 group-hover/faq:hover:opacity-100">
            <h2 className="text-3xl transition-colors duration-fast group-hover/faq:text-muted-foreground group-hover/faq:group-hover/item:text-foreground">{question}</h2>
            <p className="text-muted-foreground mt-2 transition-colors duration-fast group-hover/item:text-foreground/90 leading-[1.8em]">{answer}</p>
        </div>
    )
}

const faqs: FAQItemProps[] = [
    {
        question: "Do I need to pay anyone?",
        answer: "No. xommit has no license fee, no signup, and no telemetry. You bring a free API key from the provider you choose and skip your own credits. Google AI Studio has a free tier that covers normal use, and Vercel AI Gateway lists free models first when it ranks them.",
    },
    {
        question: "Which providers can I use?",
        answer: "Google AI Studio and Vercel AI Gateway. Google is the recommended pick because the free tier is generous. Run xommit --connect once, choose the provider, paste your key, and pick from the model list it fetches. xommit --model changes the model later.",
    },
    {
        question: "Will it commit files I did not stage?",
        answer: "No. xommit only reads git diff --cached and only runs git commit -m with the message you choose. It never runs git add, never resets the index, and never commits anything you did not stage yourself.",
    },
    {
        question: "Where is my API key stored?",
        answer: "In %APPDATA%\\xommit\\config.json, as plain text on your machine. It is never sent anywhere except the provider's own API, and it never leaves your computer otherwise. Run xommit --reset to delete it.",
    },
    {
        question: "Can I steer the message I want?",
        answer: "Yes. Anything you type after xommit becomes a hint: xommit \"fix login redirect\". The hint guides the model toward the area you care about, but if your words and the diff disagree, the diff wins. It never invents a message for a change that is not there.",
    },
    {
        question: "Does it work with my agent or AI assistant?",
        answer: "Yes, which is the whole point. Install it, connect once, then tell your agent to use xommit for committing staged files. The agent stages the work, runs xommit, and takes the commit message off your hands. Add that rule once to AGENTS.md, CLAUDE.md, or whatever file your agent reads.",
    },
    {
        question: "Can I use it offline?",
        answer: "Not for generating messages, since the message comes from the provider's API. Without a network, xommit reports it cannot reach the model and exits without committing. Your staged changes stay exactly as they were.",
    },
    {
        question: "What about huge diffs or lockfiles?",
        answer: "Both are handled. Common lockfiles, build output, dependency folders, binary assets, and minified files are ignored by default, and you can add your own to .xommitignore. If a diff is still too large for the model's context window, xommit trims it down to fit, and ignored files are still committed with the rest of the index.",
    },
    {
        question: "Does it work on any repo?",
        answer: "Any git repository works. xommit shells out to your own git install and reads the staged diff, so there is nothing to configure per project. Put a .xommitignore at the root only if you want to hide specific paths from the model.",
    },
]

export default function FAQ() {
    return (<div className="my-30 site-max flex flex-col gap-7">
        <h1 className="text-4xl">FAQs</h1>

        <div className="border-t border-l w-full grid md:grid-cols-3 grid-cols-1 group/faq">
            {faqs.map((faq) => (
                <FAQItem key={faq.question} {...faq} />
            ))}
        </div>
    </div>)
}
