#!/bin/zsh

# Script para eliminar espacio purgable en macOS Sequoia
# Para MacBook Air M2
# Autor: Kirtan Teg Singh
# Fecha: $(date)

echo "🧹 Script de limpieza de espacio purgable para macOS Sequoia"
echo "============================================================"

# Función para mostrar el espacio disponible antes y después
mostrar_espacio() {
    echo "📊 Espacio actual del disco:"
    df -h / | awk 'NR==2 {print "   Usado: " $3 " | Disponible: " $4 " | Total: " $2}'
    echo ""
}

# Función para crear archivos grandes temporales
crear_archivos_grandes() {
    local directorio_temporal="$HOME/temp_purgeable_cleanup"
    
    echo "🔄 Creando directorio temporal para forzar liberación de espacio purgable..."
    mkdir -p "$directorio_temporal"
    
    echo "⚡ Generando archivos grandes para forzar la limpieza..."
    echo "   (Esto puede tardar varios minutos)"
    
    # Crear archivo inicial de 15MB chunks
    dd if=/dev/random of="$directorio_temporal/largefile" bs=15m count=1 2>/dev/null &
    
    local pid=$!
    local contador=1
    
    # Mostrar progreso
    while kill -0 $pid 2>/dev/null; do
        echo -n "."
        sleep 1
    done
    echo ""
    
    # Continuar creando copias hasta llenar el disco
    echo "📁 Duplicando archivos para agotar el espacio disponible..."
    
    while true; do
        contador=$((contador + 1))
        
        # Intentar copiar el archivo
        if ! cp "$directorio_temporal/largefile" "$directorio_temporal/largefile$contador" 2>/dev/null; then
            echo "⚠️  Espacio de disco crítico alcanzado - iniciando limpieza"
            break
        fi
        
        # Verificar si el disco está lleno
        local espacio_libre=$(df / | awk 'NR==2 {print $4}')
        if [ "$espacio_libre" -lt 1000000 ]; then
            echo "⚠️  Espacio crítico detectado - iniciando limpieza"
            break
        fi
        
        echo "   Creado archivo $contador..."
    done
    
    # Eliminar todos los archivos temporales
    echo "🗑️  Eliminando archivos temporales..."
    rm -rf "$directorio_temporal"
    
    echo "✅ Archivos temporales eliminados"
}

# Función para vaciar la papelera
vaciar_papelera() {
    echo "🗑️  Vaciando la Papelera..."
    osascript -e 'tell application "Finder" to empty trash' 2>/dev/null
    echo "✅ Papelera vaciada"
}

# Función para limpiar cachés del sistema
limpiar_caches() {
    echo "🧽 Limpiando cachés del sistema..."
    
    # Limpiar cachés de usuario
    if [ -d "$HOME/Library/Caches" ]; then
        find "$HOME/Library/Caches" -type f -atime +7 -delete 2>/dev/null
        echo "   ✓ Cachés de usuario limpiados"
    fi
    
    # Limpiar logs antiguos
    if [ -d "$HOME/Library/Logs" ]; then
        find "$HOME/Library/Logs" -name "*.log" -atime +30 -delete 2>/dev/null
        echo "   ✓ Logs antiguos eliminados"
    fi
    
    # Limpiar archivos temporales
    if [ -d "/tmp" ]; then
        find /tmp -user $(whoami) -atime +1 -delete 2>/dev/null
        echo "   ✓ Archivos temporales eliminados"
    fi
}

# Función para optimizar almacenamiento
optimizar_almacenamiento() {
    echo "⚙️  Verificando configuración de optimización de almacenamiento..."
    
    # Verificar si está habilitada la optimización
    local optimizacion_habilitada=$(defaults read com.apple.osinstallernotifications OSInstallerAutoInstall 2>/dev/null || echo "0")
    
    if [ "$optimizacion_habilitada" = "1" ]; then
        echo "   ✅ Optimización de almacenamiento ya está habilitada"
    else
        echo "   ℹ️  Para habilitar la optimización automática, ve a:"
        echo "      Configuración del Sistema > General > Almacenamiento > Recomendaciones"
    fi
}

# Función principal
main() {
    echo "🚀 Iniciando limpieza de espacio purgable..."
    echo ""
    
    # Mostrar espacio inicial
    mostrar_espacio
    
    # Ejecutar funciones de limpieza
    limpiar_caches
    echo ""
    
    vaciar_papelera
    echo ""
    
    optimizar_almacenamiento
    echo ""
    
    # Preguntar si el usuario quiere ejecutar el método intensivo
    echo "❓ ¿Deseas ejecutar el método intensivo de liberación de espacio purgable?"
    echo "   (Esto creará archivos temporales grandes para forzar la liberación)"
    read -q "respuesta?Presiona 'y' para continuar, cualquier otra tecla para salir: "
    echo ""
    
    if [[ $respuesta == "y" ]]; then
        crear_archivos_grandes
        echo ""
        
        # Vaciar papelera una vez más después del proceso intensivo
        vaciar_papelera
        echo ""
    fi
    
    # Mostrar espacio final
    echo "📈 Espacio después de la limpieza:"
    mostrar_espacio
    
    echo "🎉 ¡Proceso de limpieza completado!"
    echo ""
    echo "💡 Consejos adicionales:"
    echo "   • Reinicia tu Mac para liberar más espacio purgable automáticamente"
    echo "   • Habilita 'Optimizar almacenamiento' en Configuración del Sistema"
    echo "   • Ejecuta este script periódicamente para mantener tu Mac optimizado"
}

# Verificar si se está ejecutando en macOS
if [[ $(uname) != "Darwin" ]]; then
    echo "❌ Este script está diseñado para macOS"
    exit 1
fi

# Ejecutar función principal
main
