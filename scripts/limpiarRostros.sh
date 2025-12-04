#!/usr/bin/env bash
set -euo pipefail

# Uso:
#   ./scripts/limpiarRostros.sh <DIRECTORIO_DATASET>

if [[ $# < 1 ]]; then
  echo "[limpiarRostros] Uso: $0 <DIRECTORIO_DATASET>"
  exit 1
fi

DATASET_DIR_ARG="${1%/}"
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

# Normalizamos el directorio dataset a absoluto
if [[ "${DATASET_DIR_ARG}" = /* ]]; then
  TARGET_DATASET_DIR="${DATASET_DIR_ARG}"
else
  TARGET_DATASET_DIR="${PROJECT_ROOT}/${DATASET_DIR_ARG}"
fi

if [[ ! -d "${TARGET_DATASET_DIR}" ]]; then
  echo "[limpiarRostros] ERROR: el directorio ${TARGET_DATASET_DIR} no existe."
  exit 1
fi

DESCRIPTORS_FILE="${TARGET_DATASET_DIR}/rostros_conocidos.txt"

if [[ ! -f "${DESCRIPTORS_FILE}" ]]; then
  echo "[limpiarRostros] AVISO: no se encontró ${DESCRIPTORS_FILE}. No se hará limpieza agresiva."
  exit 0
fi

echo "[limpiarRostros] PROJECT_ROOT: ${PROJECT_ROOT}"
echo "[limpiarRostros] Limpiando directorio ${TARGET_DATASET_DIR}, conservando solo rostros_conocidos.txt"

find "${TARGET_DATASET_DIR}" -type f ! -name 'rostros_conocidos.txt' -delete
find "${TARGET_DATASET_DIR}" -type d ! -path "${TARGET_DATASET_DIR}" -empty -delete

cp "${DESCRIPTORS_FILE}" data/

echo "[limpiarRostros] Limpieza completada."
