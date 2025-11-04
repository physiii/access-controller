#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
PROJECT_ROOT=$(cd "${SCRIPT_DIR}/.." && pwd)
SERVICE_NAME="esp32-tunnel"
DEPLOY_DIR="/opt/${SERVICE_NAME}"
SERVICE_USER="${SERVICE_USER:-esp32-tunnel}"
SERVICE_FILE="${PROJECT_ROOT}/systemd/${SERVICE_NAME}.service"
TARGET_SERVICE_FILE="/etc/systemd/system/${SERVICE_NAME}.service"

echo "Installing ${SERVICE_NAME} to ${DEPLOY_DIR}"

if [[ ! -f "${SERVICE_FILE}" ]]; then
  echo "Service unit file not found at ${SERVICE_FILE}" >&2
  exit 1
fi

if ! command -v rsync >/dev/null 2>&1; then
  echo "rsync is required. Install with: sudo apt install -y rsync" >&2
  exit 1
fi

if ! id -u "${SERVICE_USER}" >/dev/null 2>&1; then
  echo "Creating system user ${SERVICE_USER}";
  sudo useradd --system --no-create-home --shell /usr/sbin/nologin "${SERVICE_USER}"
fi

echo "Syncing project files to ${DEPLOY_DIR}"
sudo mkdir -p "${DEPLOY_DIR}"
sudo rsync -a --delete --exclude node_modules --exclude ".env" "${PROJECT_ROOT}/" "${DEPLOY_DIR}/"

if [[ ! -f "${DEPLOY_DIR}/.env" && -f "${PROJECT_ROOT}/env.example" ]]; then
  echo "Creating default environment file at ${DEPLOY_DIR}/.env"
  sudo cp "${PROJECT_ROOT}/env.example" "${DEPLOY_DIR}/.env"
fi

echo "Installing Node.js dependencies"
sudo npm install --omit=dev --prefix "${DEPLOY_DIR}"

echo "Setting ownership to ${SERVICE_USER}"
sudo chown -R "${SERVICE_USER}:${SERVICE_USER}" "${DEPLOY_DIR}"

echo "Deploying systemd service to ${TARGET_SERVICE_FILE}"
sudo cp "${SERVICE_FILE}" "${TARGET_SERVICE_FILE}"
sudo chown root:root "${TARGET_SERVICE_FILE}"

echo "Reloading systemd"
sudo systemctl daemon-reload

echo "Enabling and restarting service"
sudo systemctl enable "${SERVICE_NAME}.service"
sudo systemctl restart "${SERVICE_NAME}.service"
sudo systemctl status "${SERVICE_NAME}.service" --no-pager

echo "${SERVICE_NAME} installation complete. Update ${DEPLOY_DIR}/.env as needed and restart the service."

