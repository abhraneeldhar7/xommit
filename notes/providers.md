# Providers

Two providers are supported. Both use a plain API key and a saved model. The key is stored in `%APPDATA%\xommit\config.json`:

```json
{
  "provider": "google",
  "api_key": "...",
  "model": {
    "id": "gemini-2.5-flash",
    "name": "Gemini 2.5 Flash",
    "context_window": 1048576,
    "max_output_tokens": 8192,
    "free": true
  }
}
```

The file is written by `xommit --connect` and deleted by `xommit --reset`. There is no environment variable fallback. If the file is missing or incomplete, the commit flow stops and asks for `--connect`.

## Google AI Studio

Recommended in the picker because the free tier covers normal use.

- Base URL: `https://generativelanguage.googleapis.com/v1beta`
- Auth header: `x-goog-api-key: <key>`

List models: `GET /models`

A model is kept when:

- `supportedGenerationMethods` contains `generateContent`, when the field is present.
- The id does not contain `embedding` or `aqa`.

The `models/` prefix is stripped from the id. `inputTokenLimit` becomes `context_window`, `outputTokenLimit` becomes `max_output_tokens`.

Generate: `POST /models/<id>:generateContent`

```json
{
  "systemInstruction": { "parts": [{ "text": "..." }] },
  "contents": [
    { "role": "user", "parts": [{ "text": "..." }] }
  ],
  "generationConfig": {
    "temperature": 0.2,
    "maxOutputTokens": 512
  }
}
```

Text is read from `candidates[0].content.parts[].text`.

## Vercel AI Gateway

- Base URL: `https://ai-gateway.vercel.sh/v1`
- Auth header: `Authorization: Bearer <key>`

List models: `GET /models`

A model is kept when:

- `type` is `language`.
- The output modalities include `text`, when the field is present.

`context_window` and `max_tokens` come straight from the response. Prices come from `pricing.input` and `pricing.output`. A model is free when both are zero.

Generate: `POST /chat/completions`

```json
{
  "model": "<id>",
  "messages": [
    { "role": "system", "content": "..." },
    { "role": "user", "content": "..." }
  ],
  "temperature": 0.2,
  "max_tokens": 512
}
```

Text is read from `choices[0].message.content`, which may be a string or an array of parts.

## Ranking

Google models are scored:

1. `flash-lite`
2. `flash`
3. `pro`
4. everything else
5. preview, experimental, tts, audio, image, robotics, computer-use models last

Ties break on larger context. Flash-lite and flash are marked free.

Vercel models are sorted by free first, then lowest combined input and output price, then larger context.

The first model in the list is marked Recommended in the picker.

## Output limit

Each request asks for 512 output tokens. If the model reports a lower maximum, that lower value is used instead. The same number is reserved when the diff context is budgeted.

## Errors

Provider failures throw `ProviderError` with the HTTP status.

| Status | Message | Retry |
| --- | --- | --- |
| 401 | API key rejected, run `--connect` | no |
| 403 | access denied, body included | no |
| 429 | rate limit or quota exceeded, body included | no |
| 5xx | provider server error, body included | once |
| other 4xx | provider error, body included | no |

Network failures from WinHTTP are retried once as well. A retry is a second identical request, not a different model. If the second attempt fails, the error is printed and the process ends. Response bodies are cut to 500 characters in error messages.

## Timeouts

WinHTTP is configured with:

- resolve: 10 seconds
- connect: 10 seconds
- send: 30 seconds
- receive: 60 seconds

A timeout surfaces as a WinHTTP error, which counts as a network failure and is retried once.
