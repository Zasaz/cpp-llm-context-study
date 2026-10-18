import markdown2
from gemini_integration.client import call_gemini_api

def generate_documentation(code_content, focus_filename, doc_format):
    if doc_format == 'doxygen':
        prompt = f"""
        You are a C++ documentation assistant specializing in Doxygen.
        Analyze the following C++ code. Your task is to return the complete, original code from the file `{focus_filename}`, but with Doxygen-style comment blocks (`/** ... */`) added directly above every function, class, struct, and method.

        - For functions/methods, document the purpose, each parameter (`@param`), and the return value (`@return`).
        - For classes/structs, provide a brief description (`@brief`).

        Do not explain the code or add any text outside of the returned code block. The output should be a single C++ code block.

        Full Project Code for Context:
        ```cpp
        {code_content}
        ```
        """
        try:
            generated_code = call_gemini_api(prompt)
            if generated_code.startswith("```cpp"):
                generated_code = generated_code[len("```cpp"):].strip()
            if generated_code.endswith("```"):
                generated_code = generated_code[:-len("```")].strip()
            return {"format": "doxygen", "content": generated_code}
        except Exception as e:
            return {"error": f"Failed to generate Doxygen comments: {e}"}

    elif doc_format == 'markdown':
        prompt = f"""
        You are a C++ documentation assistant. Create a README-style Markdown summary for the file `{focus_filename}`.
        Analyze the code and generate documentation for each class and function.

        For each item, provide:
        - A clear heading for the class or function name.
        - A description of its purpose.
        - A list of its parameters and what they do.
        - A description of its return value, if any.

        Use clear and concise language. The output should be well-structured Markdown.

        Full Project Code for Context:
        ```cpp
        {code_content}
        ```
        """
        try:
            explanation_md = call_gemini_api(prompt)
            explanation_html = markdown2.markdown(explanation_md, extras=["fenced-code-blocks"])
            return {"format": "markdown", "content_html": explanation_html}
        except Exception as e:
            return {"error": f"Failed to generate Markdown documentation: {e}"}

    return {"error": "Invalid documentation format specified."}