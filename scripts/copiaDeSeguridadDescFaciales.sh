#!/usr/bin/env bash
set -euo pipefail

# Uso:
#   ./scripts/copiaDeSeguridadDescFaciales.sh <DIRECTORIO_DATASET> [ETIQUETA_OPCIONAL]

if [[ $# -lt 1 ]]; then
  echo "[copiaDeSeguridadDescFaciales] Uso: $0 <DIRECTORIO_DATASET> [ETIQUETA_OPCIONAL]"
  exit 1
fi

DATASET_DIR_ARG="${1%/}"
CUSTOM_LABEL="${2-}"

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

if [[ "${DATASET_DIR_ARG}" = /* ]]; then
  TARGET_DATASET_DIR="${DATASET_DIR_ARG}"
else
  TARGET_DATASET_DIR="${PROJECT_ROOT}/${DATASET_DIR_ARG}"
fi

DESCRIPTORS_FILE="${TARGET_DATASET_DIR}/rostros_conocidos.txt"
BACKUP_DIR="${PROJECT_ROOT}/desc_faciales_versiones"

mkdir -p "${BACKUP_DIR}"

if [[ ! -f "${DESCRIPTORS_FILE}" ]]; then
  echo "[copiaDeSeguridadDescFaciales] ERROR: no se encontró ${DESCRIPTORS_FILE}"
  exit 1
fi

LATEST_ZIP="$(ls -t "${PROJECT_ROOT}"/*.zip 2>/dev/null | head -n 1 || true)"

if [[ -n "${LATEST_ZIP}" ]]; then
  ZIP_BASENAME="$(basename "${LATEST_ZIP}")"
  BASE_LABEL="${ZIP_BASENAME%.zip}"
  echo "[copiaDeSeguridadDescFaciales] Usando nombre de ZIP más reciente: ${ZIP_BASENAME}"
elif [[ -n "${CUSTOM_LABEL}" ]]; then
  BASE_LABEL="${CUSTOM_LABEL%.zip}"
  echo "[copiaDeSeguridadDescFaciales] No se encontró ZIP, usando etiqueta proporcionada: ${BASE_LABEL}"
else
  BASE_LABEL="rostros_$(date +%Y%m%d_%H%M%S)"
  echo "[copiaDeSeguridadDescFaciales] No se encontró ZIP ni etiqueta, usando etiqueta por defecto: ${BASE_LABEL}"
fi

BACKUP_FILE="${BACKUP_DIR}/${BASE_LABEL}.txt"

echo "[copiaDeSeguridadDescFaciales] PROJECT_ROOT: ${PROJECT_ROOT}"
echo "[copiaDeSeguridadDescFaciales] Dataset: ${TARGET_DATASET_DIR}"
echo "[copiaDeSeguridadDescFaciales] Origen : ${DESCRIPTORS_FILE}"
echo "[copiaDeSeguridadDescFaciales] Destino: ${BACKUP_FILE}"

cp "${DESCRIPTORS_FILE}" "${BACKUP_FILE}"

echo "[copiaDeSeguridadDescFaciales] Copia de seguridad creada correctamente."

if [[ -n "${LATEST_ZIP}" ]]; then
  echo "[copiaDeSeguridadDescFaciales] Eliminando ZIP utilizado: ${LATEST_ZIP}"
  rm -f "${LATEST_ZIP}"
fi

echo "[copiaDeSeguridadDescFaciales] Proceso de backup finalizado."
