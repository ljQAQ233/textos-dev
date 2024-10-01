#pragma once

// api

int fork();

ssize_t write(int fd, const void *buf, size_t cnt);

ssize_t read(int fd, void *buf, size_t cnt);

int close(int fd);

