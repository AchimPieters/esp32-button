from datetime import datetime, timedelta, timezone

from app.services.device_protocol_service import derive_device_status


def test_derive_device_status_online() -> None:
    assert derive_device_status(datetime.now(timezone.utc) - timedelta(seconds=30)) == "online"


def test_derive_device_status_stale() -> None:
    assert derive_device_status(datetime.now(timezone.utc) - timedelta(seconds=180)) == "stale"


def test_derive_device_status_offline() -> None:
    assert derive_device_status(datetime.now(timezone.utc) - timedelta(seconds=600)) == "offline"


def test_derive_device_status_error_wins() -> None:
    assert derive_device_status(datetime.now(timezone.utc), error=True) == "error"
