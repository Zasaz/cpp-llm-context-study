import re
from analyzer import symbol_extractor

def generate_dependency_graph(project_code):
    symbols = symbol_extractor.extract_symbols_from_files(project_code)
    
    nodes = []
    edges = []
    
    created_nodes = set()
    

    for filename in project_code.keys():
        node_id = filename
        if node_id not in created_nodes:
            nodes.append({
                "data": {
                    "id": node_id,
                    "label": filename,
                    "type": "file"
                }
            })
            created_nodes.add(node_id)
            
    for name, sym_data in symbols.items():

        node_id = name
        

        parent_id = sym_data["file"]
        
        if "::" in name:
            class_name = name.split("::")[0]
            if class_name in symbols and symbols[class_name]["type"] in ["class", "struct"]:
                parent_id = class_name 
            
            if parent_id != sym_data["file"] and parent_id not in created_nodes:
                pass

        if node_id not in created_nodes:
            nodes.append({
                "data": {
                    "id": node_id,
                    "label": name.split("::")[-1],
                    "type": sym_data["type"],
                    "parent": parent_id,
                    "line_start": sym_data["line_start"]
                }
            })
            created_nodes.add(node_id)
            

    inheritance_pattern = re.compile(r'class\s+(\w+)\s*:\s*(?:public|private|protected)\s+(\w+)')
    
    for filename, content in project_code.items():
        for match in inheritance_pattern.finditer(content):
            derived_class = match.group(1)
            base_class = match.group(2)
            

            if derived_class in symbols and base_class in symbols:
                edges.append({
                    "data": {
                        "source": derived_class,
                        "target": base_class,
                        "label": "inherits",
                        "type": "inheritance"
                    }
                })


    known_functions = {name for name, data in symbols.items() if data["type"] == "function"}
    
    call_pattern = re.compile(r'\b(\w+(?:::\w+)?)\s*\(')
    
    for caller_name, caller_data in symbols.items():
        if caller_data["type"] != "function":
            continue
            
        body = caller_data.get("body", "")
        if not body:
            continue
            
        called_funcs = set()
        
        for match in call_pattern.finditer(body):
            called_name = match.group(1)
            
            if called_name in ["if", "while", "for", "switch", "catch", "sizeof"]:
                continue
                
            target_id = None
            
            if called_name in known_functions:
                target_id = called_name
                
            else:
                 if "::" in caller_name:
                     cls = caller_name.split("::")[0]
                     potential_full_name = f"{cls}::{called_name}"
                     if potential_full_name in known_functions:
                         target_id = potential_full_name
            

            if target_id and target_id != caller_name:
                called_funcs.add(target_id)
                
        for target in called_funcs:
            edges.append({
                "data": {
                    "source": caller_name,
                    "target": target,
                    "label": "calls",
                    "type": "function_call"
                }
            })
            
    return {
        "nodes": nodes,
        "edges": edges
    }
