import subprocess
import os
import time
import re
import tempfile
import glob
from gemini_integration.client import call_gemini_api
import markdown2
from utils import get_project_code

def get_gemini_performance_analysis(code_content, focus_filename=None):
    focus_instruction = ""
    if focus_filename:
        focus_instruction = f"Focus your analysis of potential bottlenecks and memory hotspots on the file `{focus_filename}`, using the rest of the project for context."
    
    prompt = f"""
    Analyze the following C++ code for its performance characteristics. The code may consist of multiple files.
    {focus_instruction}
    Provide ONLY a concise summary covering these points directly:
    1.  **Estimated Complexity:** (e.g., Low, Medium, High).
    2.  **Potential Bottlenecks:** Identify specific lines or functions.
    3.  **Memory Hotspots:** Identify areas likely to consume significant memory.

    DO NOT include any introductory phrases or Markdown headers.
    Start directly with point 1. Use Markdown for bolding or lists *within* the points.

    ```cpp
    {code_content}
    ```
    """
    try:
        analysis_md = call_gemini_api(prompt)
        cleaned_analysis = analysis_md.strip()
        if cleaned_analysis.startswith("```markdown"):
            cleaned_analysis = cleaned_analysis[len("```markdown"):].strip()
        if cleaned_analysis.endswith("```"):
            cleaned_analysis = cleaned_analysis[:-len("```")].strip()
        patterns_to_remove = [
            r"^\s*#+\s*Performance Analysis.*?\n",
            r"^\s*Here's a brief summary.*?\n",
            r"^\s*Okay, here's the analysis:.*?\n"
        ]
        for pattern in patterns_to_remove:
            cleaned_analysis = re.sub(pattern, "", cleaned_analysis, flags=re.IGNORECASE | re.MULTILINE).strip()
        analysis_html = markdown2.markdown(cleaned_analysis, extras=["fenced-code-blocks"])
        return analysis_html
    except Exception as e: return f"<p>Failed to get AI analysis: {e}</p>"

def parse_time_output(time_output):
    data = {"exec_time_s_val": None, "peak_mem_mb_val": None, "cpu_time_s_val": None}
    exec_time_match = re.search(r"Elapsed \(wall clock\) time \(h:mm:ss or m:ss\): (.*)", time_output)
    mem_match = re.search(r"Maximum resident set size \(kbytes\): (\d+)", time_output)
    user_time_match = re.search(r"User time \(seconds\): (.*)", time_output)
    sys_time_match = re.search(r"System time \(seconds\): (.*)", time_output)
    if exec_time_match:
        time_str = exec_time_match.group(1).strip()
        try:
            parts = list(map(float, time_str.split(':')))
            if len(parts) == 3: data["exec_time_s_val"] = parts[0] * 3600 + parts[1] * 60 + parts[2]
            elif len(parts) == 2: data["exec_time_s_val"] = parts[0] * 60 + parts[1]
            elif len(parts) == 1: data["exec_time_s_val"] = parts[0]
        except ValueError: data["exec_time_s_val"] = None
    if mem_match:
        try: data["peak_mem_mb_val"] = int(mem_match.group(1)) / 1024.0
        except ValueError: data["peak_mem_mb_val"] = None
    if user_time_match and sys_time_match:
        try:
            user_t = float(user_time_match.group(1)); sys_t = float(sys_time_match.group(1))
            data["cpu_time_s_val"] = user_t + sys_t
        except ValueError: data["cpu_time_s_val"] = None
    return data

def analyze_performance(project_dir, focus_filename=None):
    result = {
        "execution_time_s_val": None, "peak_memory_mb_val": None, "cpu_time_s_val": None,
        "execution_time_str": "N/A", "peak_memory_str": "N/A", "cpu_time_str": "N/A",
        "ai_analysis": "N/A", "error": None, "compilation": "",
    }

    try:
        project_code_for_analysis = get_project_code(project_dir)
        result["ai_analysis"] = get_gemini_performance_analysis(project_code_for_analysis, focus_filename)
        
        source_files = glob.glob(os.path.join(project_dir, '**', '*.cpp'), recursive=True)
        if not source_files:
            result['error'] = "Compilation failed: No .cpp files found in the project."
            return result

        executable_path = os.path.join(project_dir, 'perf_executable')
        
        compile_command = ['g++', '-I', project_dir] + source_files + ['-o', executable_path, '-O2', '-std=c++17', '-Wall']

        compile_proc = subprocess.run(compile_command, capture_output=True, text=True, timeout=15)
        result['compilation'] = compile_proc.stdout
        if compile_proc.stderr and compile_proc.returncode != 0:
             result['compilation'] += f"\n--- Compilation Errors/Warnings ---\n{compile_proc.stderr}"
        
        if compile_proc.returncode != 0:
            result['error'] = "Compilation failed."
            return result

        time_command = ['/usr/bin/time', '-v', executable_path]
        try:
            run_proc = subprocess.run(time_command, capture_output=True, text=True, timeout=15, input="")
            parsed_data = parse_time_output(run_proc.stderr)
            result.update({k: v for k, v in parsed_data.items() if v is not None})
            result["execution_time_str"] = f"{result['execution_time_s_val']:.4f}s" if result["execution_time_s_val"] is not None else "N/A"
            result["peak_memory_str"] = f"{result['peak_memory_mb_val']:.2f} MB" if result["peak_memory_mb_val"] is not None else "N/A (Req /usr/bin/time)"
            result["cpu_time_str"] = f"{result['cpu_time_s_val']:.3f}s" if result["cpu_time_s_val"] is not None else "N/A (Req /usr/bin/time)"
            if run_proc.returncode != 0: result['error'] = (result.get('error') or "") + f"\nExecution failed:\n{run_proc.stdout}\n{run_proc.stderr}"
            if result["execution_time_s_val"] is None and result["error"] is None: raise FileNotFoundError
        except FileNotFoundError:
            result["peak_memory_str"] = "N/A (Req /usr/bin/time)"; result["cpu_time_str"] = "N/A (Req /usr/bin/time)"
            start_time = time.perf_counter()
            try:
                run_proc_fallback = subprocess.run([executable_path], capture_output=True, text=True, timeout=15, input="")
                end_time = time.perf_counter(); exec_s = end_time - start_time
                result["execution_time_s_val"] = exec_s; result["execution_time_str"] = f"{exec_s:.4f}s (Python Measured)"
                if run_proc_fallback.returncode != 0: result['error'] = (result.get('error') or "") + f"\nExecution failed (fallback):\n{run_proc_fallback.stderr}"
            except Exception as e_fallback:
                result["execution_time_str"] = "N/A (Error in fallback)"
                result['error'] = (result.get('error') or "") + f"\nFallback execution failed: {e_fallback}"
        except Exception as e_measure:
            result['error'] = (result.get('error') or "") + f"\nExecution/Measurement failed: {e_measure}"
    
    except FileNotFoundError as e_fnf:
        result['error'] = f"Compiler (g++) or related tool not found: {e_fnf}"
    except Exception as e_outer:
        result['error'] = f"An outer error occurred during performance analysis: {str(e_outer)}"

    return result