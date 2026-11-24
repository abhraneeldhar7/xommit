import { useState } from "react";

type InstallOption = {
	id: string;
	label: string;
	prompt: string;
};

const OPTIONS: InstallOption[] = [
	{
		id: "pnpm",
		label: "pnpm",
		prompt: "pnpm add -g @antkin/xommit",
	},
	{
		id: "npm",
		label: "npm",
		prompt: "npm install -g @antkin/xommit",
	},
	{
		id: "agents",
		label: "agents",
		prompt: `Install "@antkin/xommit" globally using npm/pnpm/bun or whatever suitable package manager.
[explain all the supported ways and platforms]
[list supported ai providers]
Ask the user for their [ai provider] keys before running "xommit --connect".
Run "xommit --help" for full tutorial.
If any instruction file (agent.md, AGENTS.md, etc) exists, add a rule to use xommit for committing staged files.`,
	},
];

export default function Hero() {
	const [activeId, setActiveId] = useState(OPTIONS[0].id);
	const [copied, setCopied] = useState(false);

	const active = OPTIONS.find((option) => option.id === activeId) ?? OPTIONS[0];

	async function copyPrompt() {
		try {
			await navigator.clipboard.writeText(active.prompt);
		} catch {
			return;
		}

		setCopied(true);
		window.setTimeout(() => setCopied(false), 1500);
	}

	function selectOption(id: string) {
		setActiveId(id);
		setCopied(false);
	}

	return (
		<section aria-labelledby="hero-title">
			<h1
				id="hero-title"
				className="text-4xl font-medium text-balance text-foreground sm:text-6xl"
			>
				Professional commits for larping tech bros
			</h1>

			<div
				className="mt-8 inline-flex items-center gap-1 rounded-lg border border-border bg-card/40 p-1"
				role="tablist"
				aria-label="Install options"
			>
				{OPTIONS.map((option) => (
					<button
						key={option.id}
						type="button"
						role="tab"
						id={`install-tab-${option.id}`}
						aria-selected={option.id === active.id}
						aria-controls="install-prompt"
						onClick={() => selectOption(option.id)}
						className="cursor-pointer rounded-md px-3 py-1.5 text-sm text-muted-foreground transition-colors hover:text-foreground aria-selected:bg-muted aria-selected:text-foreground"
					>
						{option.label}
					</button>
				))}
			</div>

			<div
				id="install-prompt"
				role="tabpanel"
				aria-labelledby={`install-tab-${active.id}`}
				className="mt-4 flex items-start justify-between gap-4 rounded-xl border border-border bg-card/40 px-4 py-3.5"
			>
				<pre className="min-w-0 flex-1 whitespace-pre-wrap font-sans text-base text-foreground/85">
					{active.prompt}
				</pre>

				<button
					type="button"
					onClick={copyPrompt}
					aria-label="Copy prompt"
					className="inline-flex shrink-0 cursor-pointer items-center gap-1.5 rounded-md border border-border px-2 py-1 text-xs text-muted-foreground transition-colors hover:bg-muted hover:text-foreground"
				>
					{copied ? (
						<svg
							width="14"
							height="14"
							viewBox="0 0 24 24"
							fill="none"
							stroke="currentColor"
							strokeWidth="2"
							strokeLinecap="round"
							strokeLinejoin="round"
							aria-hidden="true"
						>
							<path d="M20 6 9 17l-5-5" />
						</svg>
					) : (
						<svg
							width="14"
							height="14"
							viewBox="0 0 24 24"
							fill="none"
							stroke="currentColor"
							strokeWidth="2"
							strokeLinecap="round"
							strokeLinejoin="round"
							aria-hidden="true"
						>
							<rect x="9" y="9" width="13" height="13" rx="2" />
							<path d="M5 15H4a2 2 0 0 1-2-2V4a2 2 0 0 1 2-2h9a2 2 0 0 1 2 2v1" />
						</svg>
					)}
					<span aria-live="polite">{copied ? "Copied" : "Copy"}</span>
				</button>
			</div>
		</section>
	);
}
