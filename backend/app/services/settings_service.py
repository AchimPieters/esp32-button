from app.models.entities import AuditLog, Setting
from app.repositories.settings_repository import SettingsRepository
from sqlalchemy.orm import Session
from datetime import datetime, timezone


class SettingsService:
    def __init__(self, db: Session):
        self.db = db
        self.repo = SettingsRepository(db)

    def create_setting(self, payload) -> Setting:
        latest = self.repo.get_latest_by_key(payload.key)
        version = 1 if latest is None else latest.version + 1
        setting = Setting(
            scope=payload.scope,
            key=payload.key,
            value=payload.value,
            version=version,
            organization_id=payload.organization_id,
            device_id=payload.device_id,
            created_by=payload.actor_id,
        )
        created = self.repo.create(setting)
        self._log("settings.change", payload.actor_id, str(created.id), None, created.value)
        return created

    def rollback(self, key: str, version: int, actor_id: str) -> Setting | None:
        target = self.repo.get_by_key_and_version(key, version)
        if target is None:
            return None
        latest = self.repo.get_latest_by_key(key)
        setting = Setting(
            scope=target.scope,
            key=target.key,
            value=target.value,
            version=(latest.version + 1) if latest else 1,
            organization_id=target.organization_id,
            device_id=target.device_id,
            created_by=actor_id,
        )
        created = self.repo.create(setting)
        self._log("settings.rollback", actor_id, str(created.id), latest.value if latest else None, created.value)
        return created

    def _log(self, action: str, actor_id: str, resource_id: str, before_state: dict | None, after_state: dict | None) -> None:
        log = AuditLog(
            actor_type="user",
            actor_id=actor_id,
            action=action,
            resource_type="setting",
            resource_id=resource_id,
            before_state=before_state,
            after_state=after_state,
            created_at=datetime.now(timezone.utc),
        )
        self.db.add(log)
        self.db.commit()
