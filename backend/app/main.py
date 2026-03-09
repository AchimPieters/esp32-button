from fastapi import FastAPI
from prometheus_client import CONTENT_TYPE_LATEST, Counter, Histogram, generate_latest
from starlette.responses import Response

from app.api.routes import auth, device_protocol, health, settings
from app.core.logging import RequestContextMiddleware

app = FastAPI(title="Enterprise Signage Platform API", version="0.2.0")
app.add_middleware(RequestContextMiddleware)

REQUEST_COUNT = Counter("api_requests_total", "Total API request count", ["path", "method", "status"])
REQUEST_LATENCY = Histogram("api_request_latency_seconds", "API latency", ["path", "method"])

app.include_router(health.router, prefix="/api")
app.include_router(auth.router, prefix="/api")
app.include_router(settings.router, prefix="/api")
app.include_router(device_protocol.router, prefix="/api")


@app.get("/health")
def healthcheck() -> dict[str, str]:
    return {"status": "ok"}


@app.get("/ready")
def readycheck() -> dict[str, str]:
    return {"status": "ready"}


@app.middleware("http")
async def metrics_middleware(request, call_next):
    import time

    start = time.perf_counter()
    response = await call_next(request)
    elapsed = time.perf_counter() - start
    REQUEST_COUNT.labels(path=request.url.path, method=request.method, status=str(response.status_code)).inc()
    REQUEST_LATENCY.labels(path=request.url.path, method=request.method).observe(elapsed)
    return response


@app.get("/metrics")
def metrics() -> Response:
    return Response(content=generate_latest(), media_type=CONTENT_TYPE_LATEST)
