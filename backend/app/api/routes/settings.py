from fastapi import APIRouter, Depends, HTTPException
from sqlalchemy.orm import Session

from app.core.rbac import require_permission
from app.db.session import get_db
from app.schemas.settings import SettingCreate, SettingRead, SettingRollback
from app.services.settings_service import SettingsService

router = APIRouter(prefix="/v1/settings", tags=["settings"])


@router.post("", response_model=SettingRead)
def create_setting(
    payload: SettingCreate,
    db: Session = Depends(get_db),
    _user_id: int = Depends(require_permission("settings.manage")),
) -> SettingRead:
    service = SettingsService(db)
    return service.create_setting(payload)


@router.post("/rollback", response_model=SettingRead)
def rollback_setting(
    payload: SettingRollback,
    db: Session = Depends(get_db),
    _user_id: int = Depends(require_permission("settings.manage")),
) -> SettingRead:
    service = SettingsService(db)
    setting = service.rollback(payload.setting_key, payload.target_version, payload.actor_id)
    if setting is None:
        raise HTTPException(status_code=404, detail="Setting version not found")
    return setting
