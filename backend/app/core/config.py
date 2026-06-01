from pydantic_settings import BaseSettings


class Settings(BaseSettings):
    app_name: str = "CoDriver"
    supabase_url: str = ""
    supabase_anon_key: str = ""
    qwen_api_key: str = ""
    llama_api_key: str = ""

    class Config:
        env_file = ".env"
        env_file_encoding = "utf-8"


settings = Settings()
