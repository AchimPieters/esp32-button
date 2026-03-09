from pydantic import BaseModel, Field


class SettingCreate(BaseModel):
    scope: str = Field(pattern="^(global|organization|device)$")
    key: str
    value: dict
    organization_id: int | None = None
    device_id: int | None = None
    actor_id: str


class SettingRollback(BaseModel):
    setting_key: str
    target_version: int
    actor_id: str


class SettingRead(BaseModel):
    id: int
    scope: str
    key: str
    value: dict
    version: int

    class Config:
        from_attributes = True
