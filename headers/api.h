#ifndef API_H_
#define API_H_

#include <stdbool.h>

void show(char* path);

void list(char* path, int image_fd);

void make_dir(char* path, char* dirname, int image_fd);

void change_directory(char* current_path, char* dirname, int image_fd);

void touch(char* path, char* filename, int image_fd);

void echo(char* path, char* filename, char* content, bool eof, int image_fd);

void cat(char* path, char* filename, int image_fd);

void remove_file(char* path, char* filename, int image_fd);

#endif // API_H_