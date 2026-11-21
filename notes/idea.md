
# Xommit | Conventional commits tool
> A cli tool which turns vague messages into conventional commits and calls git commit -m "" for us.
> make in /cli folder. this is a c++ tool.

## Flow is:
1. user writes "xommit arg arg arg arg" in any terminal (problem 1: proper install so name can be recognized).
if the first arg starts with double "-" that means this is a params call, not a plain commit message. any "-" other than double beginning position means its plain text commit message. This is as cut and dry as it needs to be, no complex shit.
available options below:
	--help: show tutorial
	--connect : connect to user's own ai token provider (google ai studio, vercel ai gateway) (explained in AI provider section)
	--reset : remove user creds
	--update : download latest version (problem 2: where should this download from? npm or github? how should the update be? should it have same flow as first install?)
	--version : show xommit version (problem 3: how to properly maintain version through the CICD which reflects here as well as in npm store)

2. if its an option param then execute their respective flows.
3. if its a plain message then: append all args into a string -> form the proper prompt (system prompt + user's vague message) + ready the git diff files (problem 4: explained later) + output schema -> user's selected ai provider 
4. accept the response and display the 3 message options received, let user select (UI layer) the option -> run git commit -m "message" and show the normal post-commit output like the file names etc whatever it shows (i dont think we need to hardcode this thing, just put git's statements on display). dont touch any branch commands.

---

## UI layer:
- show the logo from salvage\terminal_ui.h line 43 while this tool is active (generating ai response to option selection. after option is selected then remove this logo, only show git's own statements)
- use the special character to delete previous line, make this a proper function so we can just specify the number of lines (clean code) and wipe it.
- when the ai response is accepted, show the 3 options and let user use his arrow up and down keys to select the favourite one. esc to cancel this flow. enter to confirm selection.
- use the special formatting to color the text on the terminal (green for currently selected one).
- when user uses arrow up or down, use the function to clear prev 3 lines and print all them again with the new selected one as green.
- while the response is generating, show loader using dashes and "thinking" message. use the cleaning funciton to wipe prev line and show the next frame of the loader. this better be a clean abstraction.

---

## Publishing:
Whole publishing flow is a real bitch (problem 5: need proper publishing cicd to github and npm store and write publishing.md teaching me all that). don't worry about mac or linux releases now, only windows.

---

## problem 4: Feeding the context to AI.
- Estimate the chars limit using the model's spec we saved earlier.
- run git diff and properly structure the file's contents like the file's name as <full file path> plus minus diffs </full file path>.
- based on out context limit try to spread out the inclusiveness to as many file's git diffs as possible.


---

## AI provider:

- abstract and generalize out the vercel ai gateway, google ai studio thing so both behave the same and I can add more in future. use proper principles to use the providers.
- while selecting the models in --connect flow, list all the models first and show "recommended" on the free ones (like gemini flash 3.5 lite, some good ones are in vercel too.), maybe dial in other params too while selecting a good "recommended" model.
- after a model is selected save the model name, its specs like context window which is important to estimate the chars limit on problem 4. save this well structured in where user's provider choise, apikey is stored.
- make selection ui an independent UI component so it can be used in selecting response as well as here (or wherever needed) too. like a simple selection UI which dynamically clears prev X lines based on what options you fed it.
- read ai_provider_conversation.md this is a very helpful conversation with another agent.


---

## your prev conversation answers:
- "npm creates the xommit.cmd shim automatically" im very unfamiliar with this whole shim thing and installing with npm how these things work. i dont want an answer to say during interview, i want to learn this thing in depth how things are happening in core.
- "What you did not mention": yes only staged files to consider.
"empty diff" then say the thing git commit says and end.
"Binary and lockfile diffs" ok then we do need to ignore stuff, have our own gitignore which sits after user's gitignore.
"A running exe cannot overwrite itself. The rename trick is required." yes good problem, need proper solution.
"Provider errors" ok just display these then happens then end.
"Prompt content" figure out a  good system prompt then I will iterate over it.
"Code signing" in which flow will this problem occur?
"Tests" ok when this thing is fully built then ask me i will tell you.
yes --connect overwrites the saved provider and --reset resets the whole config.
no env var should not work without user setting it up himself.
btw user can just say xommit and leave it then we will do the whole thing without user's vague message.
ignore webapp.
"60 second timeout." wait what after where when?
"Defaults I will use unless you object" most correct.
"No code signing." no wait whats that?
"Non-TTY stdin gets a numbered prompt instead of arrow keys." i dont understand what you mean.
other things look fine.
"Three separate install" ok like publishing.md also explain these separately. move all md files in a notes/ folder for better cleanliness.
"I checked the old json.h and http_client.h in git history." cool you are free to reuse the good parts i dont have nothing against the old one I just need clean better structuring.
"One retry on network or 5xx, none on 401 or 429." sure but in code make sure to comment and explain what those errors mean so its more human readable.
"context_window - 512 output" you need to limit that? ok sure i think its enough but is it necesary?