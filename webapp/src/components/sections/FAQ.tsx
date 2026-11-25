type FAQItemProps = {
    question: string
    answer: string
}

function FAQItem({ question, answer }: FAQItemProps) {
    return (
        <div className="p-4.5 border-r border-b transition-all duration-fast hover:bg-muted/50 group/item group-hover/faq:opacity-60 group-hover/faq:hover:opacity-100">
            <h2 className="text-3xl transition-colors duration-fast group-hover/faq:text-muted-foreground group-hover/faq:group-hover/item:text-foreground">{question}</h2>
            <p className="text-muted-foreground mt-2 transition-colors duration-fast group-hover/item:text-foreground/90">{answer}</p>
        </div>
    )
}

const faqs: FAQItemProps[] = [
    {
        question: "What is xommit?",
        answer: "Lorem ipsum dolor sit amet consectetur adipisicing elit. Itaque earum animi reprehenderit ullam distinctio nobis repudiandae maiores molestiae nihil deserunt!",
    },
    {
        question: "How does it work?",
        answer: "Lorem ipsum dolor sit amet consectetur adipisicing elit. Itaque earum animi reprehenderit ullam distinctio nobis repudiandae maiores molestiae nihil deserunt!",
    },
    {
        question: "Which providers are supported?",
        answer: "Lorem ipsum dolor sit amet consectetur adipisicing elit. Itaque earum animi reprehenderit ullam distinctio nobis repudiandae maiores molestiae nihil deserunt!",
    },
    {
        question: "Is my API key stored?",
        answer: "Lorem ipsum dolor sit amet consectetur adipisicing elit. Itaque earum animi reprehenderit ullam distinctio nobis repudiandae maiores molestiae nihil deserunt!",
    },
    {
        question: "Can I use it offline?",
        answer: "Lorem ipsum dolor sit amet consectetur adipisicing elit. Itaque earum animi reprehenderit ullam distinctio nobis repudiandae maiores molestiae nihil deserunt!",
    },
    {
        question: "Does it work with any repo?",
        answer: "Lorem ipsum dolor sit amet consectetur adipisicing elit. Itaque earum animi reprehenderit ullam distinctio nobis repudiandae maiores molestiae nihil deserunt!",
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
