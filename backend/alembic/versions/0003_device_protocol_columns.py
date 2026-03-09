"""device protocol columns

Revision ID: 0003_device_protocol_columns
Revises: 0002_auth_rbac_and_audit_guards
Create Date: 2026-03-09
"""

from alembic import op
import sqlalchemy as sa

revision = "0003_device_protocol_columns"
down_revision = "0002_auth_rbac_and_audit_guards"
branch_labels = None
depends_on = None


def upgrade() -> None:
    op.add_column("devices", sa.Column("agent_version", sa.String(length=64), nullable=True))
    op.add_column("devices", sa.Column("last_heartbeat_at", sa.DateTime(timezone=True), nullable=True))


def downgrade() -> None:
    op.drop_column("devices", "last_heartbeat_at")
    op.drop_column("devices", "agent_version")
