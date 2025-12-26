#!/usr/bin/env bash
set -euo pipefail

# Uso:
#   ./scripts/limpiarRostros.sh <DIRECTORIO_DATASET> [NOMBRE_ARCHIVO_FINAL]
#
# - Si no se pasa NOMBRE_ARCHIVO_FINAL, usa: rostros_conocidos.txt
# - Conserva solo ese archivo dentro del dataset, elimina lo demás
# - Copia el archivo final a: <PROJECT_ROOT>/data/

if [[ $# -lt 1 ]]; then
  echo "[limpiarRostros] Uso: $0 <DIRECTORIO_DATASET> [NOMBRE_ARCHIVO_FINAL]"
  exit 1
fi

DATASET_DIR_ARG="${1%/}"
DESCRIPTORS_NAME="${2-rostros_conocidos.txt}"

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

DESCRIPTORS_FILE="${TARGET_DATASET_DIR}/${DESCRIPTORS_NAME}"

if [[ ! -f "${DESCRIPTORS_FILE}" ]]; then
  echo "[limpiarRostros] AVISO: no se encontró ${DESCRIPTORS_FILE}. No se hará limpieza agresiva."
  exit 0
fi

mkdir -p "${PROJECT_ROOT}/data"

echo "[limpiarRostros] PROJECT_ROOT: ${PROJECT_ROOT}"
echo "[limpiarRostros] Limpiando directorio ${TARGET_DATASET_DIR}, conservando solo ${DESCRIPTORS_NAME}"

find "${TARGET_DATASET_DIR}" -type f ! -name "${DESCRIPTORS_NAME}" -delete
find "${TARGET_DATASET_DIR}" -type d ! -path "${TARGET_DATASET_DIR}" -empty -delete

cp "${DESCRIPTORS_FILE}" "${PROJECT_ROOT}/data/"

echo "[limpiarRostros] Limpieza completada."
