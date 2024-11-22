#include "../include/Utility.h"
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <limits.h>
#include <sys/wait.h>
#include <string.h>

void listFileDescriptors()
{
    char fd_path[4096];
    char target_path[4096];
    ssize_t len;

    // Get the process ID of the current process
    pid_t pid = getpid();

    // Construct the path to the fd directory for the current process
    snprintf(fd_path, sizeof(fd_path), "/proc/%d/fd", pid);

    // Open the fd directory
    DIR *dir = opendir(fd_path);
    if (!dir) {
        perror("opendir");
        return;
    }

    printf("File descriptors for process %d:\n", pid);

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        // Skip "." and ".."
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        // Construct the full path to the file descriptor
        snprintf(fd_path, sizeof(fd_path), "/proc/%d/fd/%s", pid, entry->d_name);

        // Read the target of the symlink
        len = readlink(fd_path, target_path, sizeof(target_path) - 1);
        if (len != -1) {
            target_path[len] = '\0';  // Null-terminate the string
            printf("FD %s -> %s\n", entry->d_name, target_path);
        } else {
            perror("readlink");
        }
    }

    closedir(dir);    
}