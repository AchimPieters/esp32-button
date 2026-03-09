from sqlalchemy import select
from sqlalchemy.orm import Session

from app.models.entities import Setting


class SettingsRepository:
    def __init__(self, db: Session):
        self.db = db

    def create(self, setting: Setting) -> Setting:
        self.db.add(setting)
        self.db.commit()
        self.db.refresh(setting)
        return setting

    def get_latest_by_key(self, key: str) -> Setting | None:
        stmt = select(Setting).where(Setting.key == key).order_by(Setting.version.desc()).limit(1)
        return self.db.scalar(stmt)

    def get_by_key_and_version(self, key: str, version: int) -> Setting | None:
        stmt = select(Setting).where(Setting.key == key, Setting.version == version).limit(1)
        return self.db.scalar(stmt)
