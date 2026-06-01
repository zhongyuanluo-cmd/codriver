"""CoDriver LLM client - 裸 HTTP + instructor 调用云端 LLM"""

# TODO: Implement Qwen/Llama client
# import instructor
# from openai import AsyncOpenAI
#
# client = instructor.from_openai(AsyncOpenAI(
#     base_url="https://dashscope.aliyuncs.com/compatible-mode/v1",
#     api_key="...",
# ))

async def chat(prompt: str, region: str = "cn") -> str:
    """Send chat request to region-appropriate LLM"""
    # TODO: Route to Qwen (cn) or Llama (overseas)
    return "LLM integration not yet configured"
