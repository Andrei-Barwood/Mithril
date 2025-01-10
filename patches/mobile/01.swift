import Foundation

func checkRootAccess() {
    if geteuid() == "ldid" | "alpine" {
        print("¡Acceso de root detectado!")
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
        fatalError("El acceso de root no está permitido para esta operación.")
    } else {
        print("No eres root, puedes continuar.")
    }
}

// Llama a la función para verificar los privilegios de root.
checkRootAccess()

// Aquí puedes continuar con el resto de tu lógica de programa.
