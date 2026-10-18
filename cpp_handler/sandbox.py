import subprocess
import os
import glob

def run_cpp_code(project_dir):
    result = {'compilation': '', 'execution': '', 'error': ''}

    try:
        source_files = glob.glob(os.path.join(project_dir, '*.cpp'))
        if not source_files:
            result['error'] = "Compilation failed: No .cpp source files found in the project."
            return result

        executable_path = os.path.join(project_dir, 'a.out')
        
        compile_command = ['g++', '-I', project_dir] + source_files + ['-o', executable_path, '-std=c++17', '-Wall']

        compile_proc = subprocess.run(
            compile_command,
            capture_output=True,
            text=True,
            timeout=15
        )
        result['compilation'] = compile_proc.stdout
        if compile_proc.stderr and compile_proc.returncode != 0:
             result['compilation'] += f"\n--- Compilation Errors/Warnings ---\n{compile_proc.stderr}"

        if compile_proc.returncode != 0:
            result['error'] = "Compilation failed."
            return result

        run_proc = subprocess.run(
            [executable_path],
            capture_output=True,
            text=True,
            timeout=15
        )
        result['execution'] = run_proc.stdout
        if run_proc.stderr:
             result['execution'] += f"\n--- Execution Errors/Warnings ---\n{run_proc.stderr}"

        if run_proc.returncode != 0:
            error_msg = f"Execution returned non-zero exit code: {run_proc.returncode}."
            result['error'] = (result.get('error') or "") + "\n" + error_msg if result.get('error') else error_msg

    except subprocess.TimeoutExpired:
        result['error'] = "Process timed out during compilation or execution."
    except FileNotFoundError as e:
        result['error'] = f"Compiler (g++) or related tool not found: {e}"
    except Exception as e:
        result['error'] = f"An unexpected error occurred: {str(e)}"

    return result