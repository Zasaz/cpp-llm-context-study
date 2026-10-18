import os
import sys

try:
    from dotenv import load_dotenv

    load_dotenv()
except ImportError:
    pass

if len(sys.argv) > 1:
    os.environ["GEMINI_MODEL"] = sys.argv[1]

import gemini_integration.client as client

FIELDS = (
    "model_requested",
    "model_returned",
    "prompt_tokens",
    "output_tokens",
    "thought_tokens",
    "cached_tokens",
    "total_tokens",
    "latency_s",
    "ok",
)


def describe_model():
    try:
        info = client.client.models.get(model=client.MODEL_NAME)
    except Exception as e:
        print("could not read model metadata: %s" % e)
        return
    print("  display name     %s" % getattr(info, "display_name", None))
    print("  version          %s" % getattr(info, "version", None))
    print("  input limit      %s" % getattr(info, "input_token_limit", None))
    print("  output limit     %s" % getattr(info, "output_token_limit", None))


def main():
    print("model configured : %s" % client.MODEL_NAME)
    print("api key loaded   : %s" % bool(client.API_KEY))

    print("\nreported by the provider:")
    describe_model()

    try:
        reply = client.call_gemini_api(
            "Reply with exactly the two characters: OK",
            feature_name="verify_model",
        )
    except Exception as e:
        print("\nCALL FAILED: %s" % type(e).__name__)
        print(str(e)[:500])
        return 1

    print("response         : %r" % reply.strip()[:60])
    print("\nrecorded by the instrumentation:")
    entry = client.get_call_log()[-1]
    for field in FIELDS:
        print("  %-16s %s" % (field, entry.get(field)))

    return 0 if entry.get("ok") else 1


if __name__ == "__main__":
    sys.exit(main())
