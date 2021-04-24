#ifndef API_H_
#define API_H_

#include <stdbool.h>

void show(char* path);

int list(char* path, int image_fd);
int list_output(char* path, int image_fd, char* out);

int make_dir(char* path, char* dirname, int image_fd);

int change_directory(char* current_path, char* dirname, int image_fd);

int touch(char* path, char* filename, int image_fd);

int echo(char* path, char* filename, char* content, bool eof, int image_fd);

int cat(char* path, char* filename, int image_fd);
int cat_output(char* path, char* filename, int image_fd, char* out);

int remove_file(char* path, char* filename, int image_fd);

int pull(char* src, char* dest, char* current_path, int image_fd);

int push(char* current_path, char* filename, char* dst, int image_fd);

#endif // API_H_