from fastapi import APIRouter

router = APIRouter(tags=["health"])


@router.get("/v1/health")
def health() -> dict[str, str]:
    return {"status": "ok"}
