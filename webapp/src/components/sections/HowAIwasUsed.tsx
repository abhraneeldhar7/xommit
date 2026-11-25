export default function HowAiWasUsed() {
    return (<div className="my-30 site-max">
        <h1 className="text-4xl">How AI was used in this project</h1>
        <p className="md:text-md text-muted-foreground mt-4 leading-[1.8em]">I had a simple idea of feeding llms the git diffs and conventions so I can have consistent commit messages every time without looking up the rules or messing up. But a pure JavaScript cli would've costed so much more storage, so why not use js for the npm shim and keep the build as lightweight as possible, my best thought was cpp. So I fed in my whole loigc in idea.md and used my agents to setup a skeleton project, on which I iterated over to perfect it. Use of cpp was inspired by the black magic <a className="underline text-foreground" href="https://x.com/RubenVeidt" target="_blank">RubenVeidt</a> does and his 5mb build of photoshop.</p>
    </div>)
}