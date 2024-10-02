#pragma once

// api

int fork();

int execve(char *path, char *const argv[], char *const envp[]);

ssize_t write(int fd, const void *buf, size_t cnt);

ssize_t read(int fd, void *buf, size_t cnt);

int close(int fd);

