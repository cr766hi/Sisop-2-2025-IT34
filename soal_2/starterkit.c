#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>
#include <time.h>
#include <fcntl.h>
#include <syslog.h>
#include <errno.h>
#include <ctype.h>

#define STAGING_DIR "starter_kit"
#define QUARANTINE_DIR "quarantine"

char base64_map[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

// Fungsi decode Base64
char* decode_base64(const char* input) {
    int len = strlen(input);
    char* output = malloc(len);
    int val = 0, valb = -8;
    int pos = 0;
 printf("Decoding input: %s\n", input);

    for (int i = 0; i < len; i++) {
        char* ptr = strchr(base64_map, input[i]);
        if (!ptr) continue;
        val = (val << 6) + (ptr - base64_map);
        valb += 6;
        if (valb >= 0) {
            output[pos++] = (val >> valb) & 0xFF;
            valb -= 8;
        }
    }
    output[pos] = '\0';

    printf("Decoded output: %s\n", output);  

    return output;
}


// Fungsi untuk menulis log aktivitas
void write_log(char *message) {
    FILE *log = fopen("activity.log", "a");  //tidakk menimpa file yang ada
    if (log) {
        time_t now = time(NULL);
        struct tm *t = localtime(&now);
 fprintf(log, "[%02d-%02d-%d][%02d:%02d:%02d] - %s\n",
            t->tm_mday, t->tm_mon + 1, t->tm_year + 1900,
            t->tm_hour, t->tm_min, t->tm_sec, message);
        fclose(log);
    } else {
        perror("Failed to open log file");
    }
}

// Fungsi untuk memindahkan file ke direktori karantina setelah mendekripsi nama file
void quarantine_files() {
    DIR *dir = opendir(STAGING_DIR);
    if (dir == NULL) {
        perror("Failed to open staging directory");
        exit(EXIT_FAILURE);
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {
            char source_path[PATH_MAX];
            char dest_path[PATH_MAX];

            snprintf(source_path, sizeof(source_path), "%s/%s", STAGING_DIR, entry->d_name);
            snprintf(dest_path, sizeof(dest_path), "%s/%s", QUARANTINE_DIR, entry->d_name);

            if (rename(source_path, dest_path) == 0) {
                printf("Moved '%s' to quarantine successfully.\n", entry->d_name);
            } else {
                perror("Failed to move file");
            }
        }
    }

    closedir(dir);
}

// Fungsi untuk memindahkan file dari karantina ke starter_kit
void return_files() {
    DIR *dir = opendir(QUARANTINE_DIR);
    if (!dir) {
        printf("Error opening quarantine directory.\n");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {
            char src_path[512], dest_path[512];

            snprintf(src_path, sizeof(src_path), "%s/%s", QUARANTINE_DIR, entry->d_name);
 snprintf(dest_path, sizeof(dest_path), "%s/%s", STAGING_DIR, entry->d_name);


                if (rename(src_path, dest_path) == 0) {
                printf("Moved '%s' back to starter_kit successfully.\n", entry->d_name);
                write_log("Moved file back to starter_kit successfully.");
            } else {
                perror("Failed to move file back to starter_kit");
            }
        }
    }
    closedir(dir);
}


// Fungsi untuk menghapus file di dalam karantina
void eradicate_files() {
    DIR *dir = opendir("quarantine");
    if (!dir) {
        printf("Error opening quarantine directory.\n");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {
            char src_path[512], dest_path[512];

            snprintf(src_path, sizeof(src_path), "%s/%s", QUARANTINE_DIR, entry->d_name);
            snprintf(dest_path, sizeof(dest_path), "%s/%s", STAGING_DIR, entry->d_name);


                if (rename(src_path, dest_path) == 0) {
                printf("Moved '%s' back to starter_kit successfully.\n", entry->d_name);
                write_log("Moved file back to starter_kit successfully.");
            } else {
                perror("Failed to move file back to starter_kit");
            }
        }
    }
    closedir(dir);
}


// Fungsi untuk menghapus file di dalam karantina
void eradicate_files() {
    DIR *dir = opendir("quarantine");
    if (!dir) {
        printf("Error opening quarantine directory.\n");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {
            char file_path[512];
            snprintf(file_path, sizeof(file_path), "quarantine/%s", entry->d_name);

            if (remove(file_path) == 0) {
                printf("Eradicated '%s' from quarantine.\n", entry->d_name);
                write_log("Eradicated file from quarantine.");
            } else {
                perror("Failed to eradicate file");
            }
        }
    }
    closedir(dir);
}

// Fungsi untuk menulis log shutdown
void write_shutdown_log() {
    write_log("Shutdown process initiated.");
}

// Fungsi untuk menghentikan daemon jika ada
void shutdown_daemon() {
    FILE *pidfile = fopen("decrypt.pid", "r");
if (pidfile) {
        int pid;
        fscanf(pidfile, "%d", &pid);
        fclose(pidfile);

        if (kill(pid, SIGTERM) == 0) {
            write_shutdown_log();
            printf("Daemon terminated successfully.\n");
        } else {
            perror("Failed to terminate daemon");
        }
    } else {
        printf("No daemon process found.\n");
    }
}

// Fungsi untuk menjalankan dekripsi

int is_probably_base64(const char *str) {
    int len = strlen(str);
    if (len % 4 != 0) return 0;

    for (int i = 0; i < len; i++) {
        if (!isalnum(str[i]) && str[i] != '+' && str[i] != '/' && str[i] != '=') {
            return 0;
            }
    }
    return 1;
}

void decrypt_files() {
    DIR *dir = opendir("starter_kit");
    if (!dir) {
        perror("Failed to open starter_kit directory");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {
            if (!is_probably_base64(entry->d_name)) {
                continue;
            }

            char old_path[512], new_path[512];
            snprintf(old_path, sizeof(old_path), "starter_kit/%s", entry->d_name);

            char *decoded_name = decode_base64(entry->d_name);
            if (!decoded_name || strlen(decoded_name) == 0) {
                free(decoded_name);
                continue;
            }

            snprintf(new_path, sizeof(new_path), "starter_kit/%s", decoded_name);

            rename(old_path, new_path);

            free(decoded_name);
        }
    }

    closedir(dir);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s [--decrypt | --quarantine | --return | --eradicate | --shutdown]\n", argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "--decrypt") == 0) {
        decrypt_files();
    } else if (strcmp(argv[1], "--quarantine") == 0) {
        quarantine_files();
    } else if (strcmp(argv[1], "--return") == 0) {
        return_files();
    } else if (strcmp(argv[1], "--eradicate") == 0) {
        eradicate_files();
    } else if (strcmp(argv[1], "--shutdown") == 0) {
        shutdown_daemon();
    } else {
        printf("Invalid argument.\n");
        return 1;
    }

    return 0;
}
