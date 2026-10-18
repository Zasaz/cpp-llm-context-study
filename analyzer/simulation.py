import markdown2
import re
from gemini_integration.client import call_gemini_api

STATE_CHANGE_PATTERN = re.compile(r"\[STATE:\s*(.*?)\s*=\s*\"(.*?)\"\s*->\s*\"(.*?)\"\]")
TITLE_PATTERN = re.compile(r"\[TITLE:\s*(.*?)\]")

def simulate_logic(code_content, focus_filename=None):
    step_separator = "---SIMULATION_STEP_BREAK---"
    
    focus_instruction = ""
    if focus_filename:
        focus_instruction = f"In your narrative, pay special attention to events originating from or affecting the file `{focus_filename}`."

    prompt = f"""
    You are a game logic simulation assistant.
    Analyze the following C++ code, which might represent game logic from multiple files.
    Describe a conceptual "dry run" or simulation of what would happen if this code executes.
    {focus_instruction}

    Please break down your simulation into distinct logical steps or events.
    For EACH step:
    1.  Provide a very brief title for the step using the exact format: `[TITLE: Your Brief Step Title Here]`
    2.  Then, provide a clear narrative description of what happens in this step. Explain *why* actions occur based on the code logic, and describe the *consequences* or *results* of these actions or decisions. Use phrasing that indicates cause and effect (e.g., "Because [condition], the code will now... which results in..."). Make it clear what triggers the events you describe. Use Markdown for emphasis.
    3.  If this step involves a conceptual change to any important variables or game state elements, explicitly list them *after* the narrative for that step, each on a new line, using the exact format:
        `[STATE: variable_name = "old conceptual value" -> "new conceptual value"]`
        (Example: [STATE: PlayerHealth = "Full" -> "Damaged"])
    4.  Finally, use the exact separator `{step_separator}` after the complete content (title, narrative, and all state tags) for that step.

    Avoid any introductory or concluding paragraphs that are not part of a step.
    Start directly with the first step's title.

    C++ Code:
    ```cpp
    {code_content}
    ```
    Provide the simulation steps now.
    """
    try:
        full_simulation_text = call_gemini_api(prompt)
        
        cleaned_full_text = full_simulation_text.strip()
        if cleaned_full_text.startswith("```markdown"):
            cleaned_full_text = cleaned_full_text[len("```markdown"):].strip()
        if cleaned_full_text.endswith("```"):
            cleaned_full_text = cleaned_full_text[:-len("```")].strip()

        raw_steps_text = cleaned_full_text.split(step_separator)
        
        processed_steps = []
        for i, step_text in enumerate(raw_steps_text):
            step_text_trimmed = step_text.strip()
            if not step_text_trimmed:
                continue

            step_title = f"Step {i+1}" 
            narrative_md = step_text_trimmed
            state_changes = []

            title_match = TITLE_PATTERN.search(narrative_md)
            if title_match:
                step_title = title_match.group(1).strip()
                narrative_md = TITLE_PATTERN.sub("", narrative_md, count=1).strip()
            
            current_narrative_parts = []
            last_match_end = 0
            for match in STATE_CHANGE_PATTERN.finditer(narrative_md):
                current_narrative_parts.append(narrative_md[last_match_end:match.start()])
                last_match_end = match.end()
                state_changes.append({
                    "variable": match.group(1).strip(),
                    "from_value": match.group(2).strip(),
                    "to_value": match.group(3).strip()
                })
            current_narrative_parts.append(narrative_md[last_match_end:])
            
            final_narrative_md = "".join(current_narrative_parts).strip()
            
            patterns_to_remove = [ r"^\s*#+\s*Step.*?\n" ]
            for pattern in patterns_to_remove:
                final_narrative_md = re.sub(pattern, "", final_narrative_md, flags=re.IGNORECASE | re.MULTILINE).strip()

            narrative_html = markdown2.markdown(
                final_narrative_md, 
                extras=["fenced-code-blocks", "smarty-pants"]
            )
            processed_steps.append({
                "id": f"sim-step-{i}",
                "title": step_title,
                "narrative_html": narrative_html,
                "state_changes": state_changes
            })
                
        return {"simulation_steps": processed_steps}
    except Exception as e:
        return {"error": f"Failed to get or process game logic simulation: {str(e)}"}