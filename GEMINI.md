# Gemini Configuration

This file provides context for the Gemini AI assistant to understand and interact with this project.

## Project Overview

This is a C++ code analyzer web application. It allows users to upload C++ code and perform various analyses, such as:
- Explaining the code in natural language.
- Checking for performance issues.
- Suggesting optimizations.
- Simulating the code's execution.
- Generating unit tests.

## Tech Stack

- **Backend:** Python with Flask
- **Frontend:** HTML, CSS, JavaScript
- **AI Integration:** Google Gemini

## Project Structure

- `main.py`: The main Flask application file.
- `analyzer/`: Contains the logic for code analysis.
- `cpp_handler/`: Handles C++ code compilation, execution, and sandboxing.
- `gemini_integration/`: Manages interaction with the Gemini API.
- `templates/`: HTML templates for the web interface.
- `static/`: Static assets (CSS, JavaScript).
- `uploads/`: Directory for storing uploaded C++ files.
- `tests/`: Contains tests for the application.

## Development Guidelines

- Follow PEP 8 for Python code.
- Use the existing directory structure for new files.
- Ensure that any changes to the backend are accompanied by corresponding tests.
