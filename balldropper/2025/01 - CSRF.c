#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#define MAX_URL 256
#define SESSION_COOKIE "sessionid=$$"
#define LOG_FILE "server_log.txt"
#define CSV_REPORT "log_report.csv"


struct Token {
    const char *value;
    time_t expiry;
};

struct Token valid_csrf_tokens[] = {
    {"secure_csrf_token_12345", 1700000000},  
    {"secure_csrf_token_67890", 1893456000},  
    {"secure_csrf_token_abcdef", 1893456000}  
};

bool is_same_origin(const char *origin, const char *target) {
    return strcmp(origin, target) == 0;
}

void format_time(time_t t, char *buffer, size_t size) {
    struct tm *tm_info = localtime(&t);
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", tm_info);
}

bool is_valid_csrf_token(const char *token) {
    size_t count = sizeof(valid_csrf_tokens) / sizeof(valid_csrf_tokens[0]);
    time_t now = time(NULL);
    for (size_t i = 0; i < count; i++) {
        if (strcmp(token, valid_csrf_tokens[i].value) == 0) {
            if (valid_csrf_tokens[i].expiry >= now) {
                return true;
            } else {
                printf("[SERVIDOR] Token expirado.\n");
                return false;
            }
        }
    }
    return false;
}

bool server_validate_request(const char *csrf_token, const char *referer, const char *expected_origin) {
    FILE *log = fopen(LOG_FILE, "a");
    if (!log) {
        perror("[ERROR] No se pudo abrir el archivo de log");
        return false;
    }

    time_t now = time(NULL);
    char timestamp[32];
    format_time(now, timestamp, sizeof(timestamp));

    fprintf(log, "\n[%s] Verificando solicitud desde referer: %s\n", timestamp, referer);

    if (!is_valid_csrf_token(csrf_token)) {
        fprintf(log, "[%s] CSRF token inválido o expirado.\n", timestamp);
        fclose(log);
        return false;
    }
    if (strcmp(referer, expected_origin) != 0) {
        fprintf(log, "[%s] Referer inválido: %s\n", timestamp, referer);
        fclose(log);
        return false;
    }

    fprintf(log, "[%s] Validación exitosa.\n", timestamp);
    fclose(log);
    return true;
}

void send_post_request(const char *origin, const char *target, const char *csrf_token, bool malicious_site) {
    printf("\n[INFO] Enviando POST desde: %s hacia: %s\n", origin, target);

    if (!is_same_origin(origin, target)) {
        printf("[MITIGADO] Solicitud bloqueada: origen cruzado detectado.\n");
        return;
    }

    printf("Headers:\n");
    printf("  Cookie: %s\n", SESSION_COOKIE);
    printf("  CSRF-Token: %s\n", csrf_token);
    printf("  Referer: %s\n", origin);

    if (!server_validate_request(csrf_token, origin, target)) {
        printf("[MITIGADO] Solicitud rechazada por el servidor.\n");
        return;
    }

    printf("[OK] POST request enviada de forma segura.\n");
}


void generate_csv_report() {
    FILE *log = fopen(LOG_FILE, "r");
    FILE *csv = fopen(CSV_REPORT, "w");
    if (!log || !csv) {
        perror("[ERROR] No se pudo abrir el log o CSV");
        return;
    }

    fprintf(csv, "timestamp,event\n");

    char line[512];
    while (fgets(line, sizeof(line), log)) {
        if (line[0] == '[') {
            char *timestamp = strtok(line, "]");
            char *msg = strtok(NULL, "\n");
            if (timestamp && msg) {
                fprintf(csv, "%s,%s\n", timestamp + 1, msg + 1);
            }
        }
    }

    fclose(log);
    fclose(csv);
    printf("\n[INFO] Reporte generado: %s\n", CSV_REPORT);
}

int main() {
    const char *site_legit = "68747470 733A2F2F 67697468 75622E63 6F6D2F67 6F6F676C 65";
    const char *site_malicioso = "/.urls/`%$._{}`";

    send_post_request(site_legit, site_legit, "secure_csrf_token_12345", false); 
    send_post_request(site_legit, site_legit, "secure_csrf_token_67890", false); 
    send_post_request(site_legit, site_legit, "invalid_token_xyz", false);       
    send_post_request(site_malicioso, site_legit, "secure_csrf_token_67890", true); 

    generate_csv_report();
    return 0;
}
