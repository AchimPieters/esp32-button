"""auth rbac and audit guards

Revision ID: 0002_auth_rbac_and_audit_guards
Revises: 0001_initial_schema
Create Date: 2026-03-09
"""

from alembic import op
import sqlalchemy as sa

revision = "0002_auth_rbac_and_audit_guards"
down_revision = "0001_initial_schema"
branch_labels = None
depends_on = None


def add_common_cols(table: str) -> None:
    op.add_column(table, sa.Column("created_at", sa.DateTime(timezone=True), server_default=sa.func.now(), nullable=False))
    op.add_column(table, sa.Column("updated_at", sa.DateTime(timezone=True), server_default=sa.func.now(), nullable=False))
    op.add_column(table, sa.Column("created_by", sa.String(), nullable=True))
    op.add_column(table, sa.Column("deleted_at", sa.DateTime(timezone=True), nullable=True))


def upgrade() -> None:
    op.add_column("users", sa.Column("is_locked", sa.Boolean(), nullable=False, server_default=sa.false()))
    op.add_column("users", sa.Column("failed_login_attempts", sa.Integer(), nullable=False, server_default="0"))

    op.create_table(
        "user_roles",
        sa.Column("id", sa.Integer(), primary_key=True),
        sa.Column("user_id", sa.Integer(), sa.ForeignKey("users.id"), nullable=False),
        sa.Column("role_id", sa.Integer(), sa.ForeignKey("roles.id"), nullable=False),
    )
    add_common_cols("user_roles")
    op.create_unique_constraint("uq_user_roles", "user_roles", ["user_id", "role_id"])

    op.create_table(
        "role_permissions",
        sa.Column("id", sa.Integer(), primary_key=True),
        sa.Column("role_id", sa.Integer(), sa.ForeignKey("roles.id"), nullable=False),
        sa.Column("permission_id", sa.Integer(), sa.ForeignKey("permissions.id"), nullable=False),
    )
    add_common_cols("role_permissions")
    op.create_unique_constraint("uq_role_permissions", "role_permissions", ["role_id", "permission_id"])

    op.create_table(
        "refresh_tokens",
        sa.Column("id", sa.Integer(), primary_key=True),
        sa.Column("user_id", sa.Integer(), sa.ForeignKey("users.id"), nullable=False),
        sa.Column("token_hash", sa.String(255), nullable=False, unique=True),
        sa.Column("expires_at", sa.DateTime(timezone=True), nullable=False),
        sa.Column("is_revoked", sa.Boolean(), nullable=False, server_default=sa.false()),
    )
    add_common_cols("refresh_tokens")
    op.create_index("ix_refresh_tokens_user_revoked", "refresh_tokens", ["user_id", "is_revoked"])

    op.execute(
        """
        CREATE OR REPLACE FUNCTION prevent_audit_logs_modification()
        RETURNS trigger AS $$
        BEGIN
            RAISE EXCEPTION 'audit_logs is immutable';
        END;
        $$ LANGUAGE plpgsql;
        """
    )
    op.execute(
        """
        CREATE TRIGGER trg_audit_logs_immutable_update
        BEFORE UPDATE ON audit_logs
        FOR EACH ROW EXECUTE FUNCTION prevent_audit_logs_modification();
        """
    )
    op.execute(
        """
        CREATE TRIGGER trg_audit_logs_immutable_delete
        BEFORE DELETE ON audit_logs
        FOR EACH ROW EXECUTE FUNCTION prevent_audit_logs_modification();
        """
    )


def downgrade() -> None:
    op.execute("DROP TRIGGER IF EXISTS trg_audit_logs_immutable_delete ON audit_logs")
    op.execute("DROP TRIGGER IF EXISTS trg_audit_logs_immutable_update ON audit_logs")
    op.execute("DROP FUNCTION IF EXISTS prevent_audit_logs_modification")

    op.drop_index("ix_refresh_tokens_user_revoked", table_name="refresh_tokens")
    op.drop_table("refresh_tokens")
    op.drop_constraint("uq_role_permissions", "role_permissions", type_="unique")
    op.drop_table("role_permissions")
    op.drop_constraint("uq_user_roles", "user_roles", type_="unique")
    op.drop_table("user_roles")
    op.drop_column("users", "failed_login_attempts")
    op.drop_column("users", "is_locked")
