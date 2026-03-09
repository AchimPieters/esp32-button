# Enterprise Signage Platform (rebuild track)

Deze branch bevat de enterprise-herbouw opzet.

## Project layout
- `backend/`: FastAPI + SQLAlchemy + Alembic + Celery basis
- `frontend/`: placeholder voor React + TypeScript app
- `agent/`: device/embedded agentcode (huidige ESP32 component onder `agent/embedded/`)
- `deploy/`: deployment artefacten (placeholder)
- `observability/`: monitoring/logging artefacten (placeholder)
- `docs/`: baseline, architectuurbesluiten en voortgang

## Start backend (dev)
```bash
cd backend
uvicorn app.main:app --reload
```
