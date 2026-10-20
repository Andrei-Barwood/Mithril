#!/bin/zsh

# Script completo para gestionar simuladores de Xcode - Con manejo avanzado de permisos
set -e

# Función para mostrar el encabezado
mostrar_encabezado() {
    clear
    echo "╔══════════════════════════════════════════════════════════╗"
    echo "║          CONFIGURACIÓN DE SIMULADORES XCODE             ║"
    echo "║       Migración directa a discos externos               ║"
    echo "╚══════════════════════════════════════════════════════════╝"
    echo ""
}

# Función para barra de progreso simple
mostrar_progreso_simple() {
    local current=$1
    local total=$2
    local mensaje=$3
    local porcentaje=$((current * 100 / total))
    local completado=$((porcentaje / 5))
    local restante=$((20 - completado))
    
    local barra_completada=$(printf "%${completado}s" | tr ' ' '█')
    local barra_restante=$(printf "%${restante}s" | tr ' ' '░')
    
    printf "\r🚚 %-25s [%s%s] %3d%% " "$mensaje" "$barra_completada" "$barra_restante" "$porcentaje"
}

# Función para mostrar progreso con rsync
mostrar_progreso_rsync() {
    local origen=$1
    local destino=$2
    local mensaje=$3
    
    echo "📊 $mensaje"
    echo "   Origen: $origen"
    echo "   Destino: $destino"
    echo ""
    
    if command -v rsync &> /dev/null; then
        rsync -avh --progress "$origen/" "$destino/" 2>/dev/null || {
            echo "⚠️  Error con rsync, usando cp como respaldo..."
            copiar_con_progreso_manual "$origen" "$destino"
        }
    else
        echo "ℹ️  rsync no disponible, usando método alternativo..."
        copiar_con_progreso_manual "$origen" "$destino"
    fi
}

# Función para copia con progreso manual
copiar_con_progreso_manual() {
    local origen=$1
    local destino=$2
    
    echo "🔍 Contando archivos..."
    local total_archivos=$(find "$origen" -type f | wc -l | tr -d ' ')
    echo "📁 Total de archivos a copiar: $total_archivos"
    echo ""
    
    local contador=0
    local inicio=$(date +%s)
    
    find "$origen" -type f | while IFS= read -r archivo; do
        ((contador++))
        
        local ruta_relativa="${archivo#$origen/}"
        local destino_archivo="$destino/$ruta_relativa"
        local directorio_destino=$(dirname "$destino_archivo")
        
        mkdir -p "$directorio_destino"
        cp "$archivo" "$destino_archivo" 2>/dev/null
        
        if (( contador % 10 == 0 )) || (( contador == total_archivos )); then
            mostrar_progreso_simple $contador $total_archivos "Copiando archivos"
            
            local tiempo_transcurrido=$(($(date +%s) - inicio))
            if (( tiempo_transcurrido > 0 )); then
                local velocidad=$((contador / tiempo_transcurrido))
                local tiempo_restante=$(( (total_archivos - contador) / (velocidad + 1) ))
                printf " | %d archivos/s | ETA: %02d:%02d" $velocidad $((tiempo_restante/60)) $((tiempo_restante%60))
            fi
        fi
    done
    echo ""
}

# Función para obtener discos externos
obtener_discos_externos() {
    local discos=()
    local nombres_discos=()
    
    if [[ -d /Volumes ]]; then
        for volumen in /Volumes/*; do
            if [[ -d "$volumen" ]]; then
                local nombre_vol=$(basename "$volumen")
                if [[ "$nombre_vol" != "Macintosh HD" && "$nombre_vol" != "Recovery" && "$nombre_vol" != "Preboot" ]]; then
                    discos+=("$nombre_vol")
                    nombres_discos+=("$nombre_vol")
                fi
            fi
        done
    fi
    
    printf '%s\n' "${discos[@]}"
    echo "---SEPARADOR---"
    printf '%s\n' "${nombres_discos[@]}"
}

# Función para eliminar archivo por archivo cuando todo falla
eliminar_archivo_por_archivo() {
    local directorio=$1
    local archivos_eliminados=0
    local total_archivos=$(find "$directorio" -type f 2>/dev/null | wc -l | tr -d ' ')
    
    echo "   📁 Eliminando $total_archivos archivos individualmente..."
    
    # Eliminar archivos primero
    find "$directorio" -type f 2>/dev/null | while IFS= read -r archivo; do
        chmod u+w "$archivo" 2>/dev/null || sudo chmod u+w "$archivo" 2>/dev/null || true
        rm "$archivo" 2>/dev/null || sudo rm "$archivo" 2>/dev/null || true
        ((archivos_eliminados++))
        
        if (( archivos_eliminados % 50 == 0 )); then
            printf "   📄 Eliminados %d/%d archivos\n" $archivos_eliminados $total_archivos
        fi
    done
    
    # Eliminar directorios vacíos
    find "$directorio" -type d -empty -delete 2>/dev/null || \
    sudo find "$directorio" -type d -empty -delete 2>/dev/null || true
    
    # Verificar si el directorio principal se eliminó
    if [[ ! -d "$directorio" ]]; then
        return 0
    fi
    
    # Último intento: eliminar directorio principal
    rmdir "$directorio" 2>/dev/null || sudo rmdir "$directorio" 2>/dev/null || \
    rm -rf "$directorio" 2>/dev/null || sudo rm -rf "$directorio" 2>/dev/null
    
    if [[ ! -d "$directorio" ]]; then
        return 0
    fi
    
    return 1
}

# Función para forzar eliminación de backup problemático
forzar_eliminacion_backup() {
    local backup_path=$1
    
    echo "   🔧 Aplicando métodos de eliminación avanzados..."
    
    # Método 1: Eliminar atributos especiales
    if command -v chflags &> /dev/null; then
        echo "   🚩 Eliminando flags especiales..."
        sudo chflags -R nouchg,noschg,nouappnd,nosappnd "$backup_path" 2>/dev/null || true
    fi
    
    # Método 2: Cambiar ownership si es necesario
    local usuario_actual=$(whoami)
    echo "   👤 Cambiando ownership a $usuario_actual..."
    sudo chown -R "$usuario_actual" "$backup_path" 2>/dev/null || true
    
    # Método 3: Permisos completos
    echo "   🔓 Aplicando permisos completos..."
    sudo chmod -R 755 "$backup_path" 2>/dev/null || true
    
    # Método 4: Intentar eliminación nuevamente
    if rm -rf "$backup_path" 2>/dev/null; then
        return 0
    fi
    
    # Método 5: Eliminación con sudo
    if sudo rm -rf "$backup_path" 2>/dev/null; then
        return 0
    fi
    
    # Método 6: Eliminación forzada archivo por archivo
    echo "   📂 Eliminando archivo por archivo..."
    if eliminar_archivo_por_archivo "$backup_path"; then
        return 0
    fi
    
    return 1
}

# Función para cambiar permisos recursivamente
cambiar_permisos_recursivo() {
    local directorio=$1
    
    # Intentar cambiar permisos sin sudo primero
    if chmod -R u+w "$directorio" 2>/dev/null; then
        return 0
    fi
    
    # Si falla, intentar con sudo
    if sudo chmod -R u+w "$directorio" 2>/dev/null; then
        return 0
    fi
    
    # Método alternativo: cambiar permisos archivo por archivo
    echo "   📝 Cambiando permisos archivo por archivo..."
    find "$directorio" -type f -exec chmod u+w {} \; 2>/dev/null || \
    sudo find "$directorio" -type f -exec chmod u+w {} \; 2>/dev/null
    
    find "$directorio" -type d -exec chmod u+w {} \; 2>/dev/null || \
    sudo find "$directorio" -type d -exec chmod u+w {} \; 2>/dev/null
    
    return 0
}

# Función para eliminar backup con manejo de permisos
eliminar_backup_con_permisos() {
    local backup_path=$1
    
    # Verificar si el directorio existe
    if [[ ! -d "$backup_path" ]]; then
        echo "   ⚠️  El directorio ya no existe: $backup_path"
        return 0
    fi
    
    echo "   🗑️  Eliminando: $backup_path"
    
    # Método 1: Intentar eliminación normal
    if rm -rf "$backup_path" 2>/dev/null; then
        echo "   ✅ Eliminado correctamente"
        return 0
    fi
    
    # Método 2: Cambiar permisos recursivamente y eliminar
    echo "   🔓 Cambiando permisos para eliminar..."
    if cambiar_permisos_recursivo "$backup_path"; then
        if rm -rf "$backup_path" 2>/dev/null; then
            echo "   ✅ Eliminado después de cambiar permisos"
            return 0
        fi
    fi
    
    # Método 3: Usar sudo si es necesario
    if [[ "$backup_path" == /Library/* ]]; then
        echo "   🔐 Requiere permisos de administrador..."
        if sudo rm -rf "$backup_path" 2>/dev/null; then
            echo "   ✅ Eliminado con permisos de administrador"
            return 0
        fi
    else
        echo "   🔐 Intentando con sudo..."
        if sudo rm -rf "$backup_path" 2>/dev/null; then
            echo "   ✅ Eliminado con sudo"
            return 0
        fi
    fi
    
    # Método 4: Forzar eliminación con cambio de atributos
    echo "   🔨 Forzando eliminación..."
    if forzar_eliminacion_backup "$backup_path"; then
        echo "   ✅ Eliminado forzadamente"
        return 0
    fi
    
    echo "   ❌ No se pudo eliminar con ningún método"
    return 1
}

# Función para limpiar backups existentes con manejo de permisos
limpiar_backups_existentes() {
    echo "🧹 Buscando y eliminando backups existentes..."
    
    local backups_encontrados=()
    local ubicaciones_backup=(
        "/Library/Developer/CoreSimulator.backup"
        "$HOME/Library/Developer/CoreSimulator.backup"
    )
    
    # Buscar backups existentes
    for backup in "${ubicaciones_backup[@]}"; do
        if [[ -d "$backup" ]]; then
            backups_encontrados+=("$backup")
            local tamaño=$(du -sh "$backup" 2>/dev/null | cut -f1 || echo "N/A")
            local permisos=$(ls -ld "$backup" | cut -d' ' -f1)
            echo "📦 Backup encontrado: $backup ($tamaño) - Permisos: $permisos"
        fi
    done
    
    if [[ ${#backups_encontrados[@]} -eq 0 ]]; then
        echo "ℹ️  No se encontraron backups existentes"
        return 0
    fi
    
    echo ""
    read "eliminar?🗑️  ¿Eliminar todos los backups encontrados? [s/N]: "
    if [[ ! $eliminar =~ ^[sS]$ ]]; then
        echo "⏭️  Backups conservados"
        return 0
    fi
    
    echo "⏳ Eliminando backups con permisos apropiados..."
    echo ""
    
    local eliminados=0
    for backup in "${backups_encontrados[@]}"; do
        echo "🔍 Analizando: $backup"
        
        # Intentar eliminar con permisos normales primero
        if eliminar_backup_con_permisos "$backup"; then
            ((eliminados++))
            mostrar_progreso_simple $eliminados ${#backups_encontrados[@]} "Eliminando backups"
        else
            echo "❌ No se pudo eliminar: $backup"
        fi
    done
    
    echo ""
    if [[ $eliminados -eq ${#backups_encontrados[@]} ]]; then
        echo "✅ Se eliminaron todos los backups ($eliminados) - Espacio liberado!"
    else
        echo "⚠️  Se eliminaron $eliminados de ${#backups_encontrados[@]} backups"
    fi
}

# Función para configurar ubicación de usuario con manejo de permisos
configurar_ubicacion_usuario_directa() {
    local ruta_simuladores=$1
    local ubicacion_usuario="$HOME/Library/Developer/CoreSimulator"
    
    echo "⚙️  Configurando ubicación de usuario..."
    
    # Eliminar directamente cualquier configuración anterior con permisos
    if [[ -d "$ubicacion_usuario" ]] || [[ -L "$ubicacion_usuario" ]]; then
        echo "🗑️  Eliminando configuración anterior de usuario..."
        
        if [[ -L "$ubicacion_usuario" ]]; then
            # Es un enlace simbólico
            rm "$ubicacion_usuario" 2>/dev/null || sudo rm "$ubicacion_usuario"
        else
            # Es un directorio real
            if ! eliminar_backup_con_permisos "$ubicacion_usuario"; then
                echo "⚠️  Advertencia: No se pudo eliminar completamente la configuración anterior"
            fi
        fi
    fi
    
    # Crear directorio padre si no existe
    mkdir -p "$(dirname "$ubicacion_usuario")"
    
    # Crear enlace simbólico
    ln -sf "$ruta_simuladores" "$ubicacion_usuario"
    echo "✅ Enlace de usuario configurado"
}

# Función para configurar ubicación del sistema con manejo de permisos
configurar_ubicacion_sistema_directa() {
    local ruta_simuladores=$1
    local ubicacion_sistema="/Library/Developer/CoreSimulator"
    
    echo "⚙️  Configurando ubicación del sistema..."
    
    if [[ ! -d "/Library/Developer" ]]; then
        echo "📁 Creando directorio /Library/Developer..."
        sudo mkdir -p "/Library/Developer"
    fi
    
    # Eliminar directamente cualquier configuración anterior con permisos
    if [[ -d "$ubicacion_sistema" ]] || [[ -L "$ubicacion_sistema" ]]; then
        echo "🗑️  Eliminando configuración anterior del sistema..."
        
        if [[ -L "$ubicacion_sistema" ]]; then
            # Es un enlace simbólico
            sudo rm "$ubicacion_sistema" 2>/dev/null
        else
            # Es un directorio real
            if ! eliminar_backup_con_permisos "$ubicacion_sistema"; then
                echo "⚠️  Advertencia: No se pudo eliminar completamente la configuración anterior del sistema"
            fi
        fi
    fi
    
    # Crear enlace simbólico
    sudo ln -sf "$ruta_simuladores" "$ubicacion_sistema"
    echo "✅ Enlace del sistema configurado"
}

# Función mejorada para configurar simuladores
configurar_simuladores_completo() {
    local disco_id=$1
    local nombre_disco=$2
    
    echo ""
    echo "🔧 Configuración completa para: $nombre_disco"
    echo ""
    
    local ruta_base="/Volumes/$disco_id"
    local carpeta_simuladores="$ruta_base/XcodeSimulators"
    
    if [[ ! -d "$ruta_base" ]]; then
        echo "❌ Error: El disco no está accesible en $ruta_base."
        return 1
    fi
    
    # Limpiar backups antes de continuar
    limpiar_backups_existentes
    echo ""
    
    # Crear carpeta con indicador de progreso
    echo "📂 Preparando estructura de carpetas..."
    mostrar_progreso_simple 1 3 "Creando directorios"
    mkdir -p "$carpeta_simuladores"
    mostrar_progreso_simple 2 3 "Configurando permisos"
    chmod 755 "$carpeta_simuladores"
    mostrar_progreso_simple 3 3 "Estructura lista"
    echo ""
    
    # Migrar simuladores existentes ANTES de configurar enlaces
    migrar_simuladores_directa "$carpeta_simuladores"
    
    # Configurar ubicaciones después de la migración
    echo "⚙️  Configurando enlaces simbólicos..."
    configurar_ubicacion_usuario_directa "$carpeta_simuladores"
    configurar_ubicacion_sistema_directa "$carpeta_simuladores"
    echo ""
    
    echo "🎉 Configuración completa finalizada!"
    echo "📁 Ruta de simuladores: $carpeta_simuladores"
    echo ""
    echo "✅ Configuraciones aplicadas:"
    echo "   • Usuario: ~/Library/Developer/CoreSimulator → SSD externo"
    echo "   • Sistema: /Library/Developer/CoreSimulator → SSD externo" 
    echo ""
    echo "🚀 Próximos pasos:"
    echo "1. Cierra Xcode completamente"
    echo "2. Reinicia el servicio: sudo killall com.apple.CoreSimulator.CoreSimulatorService 2>/dev/null"
    echo "3. Abre Xcode - los simuladores se descargarán al SSD externo"
    echo ""
    
    read "?Presiona Enter para continuar..."
}

# Función para migración directa (sin crear backups)
migrar_simuladores_directa() {
    local ruta_simuladores=$1
    local ubicaciones_posibles=(
        "/Library/Developer/CoreSimulator"
        "$HOME/Library/Developer/CoreSimulator"
    )
    
    echo "🔍 Buscando simuladores existentes para migrar..."
    
    local ubicaciones_encontradas=()
    for ubicacion in "${ubicaciones_posibles[@]}"; do
        # Solo considerar directorios reales, no enlaces simbólicos
        if [[ -d "$ubicacion" && ! -L "$ubicacion" ]]; then
            local tamaño=$(du -sh "$ubicacion" 2>/dev/null | cut -f1)
            echo "📦 Encontrado: $ubicacion ($tamaño)"
            ubicaciones_encontradas+=("$ubicacion")
        fi
    done
    
    if [[ ${#ubicaciones_encontradas[@]} -eq 0 ]]; then
        echo "ℹ️  No se encontraron simuladores para migrar"
        return 0
    fi
    
    echo ""
    echo "🎯 Métodos de migración disponibles:"
    echo "  [1] rsync con progreso (recomendado)"
    echo "  [2] Copia manual con contador de archivos"
    echo "  [3] Omitir migración"
    echo ""
    
    read "metodo?Selecciona método [1-3]: "
    
    if [[ $metodo == "3" ]]; then
        echo "⏭️  Migración omitida"
        return 0
    fi
    
    for ubicacion in "${ubicaciones_encontradas[@]}"; do
        echo ""
        echo "📂 Migrando desde: $ubicacion"
        
        case $metodo in
            1)
                mostrar_progreso_rsync "$ubicacion" "$ruta_simuladores" "Migración con rsync"
                ;;
            2)
                copiar_con_progreso_manual "$ubicacion" "$ruta_simuladores"
                ;;
            *)
                echo "⚠️  Método no válido, usando rsync por defecto..."
                mostrar_progreso_rsync "$ubicacion" "$ruta_simuladores" "Migración con rsync"
                ;;
        esac
        
        # Eliminar ubicación original después de migrar exitosamente usando las nuevas funciones
        echo "🗑️  Eliminando ubicación original: $ubicacion"
        if eliminar_backup_con_permisos "$ubicacion"; then
            echo "✅ Original eliminado correctamente"
        else
            echo "⚠️  Advertencia: No se pudo eliminar la ubicación original completamente"
        fi
    done
    
    echo ""
    echo "✅ Migración directa completada"
}

# Función para mostrar menú de discos
mostrar_menu_discos() {
    local temp_file=$(mktemp)
    obtener_discos_externos > "$temp_file"
    
    local discos=()
    local nombres=()
    local en_nombres=false
    
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
    
    local seleccion
    while true; do
        read "seleccion?🔹 Selecciona el disco externo [0-${#discos[@]}]: "
        
        if [[ $seleccion == "0" ]]; then
            echo "👋 Saliendo..."
            return 2
        elif [[ $seleccion -ge 1 && $seleccion -le ${#discos[@]} ]]; then
            local disco_seleccionado="${discos[$seleccion]}"
            local nombre_seleccionado="${nombres[$seleccion]}"
            
            configurar_simuladores_completo "$disco_seleccionado" "$nombre_seleccionado"
            return 0
        else
            echo "❌ Selección inválida. Elige un número entre 0 y ${#discos[@]}."
        fi
    done
}

# Función principal
main() {
    mostrar_encabezado
    
    echo "🎯 Este script realizará una migración directa de simuladores:"
    echo "   • Manejo avanzado de permisos para eliminación"
    echo "   • Elimina backups existentes para liberar espacio"
    echo "   • Migración directa al SSD externo"
    echo "   • Configuración inmediata de enlaces simbólicos"
    echo ""
    echo "🔐 Capacidades de permisos:"
    echo "   • Maneja archivos de solo lectura automáticamente"
    echo "   • Elimina atributos especiales de macOS"
    echo "   • Usa sudo cuando es necesario"
    echo "   • Múltiples métodos de eliminación robustos"
    echo ""
    echo "⚠️  IMPORTANTE:"
    echo "   • Los simuladores se moverán (no copiarán) al SSD externo"
    echo "   • Se eliminarán las ubicaciones originales automáticamente"
    echo "   • Asegúrate de tener el SSD externo siempre conectado"
    echo ""
    echo "✨ Beneficios del enfoque directo:"
    echo "• 🚀 Proceso más rápido (sin backups intermedios)"
    echo "• 💾 Libera más espacio inmediatamente"
    echo "• 🧹 Limpieza automática con manejo robusto de permisos"
    echo "• ⚡ Configuración directa sin pasos adicionales"
    echo ""
    
    if ! mostrar_menu_discos; then
        local codigo_salida=$?
        if [[ $codigo_salida -eq 2 ]]; then
            exit 0
        else
            exit 1
        fi
    fi
}

# Ejecutar
main "$@"
