# Technisch beslisdocument (vastgelegd voor herbouw)

## Doel
Vaste architectuurbeslissingen expliciet maken vóór grootschalige herbouw om scope drift en ad-hoc ontwerp te voorkomen.

## Beslissingen
1. **Backend framework blijft FastAPI**
   - Motivatie: duidelijke async API-architectuur, sterk ecosysteem rond Pydantic/SQLAlchemy, goede developer velocity.

2. **Database wordt PostgreSQL**
   - Motivatie: relationele integriteit, volwassen indexing/querymogelijkheden, geschikt voor multi-tenant enterprise workloads.

3. **Queue wordt Redis + Celery**
   - Motivatie: bewezen patroon voor async taken, retries en worker-opschaling.

4. **Frontend wordt React + TypeScript**
   - Motivatie: typeveiligheid, component-hergebruik en volwassen enterprise tooling.

5. **Observability wordt Prometheus + Grafana + Loki**
   - Motivatie: scheiding metrics/logs, sterke ecosystemen voor dashboards en incidentanalyse.

## Versiebeleid
Vanaf nu hanteren we **Semantic Versioning (SemVer)** voor alle releasable onderdelen:
- `MAJOR`: incompatibele API/protocol of architectuurwijzigingen.
- `MINOR`: achterwaarts compatibele feature-uitbreidingen.
- `PATCH`: bugfixes en kleine verbeteringen zonder contractbreuk.

Aanvullend:
- Release tags volgen `vX.Y.Z`.
- Pre-releases gebruiken suffixen zoals `-alpha.N`, `-beta.N`, `-rc.N`.
- Changelog entries worden per release verplicht bijgehouden.
