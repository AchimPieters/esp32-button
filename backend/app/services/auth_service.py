from datetime import datetime, timedelta, timezone

from app.core.config import settings
from app.core.security import create_token, decode_token, hash_password, validate_password_policy, verify_password
from app.models.entities import RefreshToken
from app.repositories.user_repository import RefreshTokenRepository, UserRepository


class AuthError(Exception):
    pass


class AuthService:
    def __init__(self, db):
        self.user_repo = UserRepository(db)
        self.refresh_repo = RefreshTokenRepository(db)

    def login(self, email: str, password: str) -> tuple[str, str]:
        user = self.user_repo.get_by_email(email)
        if not user:
            raise AuthError("Invalid credentials")
        if user.is_locked:
            raise AuthError("Account locked")
        if not verify_password(password, user.password_hash):
            user.failed_login_attempts += 1
            if user.failed_login_attempts >= 5:
                user.is_locked = True
            self.user_repo.save(user)
            raise AuthError("Invalid credentials")

        user.failed_login_attempts = 0
        self.user_repo.save(user)

        access = create_token(str(user.id), "access", settings.access_token_exp_minutes)
        refresh = create_token(str(user.id), "refresh", settings.refresh_token_exp_minutes)
        token = RefreshToken(
            user_id=user.id,
            token_hash=hash_password(refresh),
            expires_at=datetime.now(timezone.utc) + timedelta(minutes=settings.refresh_token_exp_minutes),
            created_by=str(user.id),
        )
        self.refresh_repo.create(token)
        return access, refresh

    def refresh(self, refresh_token: str) -> tuple[str, str]:
        try:
            payload = decode_token(refresh_token)
        except ValueError as exc:
            raise AuthError("Invalid refresh token") from exc

        if payload.get("type") != "refresh":
            raise AuthError("Invalid token type")

        user_id = int(payload["sub"])
        active_tokens = self.refresh_repo.list_active_by_user(user_id)
        current_token = next((token for token in active_tokens if verify_password(refresh_token, token.token_hash)), None)
        if current_token is None:
            raise AuthError("Refresh token revoked or unknown")

        self.refresh_repo.revoke(current_token)
        access = create_token(str(user_id), "access", settings.access_token_exp_minutes)
        new_refresh = create_token(str(user_id), "refresh", settings.refresh_token_exp_minutes)
        self.refresh_repo.create(
            RefreshToken(
                user_id=user_id,
                token_hash=hash_password(new_refresh),
                expires_at=datetime.now(timezone.utc) + timedelta(minutes=settings.refresh_token_exp_minutes),
                created_by=str(user_id),
            )
        )
        return access, new_refresh

    def logout(self, refresh_token: str) -> None:
        try:
            payload = decode_token(refresh_token)
        except ValueError as exc:
            raise AuthError("Invalid refresh token") from exc
        if payload.get("type") != "refresh":
            raise AuthError("Invalid token type")
        user_id = int(payload["sub"])
        active_tokens = self.refresh_repo.list_active_by_user(user_id)
        current_token = next((token for token in active_tokens if verify_password(refresh_token, token.token_hash)), None)
        if current_token is None:
            raise AuthError("Refresh token revoked or unknown")
        self.refresh_repo.revoke(current_token)


def ensure_strong_password(password: str) -> None:
    if not validate_password_policy(password):
        raise AuthError("Password does not satisfy policy")
