# 1. Start with a lightweight official Python image
FROM python:3.11-slim

# 2. Install the g++ compiler and build tools
# This is the magic line that installs your C++ dependency
RUN apt-get update && apt-get install -y build-essential

# 3. Set up a working directory in the container
WORKDIR /app

# 4. Copy and install your Python requirements
COPY requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt

# 5. Copy your entire application code into the container
COPY . .

# 6. Expose the port Render will use (Render expects 10000)
EXPOSE 10000

# 7. Run the application using Gunicorn (a production web server)
# "main:app" means "in main.py, find the variable named app"
CMD ["gunicorn", "--bind", "0.0.0.0:10000", "main:app"]