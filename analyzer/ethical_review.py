import markdown2
import re
from gemini_integration.client import call_gemini_api

ETHICAL_ISSUE_PATTERN = re.compile(
    r"\[ISSUE_TYPE:\s*(.*?)\]\s*"
    r"\[SEVERITY:\s*(Low|Medium|High|Critical)\]\s*"
    r"\[LOCATION:\s*(.*?)\]\s*"
    r"\[DESCRIPTION:\s*(.*?)\]\s*"
    r"\[POTENTIAL_IMPACT:\s*(.*?)\]\s*"
    r"\[REMEDIATION:\s*(.*?)\]",
    re.DOTALL
)

def conduct_ethical_review(code_content, focus_filename=None):
    focus_instruction = f" with special attention to file `{focus_filename}`" if focus_filename else ""
    
    prompt = f"""
    You are an ethical code review expert. Analyze this C++ code for potential ethical issues.
    
    REVIEW CRITERIA:
    1. **Bias & Fairness**: Algorithmic bias, discriminatory patterns, unfair treatment
    2. **Privacy & Data Protection**: Data collection, storage, sharing practices
    3. **Security & Safety**: Vulnerabilities, potential misuse, safety-critical issues
    4. **Transparency & Accountability**: Opaque decision-making, lack of audit trails
    5. **Social Impact**: Potential societal consequences, environmental impact
    
    FOR EACH ETHICAL ISSUE FOUND, provide exactly this format:
    [ISSUE_TYPE: Specific issue category]
    [SEVERITY: Low/Medium/High/Critical]
    [LOCATION: File/function/line where issue occurs]
    [DESCRIPTION: Detailed description of the ethical concern]
    [POTENTIAL_IMPACT: What could happen if this issue is not addressed]
    [REMEDIATION: Specific steps to address the ethical concern]
    
    Separate issues with: ---ETHICAL_ISSUE_SEPARATOR---
    
    If no ethical issues are found, output: [NO_ISSUES: No ethical concerns detected]
    
    C++ Code{focus_instruction}:
    ```cpp
    {code_content}
    ```
    """
    
    try:
        ethical_analysis = call_gemini_api(prompt, feature_name="ethical_review")
        
        if "[NO_ISSUES:" in ethical_analysis:
            return {
                "issues_found": 0,
                "issues": [],
                "summary": "No ethical concerns detected in the code."
            }
        
        issues = []
        issue_blocks = ethical_analysis.split("---ETHICAL_ISSUE_SEPARATOR---")
        
        for block in issue_blocks:
            block = block.strip()
            if not block:
                continue
                
            match = ETHICAL_ISSUE_PATTERN.search(block)
            if match:
                issue_type = match.group(1).strip()
                severity = match.group(2).strip()
                location = match.group(3).strip()
                description = match.group(4).strip()
                impact = match.group(5).strip()
                remediation = match.group(6).strip()
                
                severity_scores = {"Low": 1, "Medium": 2, "High": 3, "Critical": 4}
                severity_score = severity_scores.get(severity, 0)
                
                issues.append({
                    "issue_type": issue_type,
                    "severity": severity,
                    "severity_score": severity_score,
                    "location": location,
                    "description_html": markdown2.markdown(description, extras=["fenced-code-blocks"]),
                    "impact_html": markdown2.markdown(impact, extras=["fenced-code-blocks"]),
                    "remediation_html": markdown2.markdown(remediation, extras=["fenced-code-blocks"])
                })
        
        issues.sort(key=lambda x: x["severity_score"], reverse=True)
        
        severity_counts = {"Critical": 0, "High": 0, "Medium": 0, "Low": 0}
        for issue in issues:
            severity_counts[issue["severity"]] += 1
        
        summary = f"Found {len(issues)} ethical issue(s): "
        summary_parts = []
        for sev in ["Critical", "High", "Medium", "Low"]:
            if severity_counts[sev] > 0:
                summary_parts.append(f"{severity_counts[sev]} {sev}")
        summary += ", ".join(summary_parts)
        
        return {
            "issues_found": len(issues),
            "issues": issues,
            "summary": summary,
            "severity_counts": severity_counts
        }
        
    except Exception as e:
        return {"error": f"Failed to conduct ethical review: {str(e)}"}