from pydantic import BaseModel, Field


class DeviceRegisterRequest(BaseModel):
    organization_id: int
    serial_number: str
    agent_version: str
    capabilities: dict = Field(default_factory=dict)


class DeviceHeartbeatRequest(BaseModel):
    serial_number: str
    has_error: bool = False


class DeviceMetricRequest(BaseModel):
    serial_number: str
    cpu: int


class DeviceLogRequest(BaseModel):
    serial_number: str
    level: str
    message: str


class DeviceConfigRequest(BaseModel):
    serial_number: str


class DeviceCommandRequest(BaseModel):
    serial_number: str


class DeviceCommandAckRequest(BaseModel):
    serial_number: str
    command_id: str


class DevicePlaybackStatusRequest(BaseModel):
    serial_number: str
    status: str


class DeviceResponse(BaseModel):
    status: str
