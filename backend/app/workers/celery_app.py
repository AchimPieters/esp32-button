from celery import Celery

from app.core.config import settings

celery_app = Celery("signage", broker=settings.redis_url, backend=settings.redis_url)


@celery_app.task(bind=True, autoretry_for=(Exception,), retry_backoff=True, retry_kwargs={"max_retries": 3})
def process_media(self, media_id: int) -> str:
    return f"processed:{media_id}"
