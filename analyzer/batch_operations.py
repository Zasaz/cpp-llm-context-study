import markdown2
from gemini_integration.client import call_gemini_api

def analyze_multiple_files(code_content, operation_type, focus_files=None):
    focus_instruction = ""
    if focus_files:
        focus_instruction = f" with special focus on files: {', '.join(focus_files)}"
    
    operation_prompts = {
        'architecture_review': f"""
        You are a software architecture expert. Analyze this multi-file C++ project for architectural quality.
        
        Provide a comprehensive review covering:
        
        1. **Overall Architecture Assessment**
           - Code organization and modularity
           - Dependency structure and coupling
           - File and namespace organization
        
        2. **Design Patterns & Principles**
           - Use of appropriate design patterns
           - SOLID principles adherence
           - Code reusability and extensibility
        
        3. **Cross-File Dependencies**
           - Circular dependencies identification
           - Module boundaries and interfaces
           - Build system implications
        
        4. **Architectural Recommendations**
           - Suggested refactoring opportunities
           - Module reorganization ideas
           - Long-term maintainability improvements
        
        C++ Project{focus_instruction}:
        ```cpp
        {code_content}
        ```
        
        Provide your analysis in clear Markdown format with specific examples from the code.
        """,
        
        'code_quality_audit': f"""
        You are a code quality expert. Perform a comprehensive audit of this C++ project.
        
        Analyze across these dimensions:
        
        1. **Consistency & Standards**
           - Coding style consistency across files
           - Naming conventions adherence
           - Comment and documentation quality
        
        2. **Error Handling & Robustness**
           - Exception handling patterns
           - Resource management (memory, files)
           - Input validation practices
        
        3. **Testing & Maintainability**
           - Testability of the codebase
           - Code complexity metrics
           - Documentation completeness
        
        4. **Security & Safety**
           - Potential security vulnerabilities
           - Memory safety issues
           - Input sanitization
        
        C++ Project{focus_instruction}:
        ```cpp
        {code_content}
        ```
        
        Provide specific, actionable recommendations with file references.
        """,
        
        'performance_analysis': f"""
        You are a performance optimization expert. Analyze this C++ project for performance characteristics.
        
        Focus on:
        
        1. **Algorithmic Efficiency**
           - Time complexity analysis
           - Bottleneck identification
           - Inefficient patterns
        
        2. **Memory Usage**
           - Memory allocation patterns
           - Cache locality issues
           - Memory leaks potential
        
        3. **I/O Operations**
           - File I/O efficiency
           - Network operations (if any)
           - Buffer management
        
        4. **Concurrency & Parallelism**
           - Thread safety analysis
           - Potential race conditions
           - Parallelization opportunities
        
        C++ Project{focus_instruction}:
        ```cpp
        {code_content}
        ```
        
        Provide performance recommendations with estimated impact.
        """,
        
        'dependency_analysis': f"""
        You are a dependency management expert. Analyze the dependencies and relationships in this C++ project.
        
        Analyze:
        
        1. **Internal Dependencies**
           - File inclusion patterns
           - Class and function dependencies
           - Circular dependency detection
        
        2. **External Dependencies**
           - Library usage analysis
           - System API dependencies
           - Third-party integration points
        
        3. **Build & Compilation**
           - Compilation unit organization
           - Header file dependencies
           - Link-time dependencies
        
        4. **Dependency Recommendations**
           - Dependency reduction opportunities
           - Interface improvements
           - Build optimization suggestions
        
        C++ Project{focus_instruction}:
        ```cpp
        {code_content}
        ```
        
        Provide dependency graphs and improvement suggestions.
        """
    }
    
    if operation_type not in operation_prompts:
        return {"error": f"Unknown batch operation type: {operation_type}"}
    
    prompt = operation_prompts[operation_type]
    
    try:
        analysis_result = call_gemini_api(prompt, feature_name=f"batch_{operation_type}")
        
        analysis_html = markdown2.markdown(
            analysis_result, 
            extras=["fenced-code-blocks", "smarty-pants", "tables"]
        )
        
        return {
            "analysis_html": analysis_html,
            "operation_type": operation_type,
            "focus_files": focus_files
        }
        
    except Exception as e:
        return {"error": f"Failed to perform batch analysis: {str(e)}"}