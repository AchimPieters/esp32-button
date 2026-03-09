from fastapi import APIRouter, Depends, HTTPException
from sqlalchemy.orm import Session

from app.db.session import get_db
from app.schemas.device_protocol import (
    DeviceCommandAckRequest,
    DeviceCommandRequest,
    DeviceConfigRequest,
    DeviceHeartbeatRequest,
    DeviceLogRequest,
    DeviceMetricRequest,
    DevicePlaybackStatusRequest,
    DeviceRegisterRequest,
    DeviceResponse,
)
from app.services.device_protocol_service import DeviceProtocolService

router = APIRouter(prefix="/v1/device-protocol", tags=["device-protocol"])


@router.post("/register", response_model=DeviceResponse)
def register(payload: DeviceRegisterRequest, db: Session = Depends(get_db)) -> DeviceResponse:
    service = DeviceProtocolService(db)
    service.register(payload.organization_id, payload.serial_number, payload.agent_version, payload.capabilities)
    return DeviceResponse(status="registered")


@router.post("/heartbeat", response_model=DeviceResponse)
def heartbeat(payload: DeviceHeartbeatRequest, db: Session = Depends(get_db)) -> DeviceResponse:
    service = DeviceProtocolService(db)
    try:
        device = service.heartbeat(payload.serial_number, has_error=payload.has_error)
    except ValueError as exc:
        raise HTTPException(status_code=404, detail=str(exc)) from exc
    return DeviceResponse(status=device.status)


@router.post("/metrics", response_model=DeviceResponse)
def metrics_push(payload: DeviceMetricRequest, db: Session = Depends(get_db)) -> DeviceResponse:
    service = DeviceProtocolService(db)
    service.push_metrics(payload.serial_number, payload.cpu)
    return DeviceResponse(status="accepted")


@router.post("/logs", response_model=DeviceResponse)
def logs_push(payload: DeviceLogRequest, db: Session = Depends(get_db)) -> DeviceResponse:
    service = DeviceProtocolService(db)
    service.push_logs(payload.serial_number, payload.level, payload.message)
    return DeviceResponse(status="accepted")


@router.post("/screenshot-upload", response_model=DeviceResponse)
def screenshot_upload(payload: DeviceCommandRequest) -> DeviceResponse:
    _ = payload
    return DeviceResponse(status="accepted")


@router.post("/config-fetch")
def config_fetch(payload: DeviceConfigRequest, db: Session = Depends(get_db)) -> dict:
    service = DeviceProtocolService(db)
    return {"settings": service.fetch_config(payload.serial_number)}


@router.post("/command-fetch")
def command_fetch(payload: DeviceCommandRequest) -> dict:
    _ = payload
    return {"commands": []}


@router.post("/command-ack", response_model=DeviceResponse)
def command_ack(payload: DeviceCommandAckRequest) -> DeviceResponse:
    _ = payload
    return DeviceResponse(status="acknowledged")


@router.post("/playback-status", response_model=DeviceResponse)
def playback_status(payload: DevicePlaybackStatusRequest) -> DeviceResponse:
    _ = payload
    return DeviceResponse(status="accepted")
