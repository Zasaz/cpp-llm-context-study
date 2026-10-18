import os
from werkzeug.utils import secure_filename

ALLOWED_EXTENSIONS = {'cpp', 'cxx', 'cc'}

def allowed_file(filename):
    return '.' in filename and \
           filename.rsplit('.', 1)[1].lower() in ALLOWED_EXTENSIONS

def save_uploaded_file(file, upload_folder_path): 
    if file and allowed_file(file.filename):
        filename = secure_filename(file.filename)
        filepath = os.path.join(upload_folder_path, filename)
        try:
            os.makedirs(upload_folder_path, exist_ok=True)
            file.save(filepath)
            return filepath
        except Exception as e:
            print(f"Error saving file '{filename}' to '{filepath}': {e}")
            return None
    return None