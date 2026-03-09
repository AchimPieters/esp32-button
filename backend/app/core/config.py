from pydantic_settings import BaseSettings, SettingsConfigDict


class Settings(BaseSettings):
    app_env: str = "development"
    database_url: str = "postgresql+psycopg://postgres:postgres@localhost:5432/signage"
    redis_url: str = "redis://localhost:6379/0"
    secret_key: str = "change-me"
    access_token_exp_minutes: int = 30
    refresh_token_exp_minutes: int = 60 * 24 * 14
    jwt_algorithm: str = "HS256"
    bcrypt_rounds: int = 12

    model_config = SettingsConfigDict(env_file=".env", env_file_encoding="utf-8", extra="ignore")


settings = Settings()
