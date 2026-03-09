"""initial schema

Revision ID: 0001_initial_schema
Revises:
Create Date: 2026-03-09
"""

from alembic import op
import sqlalchemy as sa
from sqlalchemy.dialects import postgresql

revision = "0001_initial_schema"
down_revision = None
branch_labels = None
depends_on = None


def add_common_cols(table: str) -> None:
    op.add_column(table, sa.Column("created_at", sa.DateTime(timezone=True), server_default=sa.func.now(), nullable=False))
    op.add_column(table, sa.Column("updated_at", sa.DateTime(timezone=True), server_default=sa.func.now(), nullable=False))
    op.add_column(table, sa.Column("created_by", sa.String(), nullable=True))
    op.add_column(table, sa.Column("deleted_at", sa.DateTime(timezone=True), nullable=True))


def upgrade() -> None:
    op.create_table("organizations", sa.Column("id", sa.Integer(), primary_key=True), sa.Column("name", sa.String(255), nullable=False, unique=True))
    add_common_cols("organizations")

    op.create_table("users", sa.Column("id", sa.Integer(), primary_key=True), sa.Column("organization_id", sa.Integer(), sa.ForeignKey("organizations.id"), nullable=False), sa.Column("email", sa.String(255), nullable=False, unique=True), sa.Column("password_hash", sa.String(255), nullable=False), sa.Column("is_active", sa.Boolean(), nullable=False, server_default=sa.true()))
    add_common_cols("users")

    for t, cols in {
        "roles": [sa.Column("id", sa.Integer(), primary_key=True), sa.Column("name", sa.String(64), unique=True, nullable=False)],
        "permissions": [sa.Column("id", sa.Integer(), primary_key=True), sa.Column("code", sa.String(100), unique=True, nullable=False)],
        "device_groups": [sa.Column("id", sa.Integer(), primary_key=True), sa.Column("organization_id", sa.Integer(), sa.ForeignKey("organizations.id"), nullable=False), sa.Column("name", sa.String(255), nullable=False)],
        "devices": [sa.Column("id", sa.Integer(), primary_key=True), sa.Column("organization_id", sa.Integer(), sa.ForeignKey("organizations.id"), nullable=False), sa.Column("group_id", sa.Integer(), sa.ForeignKey("device_groups.id"), nullable=True), sa.Column("serial_number", sa.String(128), unique=True, nullable=False), sa.Column("status", sa.String(30), nullable=False), sa.Column("capabilities", postgresql.JSONB(astext_type=sa.Text()), nullable=False, server_default=sa.text("'{}'::jsonb"))],
        "device_tags": [sa.Column("id", sa.Integer(), primary_key=True), sa.Column("organization_id", sa.Integer(), sa.ForeignKey("organizations.id"), nullable=False), sa.Column("name", sa.String(64), nullable=False)],
        "media": [sa.Column("id", sa.Integer(), primary_key=True), sa.Column("organization_id", sa.Integer(), sa.ForeignKey("organizations.id"), nullable=False), sa.Column("storage_key", sa.String(255), nullable=False, unique=True), sa.Column("checksum", sa.String(128), nullable=True)],
        "media_versions": [sa.Column("id", sa.Integer(), primary_key=True), sa.Column("media_id", sa.Integer(), sa.ForeignKey("media.id"), nullable=False), sa.Column("version", sa.Integer(), nullable=False)],
        "playlists": [sa.Column("id", sa.Integer(), primary_key=True), sa.Column("organization_id", sa.Integer(), sa.ForeignKey("organizations.id"), nullable=False), sa.Column("name", sa.String(255), nullable=False)],
        "playlist_items": [sa.Column("id", sa.Integer(), primary_key=True), sa.Column("playlist_id", sa.Integer(), sa.ForeignKey("playlists.id"), nullable=False), sa.Column("media_id", sa.Integer(), sa.ForeignKey("media.id"), nullable=True), sa.Column("sort_order", sa.Integer(), nullable=False)],
        "schedules": [sa.Column("id", sa.Integer(), primary_key=True), sa.Column("organization_id", sa.Integer(), sa.ForeignKey("organizations.id"), nullable=False), sa.Column("playlist_id", sa.Integer(), sa.ForeignKey("playlists.id"), nullable=False), sa.Column("recurrence", sa.String(128), nullable=True)],
        "layouts": [sa.Column("id", sa.Integer(), primary_key=True), sa.Column("organization_id", sa.Integer(), sa.ForeignKey("organizations.id"), nullable=False), sa.Column("name", sa.String(255), nullable=False)],
        "zones": [sa.Column("id", sa.Integer(), primary_key=True), sa.Column("layout_id", sa.Integer(), sa.ForeignKey("layouts.id"), nullable=False), sa.Column("name", sa.String(255), nullable=False)],
        "settings": [sa.Column("id", sa.Integer(), primary_key=True), sa.Column("scope", sa.String(32), nullable=False), sa.Column("organization_id", sa.Integer(), sa.ForeignKey("organizations.id"), nullable=True), sa.Column("device_id", sa.Integer(), sa.ForeignKey("devices.id"), nullable=True), sa.Column("key", sa.String(128), nullable=False), sa.Column("value", postgresql.JSONB(astext_type=sa.Text()), nullable=False), sa.Column("version", sa.Integer(), nullable=False)],
        "alerts": [sa.Column("id", sa.Integer(), primary_key=True), sa.Column("organization_id", sa.Integer(), sa.ForeignKey("organizations.id"), nullable=False), sa.Column("device_id", sa.Integer(), sa.ForeignKey("devices.id"), nullable=True), sa.Column("severity", sa.String(20), nullable=False), sa.Column("message", sa.Text(), nullable=False)],
        "device_metrics": [sa.Column("id", sa.Integer(), primary_key=True), sa.Column("device_id", sa.Integer(), sa.ForeignKey("devices.id"), nullable=False), sa.Column("cpu", sa.Integer(), nullable=True)],
        "device_logs": [sa.Column("id", sa.Integer(), primary_key=True), sa.Column("device_id", sa.Integer(), sa.ForeignKey("devices.id"), nullable=False), sa.Column("level", sa.String(20), nullable=True), sa.Column("message", sa.Text(), nullable=True)],
        "update_jobs": [sa.Column("id", sa.Integer(), primary_key=True), sa.Column("organization_id", sa.Integer(), sa.ForeignKey("organizations.id"), nullable=False), sa.Column("channel", sa.String(20), nullable=False), sa.Column("status", sa.String(20), nullable=False)],
        "deployments": [sa.Column("id", sa.Integer(), primary_key=True), sa.Column("update_job_id", sa.Integer(), sa.ForeignKey("update_jobs.id"), nullable=False), sa.Column("device_id", sa.Integer(), sa.ForeignKey("devices.id"), nullable=False), sa.Column("status", sa.String(20), nullable=False)],
    }.items():
        op.create_table(t, *cols)
        add_common_cols(t)

    op.create_table(
        "audit_logs",
        sa.Column("id", sa.Integer(), primary_key=True),
        sa.Column("actor_type", sa.String(32), nullable=False),
        sa.Column("actor_id", sa.String(64), nullable=False),
        sa.Column("organization_id", sa.Integer(), sa.ForeignKey("organizations.id"), nullable=True),
        sa.Column("action", sa.String(128), nullable=False),
        sa.Column("resource_type", sa.String(64), nullable=False),
        sa.Column("resource_id", sa.String(64), nullable=False),
        sa.Column("before_state", postgresql.JSONB(astext_type=sa.Text()), nullable=True),
        sa.Column("after_state", postgresql.JSONB(astext_type=sa.Text()), nullable=True),
        sa.Column("ip_address", sa.String(64), nullable=True),
        sa.Column("user_agent", sa.String(255), nullable=True),
        sa.Column("created_at", sa.DateTime(timezone=True), server_default=sa.func.now(), nullable=False),
    )

    op.create_index("ix_devices_org_status", "devices", ["organization_id", "status"])
    op.create_index("ix_settings_scope", "settings", ["scope", "organization_id", "device_id"])
    op.create_unique_constraint("uq_device_tags_org_name", "device_tags", ["organization_id", "name"])


def downgrade() -> None:
    for t in ["audit_logs", "deployments", "update_jobs", "device_logs", "device_metrics", "alerts", "settings", "zones", "layouts", "schedules", "playlist_items", "playlists", "media_versions", "media", "device_tags", "devices", "device_groups", "permissions", "roles", "users", "organizations"]:
        op.drop_table(t)
