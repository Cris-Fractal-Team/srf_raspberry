#!/usr/bin/env bash
set -euo pipefail

# Script para:
#  1) Descargar un ZIP con el nombre que manda el servidor
#  2) Descomprimirlo en un directorio fijo
#  3) Llamar al script de generación de descriptores faciales
#  4) Eliminar el ZIP y todo lo extra, dejando solo descriptores_faciales.bd

if [[ $# -lt 1 ]]; then
  echo "[descargarRostros] Uso: $0 <URL_ZIP>"
  exit 1
fi

URL_ZIP="$1"

# Directorio raíz del proyecto (donde está este script)
ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

echo "[descargarRostros] URL de descarga: ${URL_ZIP}"
echo "[descargarRostros] Directorio raíz: ${ROOT_DIR}"

cd "${ROOT_DIR}"

# 1) Descargar el ZIP usando el nombre por defecto del servidor
#    -O  : guarda con el nombre local
#    -J  : respeta Content-Disposition del servidor (nombre de archivo)
echo "[descargarRostros] Descargando ZIP..."
curl -fSL -OJ "${URL_ZIP}"

echo "[descargarRostros] Descarga finalizada. Buscando ZIP más reciente..."

# 2) Buscar el ZIP más reciente en el directorio
ZIP_FILE="$(ls -t *.zip 2>/dev/null | head -n 1 || true)"

if [[ -z "${ZIP_FILE}" ]]; then
  echo "[descargarRostros] ERROR: no se encontró ningún archivo .zip después de la descarga."
  exit 1
fi

echo "[descargarRostros] ZIP detectado: ${ZIP_FILE}"

# 3) Descomprimir el ZIP
DEST_DIR="${ROOT_DIR}/descriptores_faciales"

echo "[descargarRostros] Descomprimiendo en: ${DEST_DIR}"
mkdir -p "${DEST_DIR}"
unzip -o "${ZIP_FILE}" -d "${DEST_DIR}"

echo "[descargarRostros] Descompresión finalizada."

# 4) Llamar al script que genera los descriptores faciales usando ./myapp -procesa
GENERADOR_SCRIPT="${ROOT_DIR}/generarDescFaciales.sh"

if [[ -x "${GENERADOR_SCRIPT}" ]]; then
  echo "[descargarRostros] Ejecutando script de generación de descriptores: ${GENERADOR_SCRIPT}"
  "${GENERADOR_SCRIPT}" "${DEST_DIR}"
  RESULTADO_GENERADOR="$?"
  if [[ "${RESULTADO_GENERADOR}" -ne 0 ]]; then
    echo "[descargarRostros] ERROR: el script de generación terminó con código ${RESULTADO_GENERADOR}. No se limpiarán archivos."
    exit "${RESULTADO_GENERADOR}"
  fi
  echo "[descargarRostros] Script de generación finalizado correctamente."

  # 5) Limpieza:
  #    - eliminar el ZIP
  #    - eliminar todo el contenido de DEST_DIR excepto descriptores_faciales.bd
  echo "[descargarRostros] Eliminando ZIP descargado: ${ZIP_FILE}"
  rm -f "${ZIP_FILE}"

  DESCRIPTORES_BD="${DEST_DIR}/rostros_conocidos.txt"

  if [[ -f "${DESCRIPTORES_BD}" ]]; then
    echo "[descargarRostros] Limpiando directorio ${DEST_DIR}, conservando solo descriptores_faciales.bd"

    # Eliminar todos los archivos excepto descriptores_faciales.bd
    find "${DEST_DIR}" -type f ! -name 'rostros_conocidos.txt' -delete

    # Eliminar subdirectorios vacíos (si los hubiera)
    find "${DEST_DIR}" -type d ! -path "${DEST_DIR}" -empty -delete
  else
    echo "[descargarRostros] AVISO: no se encontró ${DESCRIPTORES_BD}. No se eliminarán archivos del directorio."
  fi

  echo "[descargarRostros] Limpieza completada."
else
  echo "[descargarRostros] AVISO: no se encontró script de generación:"
  echo "  ${GENERADOR_SCRIPT}"
  echo "[descargarRostros] Asegúrate de que exista y tenga permisos de ejecución (chmod +x)."
fi
