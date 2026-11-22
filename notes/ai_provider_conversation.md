im making a cli tool that uses user's own vercel ai gatewat or google ai studio apikeys. can i list models (dynamic) and make him pick which model to use? so the pricing is his headache

Yes—both Vercel AI Gateway and Google AI Studio (Gemini API) expose endpoints to fetch models dynamically at runtime, so your CLI can present an interactive picker and pass billing/quota responsibility straight to the user's own API key.

1. Dynamic Model Listing by Provider
Vercel AI Gateway:

Endpoint: GET [https://ai-gateway.vercel.sh/v1/models](https://ai-gateway.vercel.sh/v1/models) (or gateway.getAvailableModels() in the AI SDK).

Details returned: Model IDs (creator/model-name), display names, model types (language, embedding, image, video, etc.), context windows, reasoning options, and pricing metadata.

Team/key eligibility: You can pass ?include_availability with an Authorization: Bearer <AI_GATEWAY_API_KEY> header to filter or annotate models eligible under the user's team/key setup.

Google AI Studio (Gemini API):

Endpoint: GET [https://generativelanguage.googleapis.com/v1beta/models](https://generativelanguage.googleapis.com/v1beta/models) (or via the official Google GenAI SDK's models.list method, authenticated with the user's API key).

Details returned: Model resource names (e.g., models/...), display names, supported generation methods/actions, token limits, and input/output token limits.

2. Practical CLI Design Notes
Filter out non-chat models: Both catalogs include embeddings, image/video/audio generation, speech, or specialized models. If your CLI expects text/tool/chat completions, filter the returned list before rendering the selector.

Show pricing/tier cues when available: Vercel AI Gateway's model/endpoint metadata exposes pricing/context info, which helps users avoid accidentally picking an expensive frontier or reasoning model when they want a cheap/fast one.

Cache with a TTL + --refresh flag: Fetching hundreds of models on every CLI run adds latency. Cache the list locally (e.g., ~/.config/<your-cli>/models.json for 12–24 hours) and offer a --refresh or /model command to re-fetch.

Keep keys local and scoped: Store user keys in environment variables or local user config with restricted permissions (0600), never hardcoded or logged. Even when the user brings their own key, make sure your CLI clearly displays the active model before sending requests so there are no surprises on their bill.

3. Quick Example (Node / TypeScript CLI Fetcher)
TypeScript
type PickableModel = {
  provider: 'vercel-gateway' | 'google-ai-studio';
  id: string;
  label: string;
};

export async function fetchVercelModels(apiKey?: string): Promise<PickableModel[]> {
  const url = apiKey
    ? 'https://ai-gateway.vercel.sh/v1/models?include_availability'
    : 'https://ai-gateway.vercel.sh/v1/models';

  const res = await fetch(url, {
    headers: apiKey ? { Authorization: `Bearer ${apiKey}` } : undefined,
  });
  if (!res.ok) throw new Error(`Vercel Gateway error: ${res.status}`);

  const json = await res.json();
  return (json.data ?? [])
    .filter((m: any) => m.type === 'language')
    .map((m: any) => ({
      provider: 'vercel-gateway',
      id: m.id,
      label: `${m.id} (${m.name ?? m.id})`,
    }));
}

export async function fetchGoogleModels(apiKey: string): Promise<PickableModel[]> {
  const res = await fetch('https://generativelanguage.googleapis.com/v1beta/models', {
    headers: { 'x-goog-api-key': apiKey },
  });
  if (!res.ok) throw new Error(`Google AI Studio error: ${res.status}`);

  const json = await res.json();
  return (json.models ?? [])
    .filter((m: any) => Array.isArray(m.supportedGenerationMethods)
      ? m.supportedGenerationMethods.includes('generateContent')
      : true)
    .map((m: any) => ({
      provider: 'google-ai-studio',
      id: String(m.name).replace(/^models\//, ''),
      label: m.displayName ? `${m.displayName} (${m.name})` : m.name,
    }));
}
Wire that array into an interactive prompt (@inquirer/prompts select or search) and save the chosen provider + id to the user's local config.

how to select the most free one as "recommended"

Both providers handle "free" differently, so your CLI needs a two-part strategy: numeric sorting where pricing metadata exists (Vercel), and name-based heuristic matching where it doesn't (Google AI Studio).

1. Vercel AI Gateway: Sort by Token Price + Free Tier Eligibility
Vercel's /v1/models endpoint returns a pricing object on each model (pricing.input and pricing.output as per-token costs). Additionally, every Vercel team account gets $5/month in free credits that can be spent on a specific subset of Free Tier eligible models.

Truly $0 models: Check if Number(m.pricing?.input) === 0 && Number(m.pricing?.output) === 0.

Cheapest / Free-Credit-Safe fallback: If no $0 model is returned, sort ascending by (Number(m.pricing?.input ?? Infinity) + Number(m.pricing?.output ?? Infinity)) so the user burns their $5 monthly free credit as slowly as possible.

Pin the winner to the top: Tag the first match as (Recommended - Free/Cheapest) and set it as the default option in your CLI prompt.

2. Google AI Studio: Heuristic Ranking on Model IDs
Google's v1beta/models endpoint does not return pricing or billing tier fields—it only returns metadata like name, displayName, inputTokenLimit, and supportedGenerationMethods. On Google AI Studio's free tier, Flash-Lite and Flash models are the primary free-of-charge models, while Pro/Ultra/Experimental models either have strict daily caps or require billing.

Rank dynamically returned models using priority rules on the model ID:

Tier 1 (Best Free / Highest Quota): Contains flash-lite (avoid -exp, preview, or tts/image/audio variants).

Tier 2 (Standard Free Tier): Contains flash (latest stable version first, e.g., gemini-3.8-flash or gemini-2.5-flash before older versions).

Tier 3 (Everything else): pro and preview models pushed further down.

3. Drop-In Implementation (TypeScript)
TypeScript
type PickableModel = {
  provider: 'vercel-gateway' | 'google-ai-studio';
  id: string;
  label: string;
  recommended?: boolean;
};

// 1. Vercel AI Gateway: pick $0 first, otherwise lowest combined token cost
export function rankVercelModels(rawModels: any[]): PickableModel[] {
  const chatModels = rawModels.filter((m) => m.type === 'language');

  const sorted = [...chatModels].sort((a, b) => {
    const costA = Number(a.pricing?.input ?? Infinity) + Number(a.pricing?.output ?? Infinity);
    const costB = Number(b.pricing?.input ?? Infinity) + Number(b.pricing?.output ?? Infinity);
    return costA - costB;
  });

  return sorted.map((m, idx) => {
    const inCost = Number(m.pricing?.input ?? NaN);
    const outCost = Number(m.pricing?.output ?? NaN);
    const isZeroCost = inCost === 0 && outCost === 0;
    const isRecommended = idx === 0;

    const priceTag = isZeroCost
      ? 'FREE'
      : Number.isFinite(inCost)
        ? `$${(inCost * 1_000_000).toFixed(2)}/1M in`
        : 'price unknown';

    return {
      provider: 'vercel-gateway',
      id: m.id,
      recommended: isRecommended,
      label: isRecommended
        ? `★ ${m.id} [${priceTag}] (Recommended)`
        : `  ${m.id} [${priceTag}]`,
    };
  });
}

// 2. Google AI Studio: score by free-tier likelihood (flash-lite > flash > pro)
function scoreGoogleModel(id: string): number {
  const lower = id.toLowerCase();
  // Penalize non-standard/experimental variants
  if (/(preview|exp|tts|audio|image|robotics|computer-use)/.test(lower)) return 100;
  if (lower.includes('flash-lite')) return 1; // Highest free-tier rate limits
  if (lower.includes('flash')) return 2;      // Standard free-tier workhorse
  if (lower.includes('pro')) return 3;        // Tighter free quota / paid
  return 50;
}

export function rankGoogleModels(rawModels: any[]): PickableModel[] {
  const chatModels = rawModels.filter((m) =>
    Array.isArray(m.supportedGenerationMethods)
      ? m.supportedGenerationMethods.includes('generateContent')
      : true
  );

  const sorted = [...chatModels].sort((a, b) => {
    const idA = String(a.name).replace(/^models\//, '');
    const idB = String(b.name).replace(/^models\//, '');
    const scoreDiff = scoreGoogleModel(idA) - scoreGoogleModel(idB);
    if (scoreDiff !== 0) return scoreDiff;
    // Reverse alphabetical puts newer version numbers (e.g., 3.x / 2.5) above older ones
    return idB.localeCompare(idA);
  });

  return sorted.map((m, idx) => {
    const id = String(m.name).replace(/^models\//, '');
    const isRecommended = idx === 0;
    return {
      provider: 'google-ai-studio',
      id,
      recommended: isRecommended,
      label: isRecommended
        ? `★ ${id} [Free Tier Friendly] (Recommended)`
        : `  ${id}`,
    };
  });
}
Edge Case to Handle in Your CLI
Even on free tiers, both providers throw HTTP 429 (Rate Limit / Quota Exceeded) when a user hits requests-per-minute limits or runs out of free credits. If your CLI catches a 429 on the recommended model, prompt them immediately to switch models rather than crashing with a raw stack trace.

can i get a selected model's specs like context window limits or things?

Yes—both providers return detailed model specs either directly inside the list response or via a single-model lookup endpoint, so you don't need to hardcode context windows or capability flags in your CLI.

1. What Specs You Get Back
Spec	Vercel AI Gateway (/v1/models & /endpoints)	Google AI Studio (/v1beta/models)
Context Window	context_window (and per-provider context_length on /endpoints)	inputTokenLimit
Max Output Tokens	max_tokens (and max_completion_tokens on /endpoints)	outputTokenLimit
Capabilities / Tags	tags (reasoning, tool-use, vision, etc.) & modalities	supportedGenerationMethods (e.g. generateContent, countTokens)
Reasoning Controls	reasoning_options (effort, budget_tokens, toggle)	Exposed via model family / thinking config
Pricing & Cache	pricing.input, pricing.output, inputTiers, cachedInputTokens	Not included in API metadata
Default Sampling	supported_parameters (via /endpoints)	temperature, maxTemperature, topP, topK
2. Fetching Specs for a Selected Model
You already get most of these fields during your initial models list call—just save them into your local cache object when fetching the list. If you want to inspect a single model on demand (e.g., my-cli model info <id>):

Vercel AI Gateway
Base model object (from GET [https://ai-gateway.vercel.sh/v1/models](https://ai-gateway.vercel.sh/v1/models)): gives context_window, max_tokens, tags, modalities, reasoning_options, and pricing.

Per-provider routing specs (GET [https://ai-gateway.vercel.sh/v1/models/](https://ai-gateway.vercel.sh/v1/models/){creator}/{model}/endpoints): returns every upstream provider serving that model along with context_length, max_completion_tokens, supported_parameters, supports_implicit_caching, uptime_last_15m, and per-provider pricing.

Google AI Studio (Gemini API)
Single model endpoint: GET [https://generativelanguage.googleapis.com/v1beta/models/](https://generativelanguage.googleapis.com/v1beta/models/){model_id} (or ai.models.get({ model }) in @google/genai / client.models.get(model=...) in Python).

Returns inputTokenLimit, outputTokenLimit, supportedGenerationMethods, temperature, topP, topK, and version.

3. Unified Normalizer for Your CLI
Storing a normalized spec object in your local config lets your CLI enforce token budgets, warn users before sending oversized files, or hide --thinking / --vision flags when unsupported:

TypeScript
export type ModelSpecs = {
  provider: 'vercel-gateway' | 'google-ai-studio';
  id: string;
  name: string;
  contextWindow: number;
  maxOutputTokens: number;
  capabilities: {
    vision: boolean;
    tools: boolean;
    reasoning: boolean;
  };
  pricing?: {
    inputPerMillion: number;
    outputPerMillion: number;
  };
};

// Normalize from Vercel /v1/models item
export function normalizeVercelModel(m: any): ModelSpecs {
  const tags: string[] = m.tags ?? [];
  const inputModalities: string[] = m.modalities?.input ?? [];

  return {
    provider: 'vercel-gateway',
    id: m.id,
    name: m.name ?? m.id,
    contextWindow: Number(m.context_window ?? 0),
    maxOutputTokens: Number(m.max_tokens ?? 0),
    capabilities: {
      vision: tags.includes('vision') || inputModalities.includes('image'),
      tools: tags.includes('tool-use'),
      reasoning: tags.includes('reasoning') || Array.isArray(m.reasoning_options),
    },
    pricing: m.pricing
      ? {
          inputPerMillion: Number(m.pricing.input ?? 0) * 1_000_000,
          outputPerMillion: Number(m.pricing.output ?? 0) * 1_000_000,
        }
      : undefined,
  };
}

// Normalize from Google /v1beta/models item
export function normalizeGoogleModel(m: any): ModelSpecs {
  const id = String(m.name).replace(/^models\//, '');
  const lower = id.toLowerCase();

  return {
    provider: 'google-ai-studio',
    id,
    name: m.displayName ?? id,
    contextWindow: Number(m.inputTokenLimit ?? 0),
    maxOutputTokens: Number(m.outputTokenLimit ?? 0),
    capabilities: {
      // Gemini 2.5+ / 3.x / 4.x text models natively support multimodal input & tools
      vision: !lower.includes('gemma'),
      tools: !lower.includes('gemma'),
      reasoning: /(gemini-2\.5|gemini-3|gemini-4)/.test(lower) && !lower.includes('lite'),
    },
  };
}
Practical CLI Uses for These Specs
Context guardrails: Before piping a large file or git diff into the prompt, estimate tokens (~chars / 4) and warn if it exceeds contextWindow - maxOutputTokens.

Picker UI subtitles: Display 1M ctx · 65K out · reasoning right next to the model name in your terminal selector so users can make an informed choice without opening docs.