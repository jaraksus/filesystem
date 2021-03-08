#ifndef API_H_
#define API_H_

void show(char* path);

void list(char* path, int image_fd);

void make_dir(char* path, char* dirname, int image_fd);

void change_directory(char* current_path, char* dirname, int image_fd);

#endif // API_H_