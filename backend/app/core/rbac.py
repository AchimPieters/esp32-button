from fastapi import Depends, HTTPException, status
from fastapi.security import OAuth2PasswordBearer
from sqlalchemy.orm import Session

from app.core.security import decode_token
from app.db.session import get_db
from app.repositories.user_repository import UserRepository

oauth2_scheme = OAuth2PasswordBearer(tokenUrl="/api/v1/auth/login")


def get_current_user_id(token: str = Depends(oauth2_scheme)) -> int:
    try:
        payload = decode_token(token)
        sub = payload.get("sub")
        if payload.get("type") != "access" or sub is None:
            raise ValueError("invalid token")
        return int(sub)
    except ValueError as exc:
        raise HTTPException(status_code=status.HTTP_401_UNAUTHORIZED, detail="Invalid token") from exc


def require_permission(code: str):
    def checker(user_id: int = Depends(get_current_user_id), db: Session = Depends(get_db)) -> int:
        perms = UserRepository(db).list_permissions(user_id)
        if code not in perms:
            raise HTTPException(status_code=status.HTTP_403_FORBIDDEN, detail="Missing permission")
        return user_id

    return checker
