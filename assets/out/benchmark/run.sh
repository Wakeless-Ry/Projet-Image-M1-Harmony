#!/bin/bash

# Dossier source contenant les images → dossier courant
SRC_DIR="."  
# Dossier de sortie pour les PNG → sous-dossier "sortie" dans le dossier courant
OUT_DIR="./sortie"

# Crée le dossier de sortie s'il n'existe pas
mkdir -p "$OUT_DIR"

# Parcours récursif de tous les fichiers .pgm et .ppm
find "$SRC_DIR" -type f \( -iname "*.pgm" -o -iname "*.ppm" \) | while read FILE; do
    # Crée le chemin relatif par rapport à SRC_DIR
    REL_PATH="${FILE#$SRC_DIR/}"
    # Supprime l'extension et ajoute .png
    OUT_FILE="$OUT_DIR/${REL_PATH%.*}.png"
    # Crée le dossier du fichier de sortie s'il n'existe pas
    mkdir -p "$(dirname "$OUT_FILE")"
    # Conversion avec ImageMagick
    convert "$FILE" "$OUT_FILE"
    echo "Converti: $FILE → $OUT_FILE"
done

echo "Conversion terminée !"
