#!/bin/zsh

# Script para gestionar ubicaciones de simuladores de Xcode en discos externos
# Uso: chmod +x xcode_simulator_setup.zsh && ./xcode_simulator_setup.zsh

set -e  # Salir si hay errores

# Función para mostrar el encabezado
mostrar_encabezado() {
    clear
    echo "╔══════════════════════════════════════════════════════════╗"
    echo "║          CONFIGURACIÓN DE SIMULADORES XCODE             ║"
    echo "║       Gestión de ubicaciones en discos externos         ║"
    echo "╚══════════════════════════════════════════════════════════╝"
    echo ""
}

# Función para obtener discos externos disponibles
obtener_discos_externos() {
    local discos=()
    local nombres_discos=()
    
    # Buscar volúmenes montados en /Volumes (excluyendo disco principal)
    if [[ -d /Volumes ]]; then
        for volumen in /Volumes/*; do
            if [[ -d "$volumen" ]]; then
                local nombre_vol=$(basename "$volumen")
                # Excluir el disco principal y algunos volúmenes de sistema
                if [[ "$nombre_vol" != "Macintosh HD" && "$nombre_vol" != "Recovery" && "$nombre_vol" != "Preboot" ]]; then
                    discos+=("$nombre_vol")
                    nombres_discos+=("$nombre_vol")
                fi
            fi
        done
    fi
    
    # Si diskutil está disponible, también buscar discos externos
    if command -v diskutil &> /dev/null; then
        local diskutil_output=$(diskutil list external physical 2>/dev/null)
        if [[ $? -eq 0 && -n "$diskutil_output" ]]; then
            while IFS= read -r linea; do
                if [[ $linea =~ "Apple_HFS|APFS|exFAT|NTFS" ]]; then
                    local disk_name=$(echo "$linea" | awk '{print $NF}' | tr -d ' ')
                    if [[ -n "$disk_name" && ! " ${discos[*]} " =~ " $disk_name " ]]; then
                        discos+=("$disk_name")
                        nombres_discos+=("$disk_name")
                    fi
                fi
            done <<< "$diskutil_output"
        fi
    fi
    
    # Devolver los arrays como strings separados por newlines
    printf '%s\n' "${discos[@]}"
    echo "---SEPARADOR---"
    printf '%s\n' "${nombres_discos[@]}"
}

# Función para mostrar menú de discos
mostrar_menu_discos() {
    local temp_file=$(mktemp)
    obtener_discos_externos > "$temp_file"
    
    local discos=()
    local nombres=()
    local en_nombres=false
    
    # Leer el archivo temporal y separar arrays
    while IFS= read -r linea; do
        if [[ "$linea" == "---SEPARADOR---" ]]; then
            en_nombres=true
            continue
        fi
        
        if [[ "$en_nombres" == true ]]; then
            nombres+=("$linea")
        else
            discos+=("$linea")
        fi
    done < "$temp_file"
    
    rm -f "$temp_file"
    
    if [[ ${#discos[@]} -eq 0 ]]; then
        echo "⚠️  No se detectaron discos externos conectados."
        echo ""
        echo "Para usar este script, conecta un disco externo USB, Thunderbolt o FireWire."
        return 1
    fi
    
    echo "📱 Discos externos detectados:"
    echo ""
    
    local i=1
    for disco in "${discos[@]}"; do
        echo "  [$i] $disco"
        ((i++))
    done
    
    echo "  [0] Salir"
    echo ""
    
    # Leer selección del usuario
    local seleccion
    while true; do
        read "seleccion?🔹 Selecciona el disco externo [0-${#discos[@]}]: "
        
        if [[ $seleccion == "0" ]]; then
            echo "👋 Saliendo..."
            return 2
        elif [[ $seleccion -ge 1 && $seleccion -le ${#discos[@]} ]]; then
            local indice=$((seleccion - 1))
            local disco_seleccionado="${discos[$indice]}"
            local nombre_seleccionado="${nombres[$indice]}"
            
            configurar_simuladores "$disco_seleccionado" "$nombre_seleccionado"
            return 0
        else
            echo "❌ Selección inválida. Elige un número entre 0 y ${#discos[@]}."
        fi
    done
}

# Función para configurar la carpeta de simuladores
configurar_simuladores() {
    local disco_id=$1
    local nombre_disco=$2
    
    echo ""
    echo "🔧 Configurando simuladores para: $nombre_disco"
    echo ""
    
    # Determinar ruta base del disco
    local ruta_base="/Volumes/$disco_id"
    
    # Verificar que el disco esté montado y accesible
    if [[ ! -d "$ruta_base" ]]; then
        echo "❌ Error: El disco no está montado o no es accesible en $ruta_base."
        return 1
    fi
    
    # Crear carpeta para simuladores
    local carpeta_simuladores="$ruta_base/XcodeSimulators"
    
    if [[ -d "$carpeta_simuladores" ]]; then
        echo "📂 La carpeta ya existe: $carpeta_simuladores"
    else
        echo "📂 Creando carpeta de simuladores..."
        mkdir -p "$carpeta_simuladores"
        if [[ $? -eq 0 ]]; then
            echo "✅ Carpeta creada exitosamente: $carpeta_simuladores"
        else
            echo "❌ Error al crear la carpeta. Verifica los permisos."
            return 1
        fi
    fi
    
    # Configurar Xcode para usar la nueva ubicación
    configurar_xcode_ubicacion "$carpeta_simuladores"
    
    echo ""
    echo "🎉 Configuración completada!"
    echo "📁 Ruta de simuladores: $carpeta_simuladores"
    echo ""
    echo "Para aplicar los cambios:"
    echo "1. Reinicia Xcode si está abierto"
    echo "2. Los nuevos simuladores se descargarán a la ubicación externa"
    echo "3. Los simuladores existentes permanecerán en su ubicación original"
    echo ""
    
    read "?Presiona Enter para continuar..."
}

# Función para configurar la ubicación en Xcode
configurar_xcode_ubicacion() {
    local ruta_simuladores=$1
    
    echo "⚙️  Configurando Xcode para usar ubicación externa..."
    
    # Crear enlace simbólico desde la ubicación predeterminada
    local ubicacion_predeterminada="$HOME/Library/Developer/CoreSimulator"
    local backup_ubicacion="$HOME/Library/Developer/CoreSimulator.backup"
    
    # Hacer backup de la configuración existente si existe
    if [[ -d "$ubicacion_predeterminada" && ! -L "$ubicacion_predeterminada" ]]; then
        echo "📋 Creando backup de simuladores existentes..."
        if [[ -d "$backup_ubicacion" ]]; then
            rm -rf "$backup_ubicacion"
        fi
        mv "$ubicacion_predeterminada" "$backup_ubicacion"
    fi
    
    # Remover enlace existente si existe
    if [[ -L "$ubicacion_predeterminada" ]]; then
        rm "$ubicacion_predeterminada"
    fi
    
    # Crear enlace simbólico
    ln -sf "$ruta_simuladores" "$ubicacion_predeterminada"
    
    if [[ $? -eq 0 ]]; then
        echo "✅ Enlace simbólico configurado exitosamente"
        
        # Migrar simuladores existentes si hay backup
        if [[ -d "$backup_ubicacion" ]]; then
            echo ""
            echo "📦 ¿Deseas migrar los simuladores existentes a la nueva ubicación? [s/N]"
            read respuesta
            if [[ $respuesta =~ ^[sS]$ ]]; then
                echo "🚚 Migrando simuladores..."
                cp -R "$backup_ubicacion"/* "$ruta_simuladores/" 2>/dev/null || true
                echo "✅ Migración completada"
            fi
        fi
    else
        echo "❌ Error al configurar enlace simbólico"
        return 1
    fi
}

# Función principal
main() {
    mostrar_encabezado
    
    # Verificar permisos
    if [[ $EUID -eq 0 ]]; then
        echo "⚠️  No ejecutes este script como root (sudo)."
        exit 1
    fi
    
    # Mostrar información inicial
    echo "Este script te permite:"
    echo "• 📱 Ver discos externos conectados"
    echo "• 📂 Crear carpeta para simuladores de Xcode"
    echo "• ⚙️  Configurar Xcode para usar ubicación externa"
    echo "• 🚀 Mejorar rendimiento al ejecutar desde SSD externo"
    echo ""
    
    # Mostrar menú y procesar selección
    if ! mostrar_menu_discos; then
        local codigo_salida=$?
        if [[ $codigo_salida -eq 2 ]]; then
            exit 0  # Salida normal
        else
            exit 1  # Error
        fi
    fi
}

# Ejecutar función principal
main "$@"
