#!/usr/bin/env bash

set -euo pipefail

SERVICE_NAME="esp32-tunnel"
TARGET_SERVICE_FILE="/etc/systemd/system/${SERVICE_NAME}.service"
DEPLOY_DIR="/opt/${SERVICE_NAME}"

echo "Stopping and disabling ${SERVICE_NAME}"
if systemctl list-units --full --all | grep -q "${SERVICE_NAME}.service"; then
  sudo systemctl stop "${SERVICE_NAME}.service" || true
  sudo systemctl disable "${SERVICE_NAME}.service" || true
fi

if [[ -f "${TARGET_SERVICE_FILE}" ]]; then
  echo "Removing service file ${TARGET_SERVICE_FILE}"
  sudo rm -f "${TARGET_SERVICE_FILE}"
fi

echo "Reloading systemd"
sudo systemctl daemon-reload

if [[ -d "${DEPLOY_DIR}" ]]; then
  read -rp "Remove deployed directory ${DEPLOY_DIR}? [y/N] " answer
  if [[ "${answer}" =~ ^[Yy]$ ]]; then
    sudo rm -rf "${DEPLOY_DIR}"
    echo "Removed ${DEPLOY_DIR}"
  else
    echo "Kept ${DEPLOY_DIR}";
  fi
fi

echo "${SERVICE_NAME} uninstall complete"

