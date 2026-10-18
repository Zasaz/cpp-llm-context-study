import markdown2
from gemini_integration.client import call_gemini_api

def get_ai_reasoning(code_content, analysis_type, focus_filename=None):
    focus_instruction = f" with special focus on file `{focus_filename}`" if focus_filename else ""
    
    prompt = f"""
    You are an AI transparency assistant. Your task is to explain your reasoning process for analyzing C++ code.
    
    ANALYSIS CONTEXT:
    - Analysis Type: {analysis_type}
    - Focus File: {focus_filename if focus_filename else "Entire project"}
    
    Please provide a transparent breakdown of your reasoning process for this analysis. Include:
    
    1. **Initial Code Assessment**: What were your first observations about the code structure and quality?
    2. **Analysis Methodology**: What specific approaches or patterns did you look for?
    3. **Key Findings**: What were the most significant observations and why?
    4. **Decision Process**: How did you determine what suggestions to provide?
    5. **Confidence Level**: How confident are you in your analysis and why?
    
    Be honest about limitations, uncertainties, and alternative approaches you considered.
    
    C++ Code:
    ```cpp
    {code_content}
    ```
    
    Provide your transparent reasoning process in clear Markdown format.
    """
    
    try:
        reasoning_md = call_gemini_api(prompt, feature_name="ai_transparency")
        reasoning_html = markdown2.markdown(
            reasoning_md, 
            extras=["fenced-code-blocks", "smarty-pants"]
        )
        return {"reasoning_html": reasoning_html}
    except Exception as e:
        return {"error": f"Failed to generate AI reasoning: {str(e)}"}