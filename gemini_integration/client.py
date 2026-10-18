import json
import os
import threading
import time

from google import genai

try:
    from dotenv import load_dotenv

    load_dotenv()
except ImportError:
    pass

API_KEY = os.getenv("GOOGLE_API_KEY")

if not API_KEY:
    raise ValueError("GOOGLE_API_KEY environment variable not set.")

DEFAULT_MODEL = "gemini-3.7-flash"
MODEL_NAME = os.getenv("GEMINI_MODEL", DEFAULT_MODEL)

client = genai.Client(api_key=API_KEY)

feature_usage = {}
_call_log = []
_lock = threading.Lock()

RETAIN_TEXT = False
GENERATION_PARAMS = None


def _record(entry):
    with _lock:
        _call_log.append(entry)


def _build_config():
    if not GENERATION_PARAMS:
        return None
    from google.genai import types

    return types.GenerateContentConfig(**GENERATION_PARAMS)


def call_gemini_api(prompt, feature_name="general"):
    with _lock:
        feature_usage[feature_name] = feature_usage.get(feature_name, 0) + 1

    entry = {
        "timestamp": time.time(),
        "feature": feature_name,
        "model_requested": MODEL_NAME,
        "prompt_chars": len(prompt),
        "generation_params": dict(GENERATION_PARAMS) if GENERATION_PARAMS else None,
        "ok": False,
    }
    if RETAIN_TEXT:
        entry["prompt"] = prompt
    started = time.perf_counter()

    try:
        response = client.models.generate_content(
            model=MODEL_NAME,
            contents=prompt,
            config=_build_config(),
        )
    except Exception as e:
        entry["latency_s"] = time.perf_counter() - started
        entry["error"] = str(e)
        _record(entry)
        print(f"Error calling Gemini API: {e}")
        raise

    entry["latency_s"] = time.perf_counter() - started
    entry["model_returned"] = getattr(response, "model_version", None)

    usage = getattr(response, "usage_metadata", None)
    if usage is not None:
        entry["prompt_tokens"] = usage.prompt_token_count
        entry["output_tokens"] = usage.candidates_token_count
        entry["thought_tokens"] = usage.thoughts_token_count
        entry["cached_tokens"] = usage.cached_content_token_count
        entry["total_tokens"] = usage.total_token_count

    if response.parts:
        entry["ok"] = True
        text = response.text
        if RETAIN_TEXT:
            entry["response"] = text
        _record(entry)
        return text

    entry["error"] = f"empty or blocked: {response.prompt_feedback}"
    entry["refused"] = True
    _record(entry)
    print(f"Gemini response was empty or blocked. Feedback: {response.prompt_feedback}")
    return "Could not generate a response. It might have been blocked due to safety settings or other issues."


def get_feature_usage():
    return feature_usage.copy()


def get_call_log():
    with _lock:
        return list(_call_log)


def reset_call_log():
    with _lock:
        _call_log.clear()


def dump_call_log(path):
    entries = get_call_log()
    with open(path, "w", encoding="utf-8") as fh:
        for entry in entries:
            fh.write(json.dumps(entry) + "\n")
    return len(entries)
