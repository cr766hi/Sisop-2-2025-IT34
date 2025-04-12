#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <dirent.h>
#include <string.h>
#include <ctype.h>


int folder_exists(const char *path) {
    struct stat st;
    return (stat(path, &st) == 0 && S_ISDIR(st.st_mode));
}

void run_command(char *const argv[]) {
    pid_t pid = fork();
    if (pid == 0) {
        execvp(argv[0], argv);
        perror("execvp failed");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
    } else {
        perror("fork failed");
        exit(EXIT_FAILURE);
    }
}

int is_valid_filename(const char *filename) {
    size_t len = strlen(filename);
    if (len != 5) return 0;
    if (!isalnum(filename[0])) return 0;
    return strcmp(filename + 1, ".txt") == 0;
}

void create_folder_if_not_exist(const char *path) {
    if (!folder_exists(path)) {
        mkdir(path, 0755);
    }
}

// Untuk Filter
void filter_files() {
    const char *base_path = "Clues";
    const char *folders[] = {"ClueA", "ClueB", "ClueC", "ClueD"};
    const int num_folders = 4;

    create_folder_if_not_exist("Filtered");

    for (int i = 0; i < num_folders; i++) {
        char full_path[256];
        snprintf(full_path, sizeof(full_path), "%s/%s", base_path, folders[i]);
        DIR *dir = opendir(full_path);
        if (!dir) continue;

        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_type == DT_REG && strstr(entry->d_name, ".txt")) {
                char src_path[512];
                snprintf(src_path, sizeof(src_path), "%s/%s", full_path, entry->d_name);

                if (is_valid_filename(entry->d_name)) {
                    char dest_path[512];
                    snprintf(dest_path, sizeof(dest_path), "Filtered/%s", entry->d_name);
                    rename(src_path, dest_path);
                } else {
                    remove(src_path);
                }
            }
        }
        closedir(dir);
    }
}

// Untuk Combine
int is_digit_file(const char *name) {
    return isdigit(name[0]);
}

int is_alpha_file(const char *name) {
    return isalpha(name[0]);
}

int compare_names(const void *a, const void *b) {
    return strcmp(*(const char **)a, *(const char **)b);
}

void combine_filtered() {
    DIR *dir = opendir("Filtered");
    if (!dir) {
        perror("Gagal membuka folder Filtered");
        return;
    }

    char *digits[100], *alphas[100];
    int digit_count = 0, alpha_count = 0;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG && is_valid_filename(entry->d_name)) {
            if (is_digit_file(entry->d_name)) {
                digits[digit_count++] = strdup(entry->d_name);
            } else if (is_alpha_file(entry->d_name)) {
                alphas[alpha_count++] = strdup(entry->d_name);
            }
        }
    }
    closedir(dir);

    qsort(digits, digit_count, sizeof(char *), compare_names);
    qsort(alphas, alpha_count, sizeof(char *), compare_names);

    FILE *out = fopen("Combined.txt", "w");
    if (!out) {
        perror("Gagal membuka Combined.txt");
        return;
    }

    int i = 0, j = 0;
    while (i < digit_count || j < alpha_count) {
        if (i < digit_count) {
            char path[256];
            snprintf(path, sizeof(path), "Filtered/%s", digits[i]);
            FILE *f = fopen(path, "r");
            if (f) {
                char ch;
                while ((ch = fgetc(f)) != EOF)
                    fputc(ch, out);
                fclose(f);
                remove(path);
            }
            i++;
        }
        if (j < alpha_count) {
            char path[256];
            snprintf(path, sizeof(path), "Filtered/%s", alphas[j]);
            FILE *f = fopen(path, "r");
            if (f) {
                char ch;
                while ((ch = fgetc(f)) != EOF)
                    fputc(ch, out);
                fclose(f);
                remove(path);
            }
            j++;
        }
    }

    fclose(out);
    printf("Isi file digabungkan dan file asli dihapus\n");
}

// Untuk Decode
char rot13_char(char c) {
    if ('a' <= c && c <= 'z')
        return ((c - 'a' + 13) % 26) + 'a';
    else if ('A' <= c && c <= 'Z')
        return ((c - 'A' + 13) % 26) + 'A';
    else
        return c;
}

void decode_combined() {
    FILE *in = fopen("Combined.txt", "r");
    if (!in) {
        perror("Combined.txt tidak ditemukan");
        return;
    }

    FILE *out = fopen("Decoded.txt", "w");
    if (!out) {
        perror("Tidak bisa membuat Decoded.txt");
        fclose(in);
        return;
    }

    char ch;
    while ((ch = fgetc(in)) != EOF) {
        fputc(rot13_char(ch), out);
    }

    fclose(in);
    fclose(out);
    printf("Decoded.txt berhasil dibuat dari Combined.txt\n");
}

// Fungsi Main
int main(int argc, char *argv[]) {
    if (argc > 1) {
        if (strcmp(argv[1], "filter") == 0) {
            filter_files();
            return 0;
        } else if (strcmp(argv[1], "combine") == 0) {
            combine_filtered();
            return 0;
        } else if (strcmp(argv[1], "decode") == 0) {
            decode_combined();
            return 0;
        } else {
            printf("Argumen tidak dikenali: %s\n", argv[1]);
            printf("Gunakan salah satu dari:\n");
            printf("  ./action           -> Unduh dan ekstrak Clues.zip\n");
            printf("  ./action filter    -> Filter file menjadi 1 huruf atau angka\n");
            printf("  ./action combine   -> Gabungkan isi file yang difilter ke file combined.txt\n");
            printf("  ./action decode    -> Decode Combined.txt ke Decoded.txt\n");
            return 1;
        }
    }

    const char *clue_folder = "Clues";
    const char *clue_zip = "Clues.zip";
    const char *download_url = "https://drive.google.com/uc?export=download&id=1xFn1OBJUuSdnApDseEczKhtNzyGekauK";

    if (folder_exists(clue_folder)) {
        printf("Folder '%s' sudah ada. Tidak mengunduh ulang.\n", clue_folder);
        return 0;
    }

    char *wget_args[] = {
        "wget", "-O", (char *)clue_zip, (char *)download_url, NULL
    };
    run_command(wget_args);

    char *unzip_args[] = {
        "unzip", (char *)clue_zip, NULL
    };
    run_command(unzip_args);

    char *rm_args[] = {
        "rm", (char *)clue_zip, NULL
    };
    run_command(rm_args);

    return 0;
}
