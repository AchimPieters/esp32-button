# Huidige technische baseline (bevroren)

## Baseline metadata
- Repository: `esp32-button`
- Baseline branch voor herbouw: `enterprise-platform`
- Huidige componentversie: `1.2.3` (`idf_component.yml`)
- Domein: ESP-IDF C-library voor knopinput, geen backend/frontend platform.

## Inventarisatie huidige codebase

### Backend endpoints
- Niet aanwezig. Er is geen HTTP server, geen API-routes, geen auth of persistence-laag.

### Agent functionaliteit
- Niet aanwezig als afzonderlijke agent/service.
- Wel aanwezig: embedded runtime-logica in `button.c` en `toggle.c` voor inputdetectie en events.

### Webpagina's
- Niet aanwezig.

### Docker setup
- Niet aanwezig (`Dockerfile`, `docker-compose` ontbreken).

### Settings
- Runtime-configuratie via `button_config_t` (active level, long press, repeat timeout, max repeats).
- Build/config context via ESP-IDF `menuconfig` in voorbeeld/README.
- Geen centrale settingsopslag, geen versionering, geen audit.

### Device gedrag
- GPIO toggles worden via interrupt + debounce timer verwerkt.
- Button-logica ondersteunt: single, double, triple en long press events.
- Per GPIO wordt state in memory bijgehouden met statische pools en mutexen.
- Teststub aanwezig voor toggle create/delete gedrag.

## Korte architectuurbeschrijving
- **Kernmodules**:
  - `toggle.c/.h`: GPIO edge detectie, debounce, ISR-handling.
  - `button.c/.h`: eventaggregatie bovenop toggle-signalen (multi-press + long press).
  - `port.c/.h`: GPIO-portabiliteitslaag voor pin setup/read/pullup/pulldown.
- **Build/distributie**:
  - ESP-IDF componentregistratie (`CMakeLists.txt`, `component.mk`, `idf_component.yml`).
  - Dist artifacts van eerdere releases in `dist/`.
- **Testvorm**:
  - Simpele C test met stubs onder `tests/`.

## Beperkingen in huidige situatie
- Geen server-side architectuur (geen API, database, queue, web UI).
- Geen enterprise security (geen authN/authZ, auditing, rate limits, MFA).
- Geen multi-tenant concept, organisaties of RBAC.
- Geen observability stack (Prometheus/Grafana/Loki) of health endpoints.
- Geen OTA, fleet management, scheduler, playlist engine of media pipeline.
- Geen CI/CD quality gates voor modern full-stack platformdoelen.

## Features die behouden moeten blijven
1. ESP-IDF component-compatibiliteit (CMake/component.mk/idf_component.yml).
2. Betrouwbare GPIO interruptverwerking en debounce.
3. Ondersteuning voor single/double/triple/long press events.
4. Callback-gedreven API voor integratie in embedded apps.
5. Defensieve foutcodes voor create/destroy flows.
6. Basale testbaarheid via stubs en geautomatiseerde check.

## Ontbrekende enterprise-features (gap t.o.v. doelarchitectuur)
- Backend/API platform (FastAPI), datamodel en persistente opslag.
- PostgreSQL + SQLAlchemy + Alembic migraties.
- Redis/Celery workers, async jobverwerking.
- React/TypeScript frontend met beheerpanelen.
- Auth module (OAuth2/JWT, refresh, reset, lockout, MFA optioneel).
- RBAC/permissions enforcement in backend + frontend.
- Audit logging en immutable history.
- Formeel versioned device protocol.
- Robuuste agent-player architectuur (offline/fallback/recovery/update).
- Media pipeline (validatie/scanning/transcoding/versioning).
- Playlist/scheduling/layout engines.
- Fleet management, alerts, incident workflows.
- OTA update-infrastructuur.
- Enterprise observability en runbooks.
- Multi-tenant isolatie.
- End-to-end security hardening + deployment hardening.
- Volledige teststrategie + CI/CD + repo hygiene/documentatie.

## Scope-opmerking
Deze baseline bevestigt dat de huidige repository een **embedded component** is en nog niet de gevraagde enterprise signage platformbasis bevat. Verdere stappen vereisen substantiële greenfield-uitbreiding en herstructurering bovenop of naast de bestaande C-library.
