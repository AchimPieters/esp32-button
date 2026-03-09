# Rebuild status (enterprise roadmap)

Dit document volgt de 27 gevraagde stappen en markeert de huidige voortgang.

## Voltooid / substantieel gestart
1. Baseline + architectuurbesluiten vastgelegd (`docs/current-state.md`, `docs/technical-decisions.md`).
2. Repositorystructuur opgezet met `backend/`, `frontend/`, `agent/`, `deploy/`, `.github/`, `observability/`.
3. Persistentielaag gestart: PostgreSQL + SQLAlchemy modellen + Alembic migraties (`0001`, `0002`).
4. Centraal settingssysteem gestart: settings model, schema-validatie, service met versionering, rollback endpoint en audit logging.
5. Enterprise-auth uitgebreid: login + refresh + logout met tokenrotatie/revocation, account locking op failed logins, JWT uitgifte en password policy helper.
6. RBAC basis gestart: rollen/permissions-userkoppelingen + permission dependency op settings routes.
7. Audit logging gestart met immutable DB-protectie via PostgreSQL triggers op `audit_logs`.
16. Async workers gestart: Redis + Celery basisworker toegevoegd.
18. Enterprise observability gestart: `/health`, `/ready`, `/metrics`, Prometheus request counters/latency en request/correlation-id middleware.
23. Teststrategie gestart met eerste backend unit tests voor security-basics.
24. CI/CD quality gates gestart: GitHub Actions backend workflow toegevoegd voor lint (`ruff`) en tests (`pytest`).

## Nog niet volledig afgerond
8. Formeel device protocol v1 gestart: register/heartbeat/metrics/logs/config-fetch/command-fetch/command-ack/playback-status/screenshot-upload routes + schema's + documentatie (`docs/device-protocol.md`).

9 t/m 15, 17 t/m 22, 25 t/m 27: nog in uitvoering. Vereist extra iteraties voor volledige enterprise-implementatie inclusief robuuste agentplayer, mediapipeline, scheduling/layout/fleet/alerts, OTA, observability dashboards, frontend enterprise UI, multi-tenant hardening, security/deployment hardening en finale acceptatiefase.
