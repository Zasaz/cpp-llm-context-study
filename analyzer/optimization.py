import markdown2
import re
from gemini_integration.client import call_gemini_api

SUGGESTION_BLOCK_PATTERN = re.compile(
    r"\[CATEGORY:\s*(.*?)\]\s*"
    r"\[RELEVANT_CODE_START_LINE:\s*(.*?)\]\s*"
    r"\[RELEVANT_CODE_END_LINE:\s*(.*?)\]\s*"
    r"\[SUGGESTION_TITLE:\s*(.*?)\]\s*"
    r"\[PROBLEM_DESCRIPTION:\s*(.*?)\]\s*"
    r"\[HUMANIZED_EXPLANATION_MARKDOWN:\s*(.*?)\]\s*"
    r"(?:\[SUGGESTED_CODE_SNIPPET_CPP:\s*```cpp\n(.*?)\n```\]|\[SUGGESTED_CODE_SNIPPET_CPP:\s*N/A\s*\])", 
    re.DOTALL | re.IGNORECASE
)
SUGGESTION_SEPARATOR = "---OPTIMIZATION_SUGGESTION_SEPARATOR---"

def get_optimizations(code_content, focus_filename=None):
    focus_instruction = ""
    if focus_filename:
        focus_instruction = f"Please focus your optimization suggestions primarily on the file `{focus_filename}`, using the other files as necessary context."

    prompt = f"""
    You are an expert C++ code optimization assistant.
    Analyze the following C++ code project and provide specific, actionable optimization suggestions.
    {focus_instruction}
    Aim to provide the most impactful suggestions first.

    For EACH suggestion you provide (up to a maximum of 4-5 distinct suggestions), YOU MUST use the following structured format with the exact tags on separate lines:
    [CATEGORY: Performance/Readability/Memory/ModernC++/Security/Other]
    [RELEVANT_CODE_START_LINE: approximate starting line number (integer or N/A). For "Apply Changes".]
    [RELEVANT_CODE_END_LINE: approximate ending line number (integer or N/A). For "Apply Changes".]
    [SUGGESTION_TITLE: A brief, clear title for this optimization suggestion.]
    [PROBLEM_DESCRIPTION: A short, distinct description of the problem or observation. If no problem, state that.]
    [HUMANIZED_EXPLANATION_MARKDOWN: A conversational explanation of the suggestion (if any) or your assessment. This is the main descriptive text. Use Markdown formatting.]
    [SUGGESTED_CODE_SNIPPET_CPP: ```cpp
    // ONLY the optimized C++ code snippet that addresses the problem.
    // If no code change, or if the suggestion is conceptual, put N/A here.
    ```]

    Separate each complete suggestion block with:
    {SUGGESTION_SEPARATOR}

    **VERY IMPORTANT:** If the provided C++ code is already well-optimized and you find NO SPECIFIC, ACTIONABLE code changes to suggest, then output ONLY ONE block with the following specific values:
    [CATEGORY: Optimal]
    [RELEVANT_CODE_START_LINE: N/A]
    [RELEVANT_CODE_END_LINE: N/A]
    [SUGGESTION_TITLE: Code Appears Well-Optimized]
    [PROBLEM_DESCRIPTION: No specific issues requiring code modification were identified.]
    [HUMANIZED_EXPLANATION_MARKDOWN: The code follows good practices and appears efficient for its current scope. No actionable optimization suggestions are recommended at this time.]
    [SUGGESTED_CODE_SNIPPET_CPP: N/A]
    {SUGGESTION_SEPARATOR}

    User's C++ Project Code:
    ```cpp
    {code_content}
    ```
    Generate the optimization suggestions now.
    """
    try:
        raw_output = call_gemini_api(prompt)
        optimizations = []
        suggestion_blocks = raw_output.split(SUGGESTION_SEPARATOR)

        for block in suggestion_blocks:
            block = block.strip()
            if not block: continue

            match = SUGGESTION_BLOCK_PATTERN.fullmatch(block)
            if match:
                category = match.group(1).strip()
                start_line_str = match.group(2).strip()
                end_line_str = match.group(3).strip()
                title = match.group(4).strip()
                problem = match.group(5).strip() 
                explanation_markdown = match.group(6).strip()
                raw_code_snippet = match.group(7).strip() if match.group(7) else None 

                if raw_code_snippet and raw_code_snippet.lower() == "n/a":
                    raw_code_snippet = None

                start_line, end_line = None, None
                if start_line_str.lower() != "n/a" and start_line_str.isdigit():
                    start_line = int(start_line_str)
                if end_line_str.lower() != "n/a" and end_line_str.isdigit():
                    end_line = int(end_line_str)
                elif start_line is not None: 
                    end_line = start_line
                
                full_explanation_md = f"**Identified Area:** {problem}\n\n**Suggestion & Rationale:**\n{explanation_markdown}" \
                                      if problem and problem.lower() != "n/a" \
                                      else explanation_markdown
                
                explanation_html = markdown2.markdown(
                    full_explanation_md, 
                    extras=["fenced-code-blocks", "smarty-pants"]
                )
                optimizations.append({
                    "category": category,
                    "start_line": start_line,
                    "end_line": end_line,
                    "title": title,
                    "explanation_html": explanation_html,
                    "suggested_code_raw": raw_code_snippet
                })
            else:
                if len(suggestion_blocks) == 1 and not optimizations:
                     optimizations.append({
                        "category": "General", "start_line": None, "end_line": None,
                        "title": "General Analysis", 
                        "explanation_html": markdown2.markdown(f"**Note:** The AI provided feedback, but its structure could not be fully parsed into detailed suggestions. Displaying raw analysis:\n\n{block}", extras=["fenced-code-blocks", "smarty-pants"]),
                        "suggested_code_raw": None })
                print(f"Warning: Could not parse optimization block: {block[:100]}...")
        
        if not optimizations and raw_output.strip():
             optimizations.append({
                "category": "General", "start_line": None, "end_line": None,
                "title": "Raw Analysis Output", 
                "explanation_html": markdown2.markdown(f"**Note:** The AI's response was not empty but could not be structured. Raw output:\n\n{raw_output.strip()}", extras=["fenced-code-blocks", "smarty-pants"]),
                "suggested_code_raw": None })
        elif not optimizations and not raw_output.strip(): 
            optimizations.append({
                "category": "Optimal", "start_line": None, "end_line": None,
                "title": "No Output from Analyzer",
                "explanation_html": "<p>The optimization analysis engine did not return any feedback for this code.</p>",
                "suggested_code_raw": None
            })
            
        return {"optimizations": optimizations}
    except Exception as e:
        return {"error": f"Failed to get or process code optimizations: {str(e)}"}