from datetime import datetime, timedelta, timezone

from app.models.entities import Device
from app.repositories.device_repository import DeviceRepository


HEARTBEAT_TIMEOUT_SECONDS = 120
STALE_THRESHOLD_SECONDS = 300


def derive_device_status(last_heartbeat_at: datetime | None, error: bool = False) -> str:
    if error:
        return "error"
    if last_heartbeat_at is None:
        return "offline"

    age = datetime.now(timezone.utc) - last_heartbeat_at
    if age <= timedelta(seconds=HEARTBEAT_TIMEOUT_SECONDS):
        return "online"
    if age <= timedelta(seconds=STALE_THRESHOLD_SECONDS):
        return "stale"
    return "offline"


class DeviceProtocolService:
    def __init__(self, db):
        self.repo = DeviceRepository(db)

    def register(self, organization_id: int, serial_number: str, agent_version: str, capabilities: dict) -> Device:
        device = self.repo.get_by_serial(serial_number)
        if device is None:
            device = Device(
                organization_id=organization_id,
                serial_number=serial_number,
                agent_version=agent_version,
                capabilities=capabilities,
                status="offline",
                created_by=serial_number,
            )
            return self.repo.create(device)

        device.agent_version = agent_version
        device.capabilities = capabilities
        device.updated_at = datetime.now(timezone.utc)
        return self.repo.save(device)

    def heartbeat(self, serial_number: str, has_error: bool = False) -> Device:
        device = self.repo.get_by_serial(serial_number)
        if device is None:
            raise ValueError("Unknown device")

        device.last_heartbeat_at = datetime.now(timezone.utc)
        device.status = derive_device_status(device.last_heartbeat_at, error=has_error)
        return self.repo.save(device)

    def push_metrics(self, serial_number: str, cpu: int) -> None:
        device = self.repo.get_by_serial(serial_number)
        if device is None:
            raise ValueError("Unknown device")
        self.repo.add_metric(device.id, cpu, serial_number)

    def push_logs(self, serial_number: str, level: str, message: str) -> None:
        device = self.repo.get_by_serial(serial_number)
        if device is None:
            raise ValueError("Unknown device")
        self.repo.add_log(device.id, level, message, serial_number)

    def fetch_config(self, serial_number: str) -> dict:
        device = self.repo.get_by_serial(serial_number)
        if device is None:
            raise ValueError("Unknown device")
        settings = self.repo.get_device_settings(device.id)
        return {setting.key: setting.value for setting in settings}
