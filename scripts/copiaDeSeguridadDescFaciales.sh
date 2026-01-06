#!/usr/bin/env bash
set -euo pipefail

if [[ $# -lt 4 ]]; then
  echo "[copiaDeSeguridadDescFaciales] Uso: $0 <DIRECTORIO_DATASET> <API_URL> <SERIE_EQUIPO> <TOKEN> [ETIQUETA_OPCIONAL] [NOMBRE_ARCHIVO_FINAL]"
  exit 1
fi

DATASET_DIR_ARG="${1%/}"
API_URL="${2}"
SERIE_EQUIPO="${3}"
TOKEN="${4}"
CUSTOM_LABEL="${5-}"
DESCRIPTORS_NAME="${6-rostros_conocidos.txt}"

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

if [[ "${DATASET_DIR_ARG}" = /* ]]; then
  TARGET_DATASET_DIR="${DATASET_DIR_ARG}"
else
  TARGET_DATASET_DIR="${PROJECT_ROOT}/${DATASET_DIR_ARG}"
fi

DESCRIPTORS_FILE="${TARGET_DATASET_DIR}/${DESCRIPTORS_NAME}"

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

BACKUP_FILE="${TARGET_DATASET_DIR}/${BASE_LABEL}.txt"

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

echo "[copiaDeSeguridadDescFaciales] Proceso de backup local finalizado."

if [[ -z "${API_URL}" ]]; then
  echo "[copiaDeSeguridadDescFaciales] No se subió el backup: API_URL vacía"
  exit 0
fi

if [[ -z "${TOKEN}" ]]; then
  echo "[copiaDeSeguridadDescFaciales] No se subió el backup: TOKEN vacío"
  exit 0
fi

BACKUP_ENDPOINT="${API_URL%/}/srf/backup/desc-facial/${SERIE_EQUIPO}"

echo "[copiaDeSeguridadDescFaciales] Subiendo backup a: ${BACKUP_ENDPOINT}"
echo "[copiaDeSeguridadDescFaciales] Archivo: ${BACKUP_FILE}"

RESPONSE_FILE="$(mktemp)"

set +e
if [[ -n "${CUSTOM_LABEL}" ]]; then
  HTTP_CODE=$(
    curl -sS -o "${RESPONSE_FILE}" -w '%{http_code}' \
      -X POST "${BACKUP_ENDPOINT}" \
      -H "Authorization: Bearer ${TOKEN}" \
      -F "file=@${BACKUP_FILE};type=text/plain" \
      -F "nombreArchivo=${BASE_LABEL}.txt"
  )
else
  HTTP_CODE=$(
    curl -sS -o "${RESPONSE_FILE}" -w '%{http_code}' \
      -X POST "${BACKUP_ENDPOINT}" \
      -H "Authorization: Bearer ${TOKEN}" \
      -F "file=@${BACKUP_FILE};type=text/plain"
  )
fi
CURL_EXIT=$?
set -e

if [[ "${CURL_EXIT}" -ne 0 ]]; then
  echo "[copiaDeSeguridadDescFaciales] ERROR al invocar API (curl exit code ${CURL_EXIT})"
elif [[ "${HTTP_CODE}" -ge 200 && "${HTTP_CODE}" -lt 300 ]]; then
  echo "[copiaDeSeguridadDescFaciales] Backup subido correctamente. Respuesta API:"
  cat "${RESPONSE_FILE}"
  echo
else
  echo "[copiaDeSeguridadDescFaciales] ERROR: API respondió HTTP ${HTTP_CODE}"
  cat "${RESPONSE_FILE}"
  echo
fi

rm -f "${RESPONSE_FILE}"
