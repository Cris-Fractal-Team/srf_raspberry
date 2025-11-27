#!/usr/bin/env bash
set -euo pipefail

# Script para generar descriptores faciales usando ./myapp -procesa
# Estructura esperada del directorio:
#   <directorio_dataset>/
#       datos.txt
#       rostro_1.jpg
#       rostro_2.jpg
#       ...

if [[ $# -lt 1 ]]; then
  echo "[generarDescFaciales] Uso: $0 <directorio_dataset>"
  exit 1
fi

DIRECTORIO_DATASET="$1/"

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

echo "[generarDescFaciales] Directorio raíz del proyecto: ${ROOT_DIR}"
echo "[generarDescFaciales] Directorio del dataset: ${DIRECTORIO_DATASET}"

if [[ ! -d "${DIRECTORIO_DATASET}" ]]; then
  echo "[generarDescFaciales] ERROR: el directorio ${DIRECTORIO_DATASET} no existe."
  exit 1
fi

ARCHIVO_DATOS="${DIRECTORIO_DATASET}/datos.txt"

if [[ ! -f "${ARCHIVO_DATOS}" ]]; then
  echo "[generarDescFaciales] ERROR: no se encontró ${ARCHIVO_DATOS}"
  exit 1
fi

ARCHIVO_SALIDA="${DIRECTORIO_DATASET}/rostros_conocidos.txt"
MYAPP_BIN="${ROOT_DIR}/myapp"

if [[ ! -x "${MYAPP_BIN}" ]]; then
  echo "[generarDescFaciales] ERROR: no se encontró el binario ejecutable: ${MYAPP_BIN}"
  echo "[generarDescFaciales] Asegúrate de compilarlo y darle permisos de ejecución."
  exit 1
fi

# 1) Detener cualquier instancia previa de myapp que esté usando el Hailo
if pgrep -f "${MYAPP_BIN}" >/dev/null; then
  echo "[generarDescFaciales] Se encontraron instancias activas de myapp, deteniéndolas..."
  pkill -f "${MYAPP_BIN}" || true
  sleep 10
else
  echo "[generarDescFaciales] No hay instancias previas de myapp usando Hailo."
fi

echo "[generarDescFaciales] Usando:"
echo "  ARCHIVO_DATOS    = ${ARCHIVO_DATOS}"
echo "  DIRECTORIO_FOTOS = ${DIRECTORIO_DATASET}"
echo "  ARCHIVO_SALIDA   = ${ARCHIVO_SALIDA}"
echo
echo "[generarDescFaciales] Ejecutando:"
echo "  ${MYAPP_BIN} -procesa \"${ARCHIVO_DATOS}\" \"${DIRECTORIO_DATASET}\" \"${ARCHIVO_SALIDA}\""
echo

if "${MYAPP_BIN}" -procesa "${ARCHIVO_DATOS}" "${DIRECTORIO_DATASET}" "${ARCHIVO_SALIDA}"; then
  echo "[generarDescriptoresFaciales] Descriptores generados correctamente en: ${ARCHIVO_SALIDA}"
else
  RESULTADO="$?"
  echo "[generarDescriptoresFaciales] ERROR: myapp terminó con código ${RESULTADO}, se continúa de todas formas."
  # NO hacemos exit, solo seguimos
fi

