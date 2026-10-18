import markdown2
import re 
from gemini_integration.client import call_gemini_api

def explain_code(code_content, focus_filename=None):
    focus_instruction = f"Your explanation should focus primarily on the contents and logic within the file `{focus_filename}`." if focus_filename else ""

    prompt = f"""
    You are an expert C++ code explanation assistant.
    Explain the following C++ code project. {focus_instruction}
    
    Your output MUST be clear, well-structured Markdown. For each logical block of code (like a function, a class declaration, or a group of related lines), you MUST follow this format:
    1.  First, present the complete, original C++ code block inside a Markdown code fence (```cpp).
    2.  Immediately below it, provide a clear, natural language explanation of what that specific code block does, its purpose, and how it works.
    
    Do not explain line-by-line. Explain in logical, meaningful chunks. Ensure there is a clear separation between an explanation and the next code block.

    --- Start C++ Code ---
    {code_content}
    --- End C++ Code ---

    Generate the explanation now.
    """
    try:
        explanation_md = call_gemini_api(prompt)
        explanation_html = markdown2.markdown(
            explanation_md, 
            extras=["fenced-code-blocks", "smarty-pants"]
        )
        return {"explanation_html": explanation_html}
    except Exception as e:
        return {"error": f"Failed to get full code explanation: {str(e)}"}

def explain_code_snippet(code_snippet):
    prompt = f"""
    Concisely explain what the following C++ code snippet does.
    C++ Code Snippet:
    ```cpp
    {code_snippet}
    ```
    """
    try:
        explanation_md = call_gemini_api(prompt)
        explanation_html = markdown2.markdown(
            explanation_md.strip(), 
            extras=["fenced-code-blocks", "smarty-pants"]
        )
        return {"explanation_html": explanation_html}
    except Exception as e:
        return {"error": f"Failed to get snippet explanation: {str(e)}"}

def answer_chat_query(project_code, selected_code, question, file_context=None):
    context_prompt = ""

    if file_context:
        context_prompt = f"""
        The user is asking a question *specifically* about the file named `{file_context}`.
        """
    
    if selected_code and selected_code.strip():
        context_prompt += f"""
        To help, they have also *selected* the following code snippet from that file:
        ```cpp
        {selected_code}
        ```
        """
    
    context_prompt += f"""
    Here is the full code of the project for your reference:
    ```cpp
    {project_code}
    ```
    """

    prompt = f"""
    You are an expert C++ assistant.
    {context_prompt}
    Please answer the user's question concisely: "{question}"
    
    Provide your answer in well-formatted Markdown. If you are suggesting code, 
    do not include line numbers in the snippet.
    """
    try:
        response_md = call_gemini_api(prompt)
        response_html = markdown2.markdown(
            response_md, 
            extras=["fenced-code-blocks", "smarty-pants"]
        )
        return {"chat_response_html": response_html}
    except Exception as e:
        return {"error": f"Failed to get chat response: {str(e)}"}