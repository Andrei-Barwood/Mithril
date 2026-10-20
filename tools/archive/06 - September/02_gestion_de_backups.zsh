#!/bin/zsh

# Herramienta de emergencia para eliminar backups obstinados de CoreSimulator
# Específicamente diseñada para /Library/Developer/CoreSimulator.backup

set -e

# Función para mostrar encabezado de emergencia
mostrar_encabezado_emergencia() {
    clear
    echo "╔══════════════════════════════════════════════════════════╗"
    echo "║               🚨 HERRAMIENTA DE EMERGENCIA               ║"
    echo "║           Eliminación de Backups Obstinados             ║"
    echo "╚══════════════════════════════════════════════════════════╝"
    echo ""
    echo "⚠️  ADVERTENCIA: Esta herramienta usa métodos agresivos"
    echo "    Solo usar para backups que no se pueden eliminar normalmente"
    echo ""
}

# Función para verificar SIP (System Integrity Protection)
verificar_sip() {
    echo "🔍 Verificando System Integrity Protection..."
    local sip_status=$(csrutil status 2>/dev/null | grep -i enabled || echo "disabled")
    
    if [[ $sip_status =~ "enabled" ]]; then
        echo "🔒 SIP está HABILITADO - puede impedir la eliminación"
        echo ""
        echo "💡 OPCIÓN RECOMENDADA:"
        echo "   1. Reinicia tu Mac manteniendo presionado Cmd+R"
        echo "   2. En Recovery Mode, abre Terminal (Utilities → Terminal)"
        echo "   3. Ejecuta: csrutil disable && reboot"
        echo "   4. Ejecuta este script nuevamente"
        echo "   5. Reinicia a Recovery Mode y ejecuta: csrutil enable && reboot"
        echo ""
        read "continuar?¿Continuar sin desactivar SIP? (puede fallar) [s/N]: "
        if [[ ! $continuar =~ ^[sS]$ ]]; then
            echo "👋 Reinicia en Recovery Mode para desactivar SIP temporalmente"
            exit 0
        fi
    else
        echo "✅ SIP está deshabilitado - condiciones óptimas para eliminación"
    fi
}

# Función para mostrar información del backup problemático
analizar_backup_problema() {
    local backup_path="/Library/Developer/CoreSimulator.backup"
    
    echo "📊 Analizando backup problemático..."
    echo "📍 Ruta: $backup_path"
    echo ""
    
    if [[ ! -d "$backup_path" ]]; then
        echo "✅ ¡El backup ya no existe! Problema resuelto."
        exit 0
    fi
    
    # Información detallada
    local tamaño=$(du -sh "$backup_path" 2>/dev/null | cut -f1 || echo "N/A")
    local permisos=$(ls -ld "$backup_path" | cut -d' ' -f1)
    local propietario=$(ls -ld "$backup_path" | awk '{print $3":"$4}')
    local archivos=$(find "$backup_path" -type f 2>/dev/null | wc -l | tr -d ' ')
    
    echo "📏 Tamaño: $tamaño"
    echo "🔐 Permisos: $permisos"
    echo "👤 Propietario: $propietario"
    echo "📁 Archivos: $archivos"
    
    # Verificar flags especiales
    if command -v stat &> /dev/null; then
        local flags=$(stat -f %Sf "$backup_path" 2>/dev/null || echo "none")
        echo "🚩 Flags: $flags"
    fi
    
    echo ""
    echo "💾 Esto liberará aproximadamente $tamaño de espacio"
}

# Función para eliminación con modo de recuperación simulado
eliminacion_modo_recuperacion() {
    local backup_path="/Library/Developer/CoreSimulator.backup"
    
    echo "🔧 Aplicando técnicas de modo recuperación..."
    
    # Paso 1: Eliminar inmutabilidad y flags
    echo "   🚩 Eliminando flags de protección..."
    sudo chflags -R nouchg,nouappnd,noschg,nosappnd "$backup_path" 2>/dev/null || true
    
    # Paso 2: Cambiar ownership completo
    echo "   👤 Cambiando ownership al usuario actual..."
    sudo chown -R $(whoami):staff "$backup_path" 2>/dev/null || true
    
    # Paso 3: Permisos máximos
    echo "   🔓 Aplicando permisos máximos..."
    sudo chmod -R 777 "$backup_path" 2>/dev/null || true
    
    # Paso 4: Eliminar atributos extendidos
    echo "   📝 Eliminando atributos extendidos..."
    sudo xattr -rd com.apple.quarantine "$backup_path" 2>/dev/null || true
    sudo xattr -c -r "$backup_path" 2>/dev/null || true
    
    # Paso 5: Intentar eliminación directa
    if sudo rm -rf "$backup_path" 2>/dev/null; then
        echo "   ✅ Eliminado con técnicas de recuperación"
        return 0
    fi
    
    return 1
}

# Función para eliminación nuclear (último recurso)
eliminacion_nuclear() {
    local backup_path="/Library/Developer/CoreSimulator.backup"
    
    echo "💥 ELIMINACIÓN NUCLEAR - Método de último recurso"
    echo "   ⚠️  Esto puede tomar varios minutos..."
    
    # Crear script temporal para ejecución en background
    local temp_script=$(mktemp)
    cat > "$temp_script" << 'EOF'
#!/bin/zsh
backup_path="$1"

# Función para eliminar archivo por archivo de forma agresiva
eliminar_agresivo() {
    local directorio="$1"
    
    # Primero, intentar eliminar todos los archivos
    find "$directorio" -type f -print0 2>/dev/null | while IFS= read -r -d '' archivo; do
        # Eliminar atributos
        sudo chflags nouchg,nouappnd,noschg,nosappnd "$archivo" 2>/dev/null || true
        sudo xattr -c "$archivo" 2>/dev/null || true
        sudo chmod 777 "$archivo" 2>/dev/null || true
        
        # Intentar eliminación múltiple
        rm -f "$archivo" 2>/dev/null || \
        sudo rm -f "$archivo" 2>/dev/null || \
        sudo unlink "$archivo" 2>/dev/null || true
    done
    
    # Luego, eliminar directorios vacíos
    find "$directorio" -type d -depth -print0 2>/dev/null | while IFS= read -r -d '' dir; do
        sudo chflags nouchg,nouappnd,noschg,nosappnd "$dir" 2>/dev/null || true
        rmdir "$dir" 2>/dev/null || \
        sudo rmdir "$dir" 2>/dev/null || \
        sudo rm -rf "$dir" 2>/dev/null || true
    done
}

eliminar_agresivo "$backup_path"
EOF
    
    chmod +x "$temp_script"
    
    # Ejecutar script nuclear
    echo "   🔥 Iniciando eliminación archivo por archivo..."
    "$temp_script" "$backup_path" &
    local script_pid=$!
    
    # Mostrar progreso
    local inicial_size=$(du -s "$backup_path" 2>/dev/null | cut -f1 || echo "0")
    while kill -0 $script_pid 2>/dev/null; do
        local current_size=$(du -s "$backup_path" 2>/dev/null | cut -f1 || echo "0")
        local liberado=$((inicial_size - current_size))
        printf "\r   📊 Liberado: %d KB de %d KB" $liberado $inicial_size
        sleep 2
    done
    
    wait $script_pid
    rm -f "$temp_script"
    echo ""
    
    # Verificar resultado
    if [[ ! -d "$backup_path" ]]; then
        echo "   ✅ Eliminación nuclear exitosa"
        return 0
    fi
    
    return 1
}

# Función para usar herramientas de sistema de bajo nivel
eliminacion_bajo_nivel() {
    local backup_path="/Library/Developer/CoreSimulator.backup"
    
    echo "⚙️  Usando herramientas de sistema de bajo nivel..."
    
    # Método 1: Usar ditto para mover a temporal y eliminar
    local temp_dir=$(mktemp -d)
    echo "   📦 Moviendo a directorio temporal..."
    if sudo ditto "$backup_path" "$temp_dir/backup_temp" 2>/dev/null; then
        sudo rm -rf "$backup_path" 2>/dev/null
        sudo rm -rf "$temp_dir" 2>/dev/null
        if [[ ! -d "$backup_path" ]]; then
            echo "   ✅ Eliminado con ditto"
            return 0
        fi
    fi
    
    # Método 2: Usar rsync para vaciar
    echo "   🔄 Vaciando con rsync..."
    local empty_dir=$(mktemp -d)
    sudo rsync -av --delete "$empty_dir/" "$backup_path/" 2>/dev/null || true
    sudo rmdir "$backup_path" 2>/dev/null || sudo rm -rf "$backup_path" 2>/dev/null
    rm -rf "$empty_dir"
    
    if [[ ! -d "$backup_path" ]]; then
        echo "   ✅ Eliminado con rsync"
        return 0
    fi
    
    return 1
}

# Función principal de eliminación con múltiples métodos
ejecutar_eliminacion_completa() {
    local backup_path="/Library/Developer/CoreSimulator.backup"
    
    echo "🚀 Iniciando eliminación completa con múltiples métodos..."
    echo ""
    
    # Método 1: Modo recuperación simulado
    if eliminacion_modo_recuperacion; then
        echo ""
        echo "🎉 ¡ÉXITO! Backup eliminado con técnicas de recuperación"
        return 0
    fi
    
    # Método 2: Herramientas de bajo nivel
    echo ""
    if eliminacion_bajo_nivel; then
        echo ""
        echo "🎉 ¡ÉXITO! Backup eliminado con herramientas de bajo nivel"
        return 0
    fi
    
    # Método 3: Eliminación nuclear (último recurso)
    echo ""
    read "nuclear?⚠️  ¿Proceder con eliminación nuclear? (archivo por archivo) [s/N]: "
    if [[ $nuclear =~ ^[sS]$ ]]; then
        if eliminacion_nuclear; then
            echo ""
            echo "🎉 ¡ÉXITO! Backup eliminado con método nuclear"
            return 0
        fi
    fi
    
    echo ""
    echo "❌ No se pudo eliminar el backup con ningún método"
    echo "💡 Recomendación: Desactivar SIP temporalmente y reintentar"
    return 1
}

# Función para verificar el resultado final
verificar_eliminacion() {
    local backup_path="/Library/Developer/CoreSimulator.backup"
    
    echo ""
    echo "🔍 Verificando eliminación..."
    
    if [[ ! -d "$backup_path" ]]; then
        echo "✅ ¡CONFIRMADO! El backup ha sido eliminado completamente"
        echo ""
        
        # Mostrar espacio liberado
        echo "📊 Verificando espacio en disco..."
        df -h / | tail -1 | awk '{printf "💾 Espacio disponible: %s de %s (%s usado)\n", $4, $2, $5}'
        echo ""
        echo "🎉 ¡Has recuperado aproximadamente 20 GB de espacio!"
        
    elif [[ -d "$backup_path" ]]; then
        local remaining_size=$(du -sh "$backup_path" 2>/dev/null | cut -f1)
        echo "⚠️  El backup aún existe, tamaño restante: $remaining_size"
        
        # Proporcionar información de contacto o pasos adicionales
        echo ""
        echo "🆘 ÚLTIMO RECURSO:"
        echo "   1. Reinicia en Recovery Mode (Cmd+R al inicio)"
        echo "   2. En Terminal ejecuta: csrutil disable && reboot"
        echo "   3. Ejecuta este script nuevamente"
        echo "   4. Vuelve a Recovery Mode: csrutil enable && reboot"
        
    else
        echo "✅ ¡ÉXITO! El backup ya no existe"
    fi
}

# Función principal
main() {
    mostrar_encabezado_emergencia
    
    # Verificar permisos de administrador
    if [[ $EUID -eq 0 ]]; then
        echo "⚠️  No ejecutes este script como root directo. Usa tu usuario normal."
        echo "   El script pedirá sudo cuando sea necesario."
        exit 1
    fi
    
    # Verificar SIP
    verificar_sip
    echo ""
    
    # Analizar el problema
    analizar_backup_problema
    echo ""
    
    # Confirmación final
    read "proceder?🚨 ¿PROCEDER con la eliminación agresiva? [s/N]: "
    if [[ ! $proceder =~ ^[sS]$ ]]; then
        echo "👋 Operación cancelada"
        exit 0
    fi
    
    echo ""
    echo "⏰ La eliminación puede tomar varios minutos..."
    echo "   NO interrumpas el proceso una vez iniciado"
    echo ""
    
    # Ejecutar eliminación
    if ejecutar_eliminacion_completa; then
        verificar_eliminacion
    else
        verificar_eliminacion
        exit 1
    fi
    
    echo ""
    read "?Presiona Enter para salir..."
}

# Ejecutar
main "$@"
