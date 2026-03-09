from sqlalchemy import select
from sqlalchemy.orm import Session

from app.models.entities import Device, DeviceLog, DeviceMetric, Setting


class DeviceRepository:
    def __init__(self, db: Session):
        self.db = db

    def get_by_serial(self, serial_number: str) -> Device | None:
        return self.db.scalar(select(Device).where(Device.serial_number == serial_number))

    def create(self, device: Device) -> Device:
        self.db.add(device)
        self.db.commit()
        self.db.refresh(device)
        return device

    def save(self, device: Device) -> Device:
        self.db.add(device)
        self.db.commit()
        self.db.refresh(device)
        return device

    def add_metric(self, device_id: int, cpu: int, created_by: str) -> DeviceMetric:
        metric = DeviceMetric(device_id=device_id, cpu=cpu, created_by=created_by)
        self.db.add(metric)
        self.db.commit()
        self.db.refresh(metric)
        return metric

    def add_log(self, device_id: int, level: str, message: str, created_by: str) -> DeviceLog:
        log = DeviceLog(device_id=device_id, level=level, message=message, created_by=created_by)
        self.db.add(log)
        self.db.commit()
        self.db.refresh(log)
        return log

    def get_device_settings(self, device_id: int) -> list[Setting]:
        stmt = select(Setting).where(Setting.scope == "device", Setting.device_id == device_id)
        return list(self.db.scalars(stmt).all())
