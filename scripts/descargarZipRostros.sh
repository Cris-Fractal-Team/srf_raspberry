#!/usr/bin/env bash
set -euo pipefail

# Uso:
#   ./scripts/descargarZipRostros.sh <URL_ZIP> <DIRECTORIO_DESTINO>

if [[ $# -lt 2 ]]; then
  echo "[descargarZipRostros] Uso: $0 <URL_ZIP> <DIRECTORIO_DESTINO>"
  exit 1
fi

URL_ZIP="$1"
DATASET_DIR_ARG="${2%/}"
TOKEN="$3"

# Root del proyecto = carpeta padre de scripts/
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

# Si el directorio destino es relativo, lo hacemos relativo al root del proyecto
if [[ "${DATASET_DIR_ARG}" = /* ]]; then
  TARGET_DATASET_DIR="${DATASET_DIR_ARG}"
else
  TARGET_DATASET_DIR="${PROJECT_ROOT}/${DATASET_DIR_ARG}"
fi

echo "[descargarZipRostros] URL de descarga: ${URL_ZIP}"
echo "[descargarZipRostros] PROJECT_ROOT: ${PROJECT_ROOT}"
echo "[descargarZipRostros] Directorio destino (dataset): ${TARGET_DATASET_DIR}"

mkdir -p "${TARGET_DATASET_DIR}"

cd "${PROJECT_ROOT}"

echo "[descargarZipRostros] Descargando ZIP en PROJECT_ROOT..."
curl -H "Authorization: Bearer ${TOKEN}" -fSL -OJ "${URL_ZIP}"

echo "[descargarZipRostros] Descarga finalizada. Buscando ZIP más reciente..."
ZIP_FILE="$(ls -t *.zip 2>/dev/null | head -n 1 || true)"

if [[ -z "${ZIP_FILE}" ]]; then
  echo "[descargarZipRostros] ERROR: no se encontró ningún archivo .zip después de la descarga."
  exit 1
fi

echo "[descargarZipRostros] ZIP detectado: ${ZIP_FILE}"
echo "[descargarZipRostros] Descomprimiendo en: ${TARGET_DATASET_DIR}"

unzip -o "${ZIP_FILE}" -d "${TARGET_DATASET_DIR}"

echo "[descargarZipRostros] Descompresión finalizada."

# Si quieres, puedes borrar el ZIP del root del proyecto:
# echo "[descargarZipRostros] Eliminando ZIP descargado: ${ZIP_FILE}"
# rm -f "${ZIP_FILE}"

echo "[descargarZipRostros] Proceso completado."
