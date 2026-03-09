# Device protocol v1

Dit document beschrijft het formele versioned device protocol (`v1`) tussen agent en backend.

## Endpoints
Alle endpoints staan onder `/api/v1/device-protocol`.

1. `POST /register`
2. `POST /heartbeat`
3. `POST /metrics`
4. `POST /logs`
5. `POST /screenshot-upload`
6. `POST /config-fetch`
7. `POST /command-fetch`
8. `POST /command-ack`
9. `POST /playback-status`

## Device capabilities
Bij registratie stuurt de agent minimaal:
- OS
- hardware type
- display outputs
- CPU
- memory
- resolution
- agent version
- connectivity

Deze gegevens worden opgeslagen in `devices.capabilities`.

## Status afleiding
Status wordt automatisch afgeleid op basis van heartbeat:
- `online`: heartbeat binnen 120 seconden
- `stale`: heartbeat tussen 120 en 300 seconden
- `offline`: heartbeat ouder dan 300 seconden of nooit gezien
- `error`: expliciete fout via heartbeat payload

## Contractniveau
Payloadcontracten zijn gedefinieerd met Pydantic schema's in `backend/app/schemas/device_protocol.py`.
