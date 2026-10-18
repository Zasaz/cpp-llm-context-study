import re
from gemini_integration.client import call_gemini_api

SUGGESTION_PATTERN = re.compile(
    r"\[LINE:\s*(\d+)\]\s*"
    r"\[TYPE:\s*(Performance|Readability|Bug|Security|Modernization|Other)\]\s*"
    r"\[SEVERITY:\s*(Low|Medium|High)\]\s*"
    r"\[MESSAGE:\s*(.*?)\]\s*"
    r"\[SUGGESTION:\s*(.*?)\]",
    re.DOTALL
)

def get_code_suggestions(code_content, focus_filename=None):
    focus_instruction = f" with special focus on file `{focus_filename}`" if focus_filename else ""
    
    prompt = f"""
    You are a C++ code review expert. Analyze the following C++ code and provide specific, actionable suggestions for improvement.
    
    {focus_instruction}
    
    FOR EACH SUGGESTION, provide exactly this format:
    [LINE: line_number]
    [TYPE: Performance/Readability/Bug/Security/Modernization/Other]
    [SEVERITY: Low/Medium/High]
    [MESSAGE: Brief description of the issue]
    [SUGGESTION: Specific code suggestion or improvement]
    
    Rules:
    - Only suggest changes that are objectively improvements
    - Focus on code that has clear issues or could be significantly improved
    - Be very specific about the exact line and what to change
    - For suggestions, provide the exact code that should replace the current code
    - Limit to 5-7 most important suggestions
    
    Separate suggestions with: ---SUGGESTION_SEPARATOR---
    
    If no meaningful suggestions are found, output: [NO_SUGGESTIONS: Code appears well-written]
    
    C++ Code:
    ```cpp
    {code_content}
    ```
    """
    
    try:
        suggestions_output = call_gemini_api(prompt, feature_name="code_suggestions")
        
        if "[NO_SUGGESTIONS:" in suggestions_output:
            return {"suggestions": []}
        
        suggestions = []
        suggestion_blocks = suggestions_output.split("---SUGGESTION_SEPARATOR---")
        
        for block in suggestion_blocks:
            block = block.strip()
            if not block:
                continue
                
            match = SUGGESTION_PATTERN.search(block)
            if match:
                line_num = int(match.group(1))
                suggestion_type = match.group(2)
                severity = match.group(3)
                message = match.group(4).strip()
                suggestion = match.group(5).strip()
                
                severity_scores = {"Low": 1, "Medium": 2, "High": 3}
                severity_score = severity_scores.get(severity, 1)
                
                suggestions.append({
                    "line": line_num,
                    "type": suggestion_type,
                    "severity": severity,
                    "severity_score": severity_score,
                    "message": message,
                    "suggestion": suggestion
                })
        
        suggestions.sort(key=lambda x: (x["line"], -x["severity_score"]))
        
        return {"suggestions": suggestions}
        
    except Exception as e:
        return {"error": f"Failed to get code suggestions: {str(e)}"}