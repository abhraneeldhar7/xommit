import { cn, coptToClipboard } from "@/lib/utils";
import { useState } from "react";
import { icons } from "@/lib/icons";
import VscodeImg from "../../assets/vscode.png"
import GradientImg from "../../assets/gradient.jpg"
import XommitLogo from "../XommitLogo";

type InstallOption = {
	label: string;
	prompt: string;
};

const InstallCommands: InstallOption[] = [
	{
		label: "pnpm",
		prompt: "pnpm add -g @antkin/xommit",
	},
	{
		label: "npm",
		prompt: "npm install -g @antkin/xommit",
	},
	{
		label: "bun",
		prompt: "bun install -g @antkin/xommit",
	},
	{
		label: "agents",
		prompt: `Install "@antkin/xommit" globally using npm, pnpm, bun, or your preferred package manager:
- npm: npm install -g @antkin/xommit
- pnpm: pnpm add -g @antkin/xommit
- bun: bun add -g @antkin/xommit

Supported AI providers: Google AI Studio, Vercel AI SDK.

Ask the user for their API keys for the chosen provider, then run "xommit --connect" to configure authentication.

Run "xommit --help" for full tutorial and usage examples.

If any instruction file (AGENTS.md, agent.md, CLAUDE.md, .cursorrules, etc.) exists in the project, add a rule to use xommit for committing staged files automatically.`,
	},
];

export default function Hero() {



	function InstallCopy() {
		const [active, setActive] = useState(InstallCommands[0].label);
		const [copied, setCopied] = useState(false);

		const activeOption = InstallCommands.find((option) => option.label === active) ?? InstallCommands[0];


		return (<div className="mt-6 flex flex-col gap-2">

			<div className="flex gap-1">
				{InstallCommands.map((option, index) => (
					<button key={index} className={cn("border-1 border-transparent text-sm rounded-sm hover:bg-muted px-2 py-1 transition-all duration-fast", option.label == active && "bg-muted border-border/70")} onClick={() => (setActive(option.label))}>{option.label}</button>
				))}
			</div>


			<div className="border rounded-md w-fit p-[3px] flex gap-2 items-center">
				<p className="truncate md:w-70 w-full leading-[1.1em] px-3"><span className="text-sm text-destructive mr-2">{activeOption.label == "agents" ? ">" : "$"}</span>{activeOption.prompt}</p>
				<button className={cn("bg-destructive px-3 py-2 rounded-sm transition-all duration-fast hover:bg-destructive/70 w-15 flex items-center justify-center relative overflow-hidden", copied && "hover:bg-destructive")} onClick={() => {
					coptToClipboard(activeOption.prompt)
					setCopied(true);
					window.setTimeout(() => setCopied(false), 1000);
				}}>
					<div className={cn("flex items-center justify-center w-full transition-all duration-fast ease-out", copied && "translate-y-[-100%] opacity-0", !copied && "translate-y-0 opacity-100")}>
						Copy
					</div>
					<div className={cn("absolute inset-0 flex items-center justify-center transition-all duration-fast ease-out", !copied && "translate-y-[100%] opacity-0", copied && "translate-y-0 opacity-100")}>
						<icons.check className="h-4.5 w-4.5" strokeWidth={2} />
					</div>
				</button>
			</div>

		</div>)
	}

	return (
		<div className="site-max pb-30">

			<h1 className="text-3xl md:text-5xl lg:text-6xl leading-[1.25em]">Cringe Commits <br className="lg:hidden" />for techbros</h1>

			<InstallCopy />

			<div className="my-2">
				<a href="#other-install" className="text-xs transition-all duration-fast text-muted-foreground hover:text-foreground hover:underline">Other ways to install <icons.info className="inline size-[12px]" strokeWidth={2} /></a>
			</div>

			<div className="mt-4 relative overflow-hidden w-full p-6">
				<img src={GradientImg.src} className="object-cover h-full w-full absolute inset-0 z-[-1]" />

				<div className="md:absolute top-0 left-0 p-6 w-[60%] z-[2]">

					<div className="bg-[#101010] h-100 w-full flex flex-col rounded-xs">
						<div className="w-full h-7 flex justify-end items-center">
							<div className="w-fit h-full p-[6px] px-[8px] hover:bg-muted flex items-center transition-all duration-fast"><icons.Minus className="h-3 w-3" /></div>
							<div className="w-fit h-full p-[6px] px-[8px] hover:bg-muted flex items-center transition-all duration-fast"><icons.Square className="h-3 w-3" /></div>
							<div className="w-fit h-full p-[6px] px-[8px] hover:bg-[red] flex items-center transition-all duration-fast"><icons.Close className="h-4 w-4" /></div>
						</div>
						<div className="p-3 px-5">
							<XommitLogo className="text-[8px] sm:text-[10px] leading-[1em]" />
						</div>

					</div>
				</div>

				<div className="h-fit w-[80%] overflow-hidden ml-auto rounded-lg mt-5">
					<img src={VscodeImg.src} className="w-full h-fit object-contain overflow-hidden blur-[3px]" />
				</div>
			</div>

		</div>
	);
}
