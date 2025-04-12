#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>

#define LOG_FILE "debugmon.log"

// f
void log_status(const char *process_name, const char *status) {
    FILE *log_file = fopen(LOG_FILE, "a");
    if (log_file) {
        time_t now = time(NULL);
        struct tm *t = localtime(&now);
        fprintf(log_file, "[%02d:%02d:%04d]-[%02d:%02d:%02d]_%s_STATUS(%s)\n",
                t->tm_mday, t->tm_mon + 1, t->tm_year + 1900,
                t->tm_hour, t->tm_min, t->tm_sec,
                process_name, status);
        fclose(log_file);
    }
}

// a
void list_processes(const char *user) {
    char *args[] = {"/bin/ps", "aux", NULL};
    execve(args[0], args, NULL);
}

// b
void daemon_mode(const char *user) {
    log_status("daemon", "RUNNING");
    printf("Monitoring user %s in daemon mode.\n", user);
}

// c
void stop_mode(const char *user) {
    log_status("stop", "RUNNING");
    printf("Stopped monitoring user %s.\n", user);
}

// d
void fail_mode(const char *user) {
    log_status("fail", "FAILED");
    printf("All processes for user %s have been failed.\n", user);
}

// e
void revert_mode(const char *user) {
    log_status("revert", "RUNNING");
    printf("User  %s can now run processes again.\n", user);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: ./debugmon <command> <user>\n");
        return 1;
    }

    const char *command = argv[1];
    const char *user = argv[2];

    if (strcmp(command, "list") == 0) {
        list_processes(user);
    } else if (strcmp(command, "daemon") == 0) {
        daemon_mode(user);
    } else if (strcmp(command, "stop") == 0) {
        stop_mode(user);
    } else if (strcmp(command, "fail") == 0) {
        fail_mode(user);
    } else if (strcmp(command, "revert") == 0) {
        revert_mode(user);
    } else {
        printf("Unknown command: %s\n", command);
        return 1;
    }

    return 0;
}
