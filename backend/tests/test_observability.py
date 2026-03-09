from fastapi.testclient import TestClient

from app.main import app


client = TestClient(app)


def test_metrics_endpoint_exposes_prometheus_output() -> None:
    client.get("/health")
    response = client.get("/metrics")
    assert response.status_code == 200
    assert "text/plain" in response.headers["content-type"]
    assert "api_requests_total" in response.text


def test_request_context_headers_are_set() -> None:
    response = client.get("/ready")
    assert response.status_code == 200
    assert "x-request-id" in response.headers
    assert "x-correlation-id" in response.headers
    assert "x-latency-ms" in response.headers
