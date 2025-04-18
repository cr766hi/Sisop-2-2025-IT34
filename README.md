# Sisop-2-2025-IT34
Member :
1. Christiano Ronaldo Silalahi (5027241025)
2. Naila Cahyarani Idelia (5027241063)
3. Daniswara Fausta Novanto (5027241050)



# Soal Modul 2

## Soal 1

Buat sebuah *file* action.c

```bash
$ nano action.c
```
Coba program dan compile terlebih dahulu dengan `gcc action.c -o action`


```bash
int folder_exists(const char *path) {
    struct stat st;
    return (stat(path, &st) == 0 && S_ISDIR(st.st_mode));
}
```
Fungsi untuk mengecek apakah folder sudah ada

```bash
void create_folder_if_not_exist(const char *path) {
    if (!folder_exists(path)) {
        mkdir(path, 0755);
    }
}
```
Fungsi untuk membuat folder

```bash
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
```
Fungsi untuk child process

```bash
int is_valid_filename(const char *filename) {
    size_t len = strlen(filename);
    if (len != 5) return 0;
    if (!isalnum(filename[0])) return 0;
    return strcmp(filename + 1, ".txt") == 0;
}
```
Fungsi untuk mengecek apakah nama file valid

```bash
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
```
Fungsi main untuk argumen dan error handling

Output jika argumen tidak tepat: ![Image](https://github.com/user-attachments/assets/13620345-46f8-42a7-835f-e04cdc4041c6)


### a.

Download dan unzip file

Execute program berikut dengan cara `./action.c`
```bash
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
```
Output: ![Image](https://github.com/user-attachments/assets/3a460039-a330-451e-83d1-e53a8f1b040d)

### b.

Filter Clue dengan 1 huruf dan 1 angka

Execute programm berikut dengan cara `./action.c filter`
```bash
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
```

### c.

Execute program berikut dengan cara `./action combine`
```bash
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
```
Funngsi digunakan untuk combine file

### d.

Decode file dengan ROT13

Execute program berikut dengan cara `./action decode`
```bash
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
```

### e.

Hasil setelah input password: ![Image](https://github.com/user-attachments/assets/950028b2-c6e2-48d8-a059-8b3ac36f3c17)



pertama-tama download dulu file starterkitnya

![image](https://github.com/user-attachments/assets/8167c126-4682-436f-8e1d-8c7d6bfcf1e2)

setelah di download, file starterkit langsung di unzip

lalu buatlah code dengan

```bash
nano starterkit.c
```

berikut adalah fungsi codenya :

### 1. **Deklarasi Variabel dan Fungsi Utama**
Di awal kode, kita mendeklarasikan beberapa variabel penting:
- `char *pidfile = "decrypt.pid";`  
  Menyimpan nama file PID yang digunakan untuk melacak proses dekripsi.
  
- Fungsi `main()` akan menangani parsing argumen dari command line menggunakan `getopt_long()`.

### 2. **Fungsi `decrypt()`**
Fungsi ini bertanggung jawab untuk:
- Membuat direktori quarantine dan menjalankan daemon
- Menggunakan fungsi `base64_decode()` untuk mendekripsi nama file yang terenkripsi dengan Base64.
- Mengubah nama file yang telah didekripsi dan memindahkannya ke direktori `starter_kit` yang sama.
![image](https://github.com/user-attachments/assets/7a5ed39d-ec2b-4ec7-9241-58b8d8864cff)

### 3. **Fungsi `move_to_quarantine()`**
Fungsi ini bertugas untuk:
- Memindahkan file dari direktori `starter_kit` ke `quarantine`.
- juga mengenkripsi isi dari starter_kit.
![image](https://github.com/user-attachments/assets/89944b79-273f-4dce-ab40-222721d928c6)


### 4. **Fungsi `return_from_quarantine()`**
Fungsi ini digunakan untuk mengembalikan file yang telah dipindahkan ke `quarantine` ke direktori `starter_kit`.
- Program memeriksa apakah file ada di direktori `quarantine`, lalu memindahkannya kembali ke `starter_kit` dengan `rename()`.
![image](https://github.com/user-attachments/assets/f8ad5e1b-46d2-4971-91b7-e9bc6b1217bd)


### 5. **Fungsi `eradicate_files()`**
Fungsi ini digunakan untuk menghapus file yang ada di direktori `quarantine`.
- Program akan memeriksa apakah file ada di direktori `quarantine` dan menghapusnya menggunakan `remove()`.
![image](https://github.com/user-attachments/assets/bd79b7aa-dacb-444a-80b9-7bc520385390)


### 6. **Fungsi `shutdown_decryption()`**
Fungsi ini bertugas untuk mematikan proses daemon dekripsi dengan membaca PID dari file `decrypt.pid` dan menghentikan proses yang terkait menggunakan `kill()`.

### 7. **Fungsi `log_activity()`**
Fungsi ini bertanggung jawab untuk mencatat semua aktivitas program dalam file log `activity.log`. Setiap kali ada operasi yang dilakukan (seperti dekripsi, pemindahan file, penghapusan, dan shutdown), fungsi ini akan mencatatnya dengan timestamp dan jenis aksi yang dilakukan.
![image](https://github.com/user-attachments/assets/7cbee99e-913f-402d-837c-4e91dc755d62)

### 8. **Menangani Argumen Command Line**
Program ini menggunakan `getopt_long()` untuk menangani berbagai opsi yang diberikan pengguna:
- `--decrypt`: Memulai proses dekripsi file.
- `--quarantine`: Memindahkan file yang telah didekripsi ke direktori `quarantine`.
- `--return`: Mengembalikan file dari `quarantine` ke `starter_kit`.
- `--eradicate`: Menghapus file di direktori `quarantine`.
- `--shutdown`: Menghentikan proses daemon yang sedang berjalan.

</div>

## Soal 4

Buat sebuah *file* debugmon.c

```bash
$ nano debugmon.c
```
Coba program dan compile terlebih dahulu dengan `gcc debugmon.c -o debugmon`

### a.

Fungsi do_list:

```bash
void do_list(const char *user) {
    char cmd[128];
    snprintf(cmd, sizeof(cmd), "ps -u %s -o pid,comm,%%cpu,%%mem", user);

    FILE *fp = popen(cmd, "r");
    if (fp == NULL) {
        perror("Gagal menjalankan ps");
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        printf("%s", line);
    }

    pclose(fp);
}
```

Fungsi ini digunakan untuk menampilkan daftar proses yang dimiliki oleh user.

Kemudian execute program dengan cara `./debugmon list <user>`.

![show list](assets/list.png)

### b. 

Fungsi do_daemon:

```bash
void do_daemon(const char *user) {
    char failfile[64];
    snprintf(failfile, sizeof(failfile), "fail.%s", user);
    if (access(failfile, F_OK) == 0) {
        printf("User %s sedang dalam mode FAIL. Tidak bisa menjalankan daemon.\n", user);
        return;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("Fork gagal");
        return;
    }

    if (pid > 0) {
        FILE *pf = fopen("debugmon.pid", "w");
        if (pf) {
            fprintf(pf, "%d", pid);
            fclose(pf);
        }
        printf("Debugmon sedang memantau user: %s (PID: %d)\n", user, pid);
        return;
    }

    // Child daemon
    while (1) {
        char cmd[128];
        snprintf(cmd, sizeof(cmd), "ps -u %s -o comm=", user);
        FILE *fp = popen(cmd, "r");
        if (fp) {
            char line[128];
            while (fgets(line, sizeof(line), fp)) {
                line[strcspn(line, "\n")] = 0;
                log_status(line, "RUNNING");
            }
            pclose(fp);
        }
        sleep(5);
    }
}
```

Fungsi ini mengubah program menjadi daemon yang memantau proses user secara berkala dan mencatat statusnya dalam berkas log.

Kemudian execute program dengan cara `./debugmon daemon <user>`.

![show daeomon](assets/daemon.png)
![show daeomon(2)](assets/daemon(2).png)

### c. 

Fungsi do_stop:

```bash
void do_stop() {
    FILE *pf = fopen("debugmon.pid", "r");
    if (!pf) {
        printf("Tidak ditemukan PID file. Debugmon mungkin tidak aktif.\n");
        return;
    }

    int pid;
    fscanf(pf, "%d", &pid);
    fclose(pf);

    if (kill(pid, SIGTERM) == 0) {
        printf("Debugmon (PID %d) dihentikan.\n", pid);
        remove("debugmon.pid");
        log_status("stop", "SUCCESS");
    } else {
        perror("Gagal menghentikan daemon");
    }
}
```

Fungsi ini digunakan untuk menghentikan daemon debugmon yang sedang berjalan.

Kemudian execute program dengan cara `./debugmon stop <user>`.

![show stop](assets/stop.png)
![show stop(2)](assets/stop(2).png)


### d. 

Fungsi do_fail:

```bash
void do_fail(const char *user) {
    printf("Mematikan semua proses milik user: %s...\n", user);

    char cmd[128];
    snprintf(cmd, sizeof(cmd), "ps -u %s -o pid=,comm=", user);
    FILE *fp = popen(cmd, "r");
    if (fp) {
        char line[256];
        while (fgets(line, sizeof(line), fp)) {
            int pid;
            char proc[128];

            if (sscanf(line, "%d %[^\n]", &pid, proc) != 2) {
                continue;
            }

            printf("Coba bunuh: %s (PID: %d)\n", proc, pid);

            if (kill(pid, SIGKILL) == 0) {
                printf("Berhasil bunuh: %s (PID: %d)\n", proc, pid);
                log_status(proc, "FAILED");
            } else {
                perror("Gagal membunuh proses");
            }
        }
        pclose(fp);
    } else {
        perror("Gagal membuka output ps");
    }

    char failfile[64];
    snprintf(failfile, sizeof(failfile), "fail.%s", user);
    FILE *fail = fopen(failfile, "w");
    if (fail) fclose(fail);

    printf("User %s sekarang dalam mode FAIL.\n", user);
}
```

Fungsi ini dapat digunakan untuk menghentikan semua proses user secara paksa dan mengaktifkan mode "FAIL" untuk user tersebut.

Kemudian execute program dengan cara `./debugmon fail <user>`.

![show fail](assets/fail.png)

### e.

Fungsi do_revert:

```bash
void do_revert(const char *user) {
    char failfile[64];
    snprintf(failfile, sizeof(failfile), "fail.%s", user);
    if (remove(failfile) == 0) {
        printf("User %s telah keluar dari mode FAIL.\n", user);
        log_status("revert", "SUCCESS");
    } else {
        printf("User %s tidak dalam mode FAIL.\n", user);
    }
}
```

Dengan menggunakan fungsi ini, berkas indikator "fail" user dihapus. Fungsi ini mengeluarkan user dari mode "FAIL" dan memungkinkan daemon debugmon untuk dipantau kembali.

Kemudian execute program dengan cara `./debugmon revert <user>`.

![show revert](assets/revert.png)
![show revert(2)](assets/revert(2).png)

### f.

Fungsi log_status:

```bash
#include <stdio.h>      
#include <stdlib.h>     
#include <string.h>     
#include <unistd.h>     
#include <sys/types.h>  
#include <signal.h>     
#include <time.h>

void log_status(const char *proc, const char *status) {
    FILE *log = fopen("debugmon.log", "a");
    if (!log) return;

    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);

    char date[20], timebuf[20];
    strftime(date, sizeof(date), "%d:%m:%Y", tm_info);
    strftime(timebuf, sizeof(timebuf), "%H:%M:%S", tm_info);

    fprintf(log, "[%s]-[%s]_%s_STATUS(%s)\n", date, timebuf, proc, status);
    fclose(log);
}      
```

Fungsi ini mencatat status proses ke dalam berkas log bernama `debugmon.log`. Tanggal, waktu, nama proses, dan status akan disertakan dalam setiap catatan log.

Fungsi main:

```bash
int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Gunakan perintah: list|daemon|stop|fail|revert <user>\n");
        return 1;
    }

    const char *cmd = argv[1];
    const char *user = argc > 2 ? argv[2] : NULL;

    if (strcmp(cmd, "list") == 0 && user) do_list(user);
    else if (strcmp(cmd, "daemon") == 0 && user) do_daemon(user);
    else if (strcmp(cmd, "stop") == 0) do_stop();
    else if (strcmp(cmd, "fail") == 0 && user) do_fail(user);
    else if (strcmp(cmd, "revert") == 0 && user) do_revert(user);
    else printf("Perintah tidak dikenali atau user tidak disebutkan.\n");

    return 0;
}
```
