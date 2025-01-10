import Foundation

// Función para buscar patrones sospechosos en el archivo de registro
func detectIntrusion() {
    let logFilePath = "/var/log/system.log" // Ruta del archivo de registro del sistema
    
    do {
        let logFileContents = try String(contentsOfFile: logFilePath)
        
        // Verificar si hay un acceso de root no autorizado en el archivo de registro
        if logFileContents.contains("su: ROOT USER ") {
            print("Se ha detectado un acceso de root no autorizado en el archivo de registro del sistema.")


        func ejecutarScriptZsh(ruta: String) {
            let process = Process()
            process.executableURL = URL(fileURLWithPath: "/bin/zsh") // Usamos zsh como intérprete
            process.arguments = [ruta] // Ruta al archivo .zsh

            do {
                try process.run() // Ejecutamos el proceso
                process.waitUntilExit() // Esperamos a que termine
                print("Script ejecutado con éxito.")
            } catch {
                print("Error al ejecutar el script: \(error)")
            }
        }

        // Ruta completa al archivo .zsh
        let rutaScript = "03.zsh"
        ejecutarScriptZsh(ruta: rutaScript)

        } else {
            print("No se han detectado intrusiones.")
        }
    } catch { 
      print("Error al leer el archivo de registro del sistema: \(error)")
    }
}

// Llama a la función para detectar intrusiones
detectIntrusion()

