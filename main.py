import os
import tempfile
import shutil
from flask import Flask, request, jsonify
from flask_cors import CORS 

from cpp_handler.sandbox import run_cpp_code
from cpp_handler.test_runner import run_test_suite
from analyzer.natural_language import explain_code, answer_chat_query
from analyzer.performance import analyze_performance
from analyzer.simulation import simulate_logic
from analyzer.optimization import get_optimizations
from analyzer import unit_tests as unit_test_generator
from analyzer.documentation import generate_documentation

from analyzer.ai_transparency import get_ai_reasoning
from analyzer.code_health import calculate_health_score
from analyzer.ethical_review import conduct_ethical_review
from analyzer.code_suggestions import get_code_suggestions
from analyzer.batch_operations import analyze_multiple_files
from analyzer.architecture import generate_dependency_graph

from utils import get_project_code
from analyzer import context_strategies

app = Flask(__name__)
CORS(app)

@app.route('/api/health')
def health_check():
    return jsonify({"status": "healthy"}), 200

@app.route('/api/chat', methods=['POST'])
def chat_with_project():
    data = request.json
    if not data:
        return jsonify({"error": "No JSON payload received"}), 400

    question = data.get('question')
    files_data = data.get('files')
    selection = data.get('selection')
    file_context = data.get('file_context')
    
    if not question or not files_data:
        return jsonify({"error": "Missing 'question' or 'files' in payload"}), 400

    temp_project_dir = None
    try:
        temp_project_dir = write_project_to_temp(files_data)
        project_code_for_analysis, context_meta = context_strategies.select_context(
            temp_project_dir,
            strategy=data.get('strategy'),
            active_file=file_context,
            selection=selection,
        )

        analysis_result = answer_chat_query(
            project_code_for_analysis,
            selection,
            question,
            file_context
        )

        if isinstance(analysis_result, dict):
            analysis_result['context_meta'] = context_meta
        return jsonify(analysis_result)

    except Exception as e:
        print(f"An error occurred: {e}") 
        return jsonify({"error": f"An internal server error occurred: {str(e)}"}), 500
        
    finally:
        if temp_project_dir and os.path.exists(temp_project_dir):
            shutil.rmtree(temp_project_dir)

@app.route('/api/analyze', methods=['POST'])
def analyze_project():
    data = request.json
    if not data:
        return jsonify({"error": "No JSON payload received"}), 400

    feature = data.get('feature')
    files_data = data.get('files')
    active_file = data.get('active_file')
    
    if not feature or not files_data:
        return jsonify({"error": "Missing 'feature' or 'files' in payload"}), 400

    temp_project_dir = None
    try:
        temp_project_dir = write_project_to_temp(files_data)
        project_code_for_analysis, context_meta = context_strategies.select_context(
            temp_project_dir,
            strategy=data.get('strategy'),
            active_file=active_file,
            selection=data.get('selection'),
        )

        analysis_result = {}

        if feature == 'run_project':
            analysis_result = run_cpp_code(temp_project_dir)
        elif feature == 'explain_code':
            analysis_result = explain_code(project_code_for_analysis, active_file)
        elif feature == 'generate_tests':
            analysis_result = unit_test_generator.generate_tests(project_code_for_analysis, active_file)
        elif feature == 'run_test_suite':
            test_suite_code = data.get('test_suite_code')
            if not test_suite_code:
                return jsonify({"error": "Missing 'test_suite_code' for this feature"}), 400
            analysis_result = run_test_suite(temp_project_dir, test_suite_code)
        elif feature == 'generate_docs':
            doc_format = data.get('doc_format', 'doxygen')
            analysis_result = generate_documentation(project_code_for_analysis, active_file, doc_format)
        elif feature == 'analyze_performance':
            analysis_result = analyze_performance(temp_project_dir, active_file)
        elif feature == 'simulate_logic':
            analysis_result = simulate_logic(project_code_for_analysis, active_file)
        elif feature == 'optimize_code':
            analysis_result = get_optimizations(project_code_for_analysis, active_file)
        
        elif feature == 'ai_transparency':
            analysis_type = data.get('analysis_type', 'general')
            analysis_result = get_ai_reasoning(project_code_for_analysis, analysis_type, active_file)
        elif feature == 'code_health':
            analysis_result = calculate_health_score(project_code_for_analysis, active_file)
        elif feature == 'ethical_review':
            analysis_result = conduct_ethical_review(project_code_for_analysis, active_file)
        elif feature == 'code_suggestions':
            analysis_result = get_code_suggestions(project_code_for_analysis, active_file)
        elif feature == 'batch_operations':
            operation_type = data.get('operation_type', 'architecture_review')
            focus_files = data.get('focus_files')
            analysis_result = analyze_multiple_files(project_code_for_analysis, operation_type, focus_files)
        else:
            return jsonify({"error": f"Unknown feature: {feature}"}), 400

        if isinstance(analysis_result, dict):
            analysis_result['context_meta'] = context_meta
        return jsonify(analysis_result)



    except Exception as e:
        print(f"An error occurred: {e}") 
        return jsonify({"error": f"An internal server error occurred: {str(e)}"}), 500
        
    finally:
        if temp_project_dir and os.path.exists(temp_project_dir):
            shutil.rmtree(temp_project_dir)

@app.route('/api/architecture/graph', methods=['POST'])
def architecture_graph():
    data = request.json
    if not data:
        return jsonify({"error": "No JSON payload received"}), 400

    files_data = data.get('files')
    if not files_data:
        return jsonify({"error": "Missing 'files' in payload"}), 400

    project_code = {}
    for f in files_data:
        if 'name' in f and 'content' in f:
            project_code[f['name']] = f['content']
            
    try:
        graph_data = generate_dependency_graph(project_code)
        return jsonify(graph_data)
    except Exception as e:
        print(f"Graph generation error: {e}")
        return jsonify({"error": str(e)}), 500

def write_project_to_temp(files_data):
    temp_dir = tempfile.mkdtemp()
    
    for file_data in files_data:
        filename = file_data.get('name')
        content = file_data.get('content')
        if not filename or content is None:
            continue
            
        file_path = os.path.join(temp_dir, filename)
        file_dir = os.path.dirname(file_path)
        if not os.path.exists(file_dir):
            os.makedirs(file_dir)
            
        with open(file_path, 'w', encoding='utf-8') as f:
            f.write(content)
            
    return temp_dir

if __name__ == '__main__':
    app.run(debug=True, host='0.0.0.0', port=5000)