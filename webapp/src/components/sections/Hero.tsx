import { cn } from "@/lib/utils";
import { useState } from "react";
import { icons } from "@/lib/icons";
import GradientImg from "../../assets/gradient.webp"
import VscodeImg from "../../assets/vscode.webp"
import WindowsLogoImg from "../../assets/windows_logo.svg"
import LinuxLogoImg from "../../assets/linux_logo.svg"

import CopyCommand from "../CopyCommand";
import TerminalDemo from "../TerminalDemo";

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

Ask the user for their API keys for the chosen provider, then run "xommit --connect" to configure.

Run "xommit --help" for full tutorial.

If any instruction file (AGENTS.md, agent.md, CLAUDE.md, .cursorrules, etc.) exists in the project, add a rule to use xommit for committing staged files automatically at proper phases.`,
	},
];

export default function Hero({ version }: { version: string }) {



	function InstallCopy() {
		const [active, setActive] = useState(InstallCommands[0].label);

		const activeOption = InstallCommands.find((option) => option.label === active) ?? InstallCommands[0];


		return (<div className="mt-6 flex flex-col gap-2">

			<div className="flex gap-1">
				{InstallCommands.map((option, index) => (
					<button key={index} className={cn("text-sm rounded-sm text-foreground/60 hover:bg-muted/50 px-2 py-1 transition-all duration-fast", option.label == active && "bg-muted/90 text-foreground")} onClick={() => (setActive(option.label))}>{option.label}</button>
				))}
			</div>


			<div className="w-full flex md:w-90">
				<CopyCommand
					fill
					command={activeOption.prompt}
					prefix={activeOption.label == "agents" ? ">" : "$"}
				/>
			</div>

		</div>)
	}

	return (
		<section aria-labelledby="hero-heading" className="site-max">

			<p className="text-xs text-foreground/80">
				xommit
				<span id="version-pill" className="text-[0.7rem] rounded-sm bg-foreground/40 px-2 py-1 leading-[1em] w-fit ml-2">
					v{version}
				</span>
			</p>

			<h1 id="hero-heading" className="mt-2 text-[2.7rem] leading-[1.25em] md:text-5xl lg:text-6xl leading-[1.25em]">Cringe Commits <br className="lg:hidden" />for tech bros</h1>

			<InstallCopy />

			<div className="my-2">
				<a href="#other-install" className="text-xs transition-all duration-fast text-muted-foreground/70 hover:text-foreground hover:underline"><icons.info className="inline size-[14px]" strokeWidth={2} /> Other ways to install </a>
			</div>

			<div className="mt-4 relative overflow-hidden w-full p-5 rounded-md">
				<img src={GradientImg.src} alt="" width={GradientImg.width} height={GradientImg.height} className="object-cover h-full w-full absolute inset-0 z-[-1]" fetchPriority="high" decoding="sync" loading="eager" />

				<div className="md:absolute top-0 left-0 md:p-6 w-full md:w-[60%] z-[2]">
					<TerminalDemo />
				</div>

				<div className="h-fit w-[80%] overflow-hidden ml-auto rounded-lg mt-5 hidden md:flex">
					<img src={VscodeImg.src} alt="xommit running in a terminal next to a code editor" width={VscodeImg.width} height={VscodeImg.height} className="w-full h-fit object-contain overflow-hidden blur-[3px]" loading="lazy" fetchPriority="high" decoding="async" />
				</div>


				{/* <div className="absolute z-[3] bottom-0 left-0 w-full h-[50%] bg-linear-to-t from-[var(--background)] from-0% to-transparent to-100%" /> */}
			</div>


			<p className="text-md flex items-center leading-[1em] w-full my-15 justify-center gap-5">
				Supported on
				<img src={WindowsLogoImg.src} alt="Windows" width={WindowsLogoImg.width} height={WindowsLogoImg.height} className="inline aspect-square h-[20px] w-[20px] object-contain grayscale transition-all duration-default hover:grayscale-0" loading="lazy" decoding="async" />
				<img src={LinuxLogoImg.src} alt="Linux" width={LinuxLogoImg.width} height={LinuxLogoImg.height} className="inline aspect-square h-[20px] w-[20px] object-contain grayscale transition-all duration-default hover:grayscale-0" loading="lazy" decoding="async" />
			</p>


		</section>
	);
}
