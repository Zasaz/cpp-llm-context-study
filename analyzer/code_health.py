import markdown2
import re
from gemini_integration.client import call_gemini_api

HEALTH_CATEGORY_PATTERN = re.compile(
    r"\[CATEGORY:\s*(.*?)\]\s*"
    r"\[SCORE:\s*(\d+)/(\d+)\]\s*"
    r"\[WEIGHT:\s*(\d+)\]\s*"
    r"\[EXPLANATION:\s*(.*?)\]\s*"
    r"\[RECOMMENDATIONS:\s*(.*?)\]",
    re.DOTALL
)

def calculate_health_score(code_content, focus_filename=None):
    focus_instruction = f" with special attention to file `{focus_filename}`" if focus_filename else ""
    
    prompt = f"""
    You are a code quality assessment expert. Analyze this C++ code and provide a comprehensive health score assessment.
    
    ASSESSMENT CRITERIA:
    1. **Readability & Structure** (Weight: 20)
       - Code organization, naming conventions, comments
       - File structure and modularity
       
    2. **Functionality & Correctness** (Weight: 25)
       - Logical correctness, error handling
       - Algorithm efficiency, edge cases
       
    3. **Maintainability** (Weight: 20)
       - Code complexity, coupling, documentation
       - Ease of modification and extension
       
    4. **Security & Safety** (Weight: 15)
       - Memory safety, input validation
       - Potential vulnerabilities
       
    5. **Performance** (Weight: 10)
       - Efficiency of algorithms and data structures
       - Resource usage optimization
       
    6. **Modern C++ Practices** (Weight: 10)
       - Use of modern C++ features (C++17/20)
       - Best practices adherence
    
    FOR EACH CATEGORY, provide exactly this format:
    [CATEGORY: Category Name]
    [SCORE: X/10] (where X is 0-10)
    [WEIGHT: Y] (the weight from above)
    [EXPLANATION: Detailed explanation of the score with specific code examples]
    [RECOMMENDATIONS: Specific, actionable improvement suggestions]
    
    Separate categories with: ---HEALTH_CATEGORY_SEPARATOR---
    
    After all categories, provide an overall score calculation and improvement roadmap.
    
    C++ Code{focus_instruction}:
    ```cpp
    {code_content}
    ```
    """
    
    try:
        health_analysis = call_gemini_api(prompt, feature_name="code_health")
        
        categories = []
        total_score = 0
        total_weight = 0
        
        category_blocks = health_analysis.split("---HEALTH_CATEGORY_SEPARATOR---")
        
        for block in category_blocks:
            block = block.strip()
            if not block:
                continue
                
            match = HEALTH_CATEGORY_PATTERN.search(block)
            if match:
                category = match.group(1).strip()
                score = int(match.group(2))
                max_score = int(match.group(3))
                weight = int(match.group(4))
                explanation = match.group(5).strip()
                recommendations = match.group(6).strip()
                
                normalized_score = (score / max_score) * 100
                weighted_score = normalized_score * (weight / 100)
                
                categories.append({
                    "category": category,
                    "score": score,
                    "max_score": max_score,
                    "weight": weight,
                    "normalized_score": round(normalized_score, 1),
                    "weighted_score": round(weighted_score, 1),
                    "explanation_html": markdown2.markdown(explanation, extras=["fenced-code-blocks"]),
                    "recommendations_html": markdown2.markdown(recommendations, extras=["fenced-code-blocks"])
                })
                
                total_score += weighted_score
                total_weight += weight
        
        overall_score = round((total_score / total_weight) * 100, 1) if total_weight > 0 else 0

        if overall_score >= 90:
            health_level = "Excellent"
            health_color = "success"
        elif overall_score >= 75:
            health_level = "Good"
            health_color = "info"
        elif overall_score >= 60:
            health_level = "Fair"
            health_color = "warning"
        else:
            health_level = "Needs Improvement"
            health_color = "danger"
        
        return {
            "overall_score": overall_score,
            "health_level": health_level,
            "health_color": health_color,
            "categories": categories,
            "raw_analysis": health_analysis
        }
        
    except Exception as e:
        return {"error": f"Failed to calculate health score: {str(e)}"}