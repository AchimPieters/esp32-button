import pytest

from app.core.security import create_token, decode_token, validate_password_policy


def test_password_policy_rejects_weak_password() -> None:
    assert validate_password_policy("weakpass") is False


def test_password_policy_accepts_strong_password() -> None:
    assert validate_password_policy("Str0ng!Password") is True


def test_create_token_returns_jwt_string() -> None:
    token = create_token("123", "access", 10)
    assert isinstance(token, str)
    assert token.count(".") == 2


def test_decode_token_roundtrip() -> None:
    token = create_token("321", "refresh", 10)
    payload = decode_token(token)
    assert payload["sub"] == "321"
    assert payload["type"] == "refresh"


def test_decode_token_rejects_invalid_token() -> None:
    with pytest.raises(ValueError):
        decode_token("broken.token.value")
