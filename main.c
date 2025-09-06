#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>

#define MAX_PATH 1024
#define MAX_CONTENT 65536
#define MAX_LOCALES 10
#define MAX_LOCALE_ENTRIES 100

typedef struct {
    char key[256];
    char value[1024];
} LocaleEntry;

typedef struct {
    char code[8];
    LocaleEntry entries[MAX_LOCALE_ENTRIES];
    int count;
} Locale;

typedef struct {
    Locale locales[MAX_LOCALES];
    int count;
    int has_locales;
} LocaleData;

// Safe string copy
void safe_strcpy(char *dest, const char *src, size_t dest_size) {
    if (!dest || !src || dest_size == 0) return;
    strncpy(dest, src, dest_size - 1);
    dest[dest_size - 1] = '\0';
}

// Utility functions
int create_directory(const char *path) {
    if (!path) return -1;
    
    char tmp[MAX_PATH];
    char *p = NULL;
    size_t len;

    safe_strcpy(tmp, path, sizeof(tmp));
    len = strlen(tmp);
    if (len > 0 && tmp[len - 1] == '/')
        tmp[len - 1] = 0;

    for (p = tmp + 1; *p; p++) {
        if (*p == '/') {
            *p = 0;
            if (mkdir(tmp, 0755) != 0 && errno != EEXIST) {
                return -1;
            }
            *p = '/';
        }
    }
    if (mkdir(tmp, 0755) != 0 && errno != EEXIST) {
        return -1;
    }
    return 0;
}

int copy_file(const char *src, const char *dest) {
    if (!src || !dest) return -1;
    
    FILE *source = fopen(src, "rb");
    if (!source) {
        printf("Warning: Could not open source file: %s\n", src);
        return -1;
    }

    // Create directory for destination file
    char dest_dir[MAX_PATH];
    safe_strcpy(dest_dir, dest, sizeof(dest_dir));
    char *last_slash = strrchr(dest_dir, '/');
    if (last_slash) {
        *last_slash = '\0';
        create_directory(dest_dir);
    }

    FILE *target = fopen(dest, "wb");
    if (!target) {
        printf("Warning: Could not create destination file: %s\n", dest);
        fclose(source);
        return -1;
    }

    char buffer[4096];
    size_t bytes;
    while ((bytes = fread(buffer, 1, sizeof(buffer), source)) > 0) {
        fwrite(buffer, 1, bytes, target);
    }

    fclose(source);
    fclose(target);
    return 0;
}

int copy_directory_recursive(const char *src, const char *dest) {
    if (!src || !dest) return -1;
    
    DIR *dir = opendir(src);
    if (!dir) {
        printf("Warning: Could not open directory: %s\n", src);
        return -1;
    }

    if (create_directory(dest) != 0) {
        printf("Warning: Could not create directory: %s\n", dest);
        closedir(dir);
        return -1;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        char src_path[MAX_PATH];
        char dest_path[MAX_PATH];
        snprintf(src_path, sizeof(src_path), "%s/%s", src, entry->d_name);
        snprintf(dest_path, sizeof(dest_path), "%s/%s", dest, entry->d_name);

        struct stat statbuf;
        if (stat(src_path, &statbuf) == 0) {
            if (S_ISDIR(statbuf.st_mode)) {
                copy_directory_recursive(src_path, dest_path);
            } else {
                copy_file(src_path, dest_path);
            }
        }
    }

    closedir(dir);
    return 0;
}

int load_locale_file(const char *filepath, Locale *locale) {
    if (!filepath || !locale) return -1;
    
    FILE *file = fopen(filepath, "r");
    if (!file) {
        printf("Warning: Could not open locale file: %s\n", filepath);
        return -1;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (length <= 0 || length > 100000) {
        printf("Warning: Invalid file size for locale: %s\n", filepath);
        fclose(file);
        return -1;
    }

    char *content = malloc(length + 1);
    if (!content) {
        printf("Error: Memory allocation failed\n");
        fclose(file);
        return -1;
    }

    size_t read_bytes = fread(content, 1, length, file);
    content[read_bytes] = '\0';
    fclose(file);

    // Simple JSON parsing - extract key-value pairs
    locale->count = 0;
    char *pos = content;

    while (*pos && locale->count < MAX_LOCALE_ENTRIES) {
        // Find opening quote for key
        char *quote1 = strchr(pos, '"');
        if (!quote1) break;
        quote1++;

        // Find closing quote for key
        char *quote2 = strchr(quote1, '"');
        if (!quote2) break;

        size_t key_len = quote2 - quote1;
        if (key_len >= sizeof(locale->entries[0].key) || key_len == 0) {
            pos = quote2 + 1;
            continue;
        }

        // Copy key
        strncpy(locale->entries[locale->count].key, quote1, key_len);
        locale->entries[locale->count].key[key_len] = '\0';

        // Find colon
        char *colon = strchr(quote2, ':');
        if (!colon) {
            pos = quote2 + 1;
            continue;
        }

        // Find opening quote for value
        char *quote3 = strchr(colon, '"');
        if (!quote3) {
            pos = quote2 + 1;
            continue;
        }
        quote3++;

        // Find closing quote for value
        char *quote4 = strchr(quote3, '"');
        if (!quote4) {
            pos = quote3;
            continue;
        }

        size_t value_len = quote4 - quote3;
        if (value_len >= sizeof(locale->entries[0].value)) {
            pos = quote4 + 1;
            continue;
        }

        // Copy value
        strncpy(locale->entries[locale->count].value, quote3, value_len);
        locale->entries[locale->count].value[value_len] = '\0';

        locale->count++;
        pos = quote4 + 1;
    }

    free(content);
    printf("Loaded locale with %d entries from %s\n", locale->count, filepath);
    return 0;
}

int load_locales(LocaleData *locale_data) {
    if (!locale_data) return -1;
    
    locale_data->has_locales = 0;
    locale_data->count = 0;
    
    DIR *dir = opendir("locale");
    if (!dir) {
        printf("No locale directory found\n");
        return 0;
    }

    locale_data->has_locales = 1;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL && locale_data->count < MAX_LOCALES) {
        if (strstr(entry->d_name, ".json")) {
            char filepath[MAX_PATH];
            snprintf(filepath, sizeof(filepath), "locale/%s", entry->d_name);

            // Extract locale code from filename
            char locale_code[256];
            safe_strcpy(locale_code, entry->d_name, sizeof(locale_code));
            char *dot = strrchr(locale_code, '.');
            if (dot) *dot = '\0';

            if (strlen(locale_code) < sizeof(locale_data->locales[0].code)) {
                safe_strcpy(locale_data->locales[locale_data->count].code, locale_code, 
                           sizeof(locale_data->locales[locale_data->count].code));
                
                if (load_locale_file(filepath, &locale_data->locales[locale_data->count]) == 0) {
                    locale_data->count++;
                }
            }
        }
    }

    closedir(dir);
    return 0;
}

char* get_locale_value(const Locale *locale, const char *key) {
    if (!locale || !key) return NULL;
    
    for (int i = 0; i < locale->count; i++) {
        if (strcmp(locale->entries[i].key, key) == 0) {
            return locale->entries[i].value;
        }
    }
    return NULL;
}

char* load_file(const char *path) {
    if (!path) return NULL;
    
    FILE *file = fopen(path, "r");
    if (!file) return NULL;

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (length <= 0 || length > MAX_CONTENT) {
        fclose(file);
        return NULL;
    }

    char *content = malloc(length + 1);
    if (!content) {
        fclose(file);
        return NULL;
    }

    size_t read_bytes = fread(content, 1, length, file);
    content[read_bytes] = '\0';
    fclose(file);

    return content;
}

char* load_partial(const char *name) {
    if (!name) return NULL;
    
    char path[MAX_PATH];
    snprintf(path, sizeof(path), "src/partials/%s.html", name);
    return load_file(path);
}

char* process_template(const char *content, const Locale *locale) {
    if (!content) return NULL;
    
    size_t content_len = strlen(content);
    char *result = malloc(MAX_CONTENT);
    if (!result) return NULL;

    char *output = result;
    const char *input = content;
    size_t remaining = MAX_CONTENT - 1;

    while (*input && remaining > 1) {
        if (strncmp(input, "<!-- %include.", 14) == 0) {
            // Handle includes
            input += 14;
            const char *end = strstr(input, "% -->");
            if (end) {
                size_t name_len = end - input;
                if (name_len < 255) {
                    char partial_name[256];
                    strncpy(partial_name, input, name_len);
                    partial_name[name_len] = '\0';

                    char *partial_content = load_partial(partial_name);
                    if (partial_content) {
                        char *processed_partial = process_template(partial_content, locale);
                        if (processed_partial) {
                            size_t partial_len = strlen(processed_partial);
                            if (partial_len < remaining) {
                                strcpy(output, processed_partial);
                                output += partial_len;
                                remaining -= partial_len;
                            }
                            free(processed_partial);
                        }
                        free(partial_content);
                    }
                }
                input = end + 5;
            } else {
                *output++ = *input++;
                remaining--;
            }
        } else if (strncmp(input, "%locale.", 8) == 0) {
            // Handle locale variables
            input += 8;
            const char *end = strchr(input, '%');
            if (end && locale) {
                size_t key_len = end - input;
                if (key_len < 255) {
                    char key[256];
                    strncpy(key, input, key_len);
                    key[key_len] = '\0';

                    char *value = get_locale_value(locale, key);
                    if (value) {
                        size_t value_len = strlen(value);
                        if (value_len < remaining) {
                            strcpy(output, value);
                            output += value_len;
                            remaining -= value_len;
                        }
                    }
                }
                input = end + 1;
            } else {
                *output++ = *input++;
                remaining--;
            }
        } else {
            *output++ = *input++;
            remaining--;
        }
    }

    *output = '\0';
    return result;
}

int process_pages_directory(const char *src_dir, const char *build_dir, 
                           const char *relative_path, const LocaleData *locale_data) {
    if (!src_dir || !build_dir || !relative_path || !locale_data) return -1;
    
    char full_src_path[MAX_PATH];
    if (strlen(relative_path) > 0) {
        snprintf(full_src_path, sizeof(full_src_path), "%s/%s", src_dir, relative_path);
    } else {
        safe_strcpy(full_src_path, src_dir, sizeof(full_src_path));
    }

    DIR *dir = opendir(full_src_path);
    if (!dir) {
        printf("Warning: Could not open pages directory: %s\n", full_src_path);
        return -1;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        char src_path[MAX_PATH];
        char rel_path[MAX_PATH];
        
        snprintf(src_path, sizeof(src_path), "%s/%s", full_src_path, entry->d_name);
        
        if (strlen(relative_path) > 0) {
            snprintf(rel_path, sizeof(rel_path), "%s/%s", relative_path, entry->d_name);
        } else {
            safe_strcpy(rel_path, entry->d_name, sizeof(rel_path));
        }

        struct stat statbuf;
        if (stat(src_path, &statbuf) == 0) {
            if (S_ISDIR(statbuf.st_mode)) {
                process_pages_directory(src_dir, build_dir, rel_path, locale_data);
            } else if (strstr(entry->d_name, ".html")) {
                printf("Processing: %s\n", rel_path);
                
                char *content = load_file(src_path);
                if (!content) {
                    printf("Warning: Could not load file: %s\n", src_path);
                    continue;
                }

                if (locale_data->has_locales && locale_data->count > 0) {
                    for (int i = 0; i < locale_data->count; i++) {
                        char output_path[MAX_PATH];
                        snprintf(output_path, sizeof(output_path), "%s/%s/%s", 
                                build_dir, locale_data->locales[i].code, rel_path);

                        // Create directory structure
                        char output_dir[MAX_PATH];
                        safe_strcpy(output_dir, output_path, sizeof(output_dir));
                        char *last_slash = strrchr(output_dir, '/');
                        if (last_slash) {
                            *last_slash = '\0';
                            create_directory(output_dir);
                        }

                        char *processed = process_template(content, &locale_data->locales[i]);
                        if (processed) {
                            FILE *output_file = fopen(output_path, "w");
                            if (output_file) {
                                fputs(processed, output_file);
                                fclose(output_file);
                            } else {
                                printf("Warning: Could not write file: %s\n", output_path);
                            }
                            free(processed);
                        }
                    }
                } else {
                    char output_path[MAX_PATH];
                    snprintf(output_path, sizeof(output_path), "%s/%s", build_dir, rel_path);

                    // Create directory structure
                    char output_dir[MAX_PATH];
                    safe_strcpy(output_dir, output_path, sizeof(output_dir));
                    char *last_slash = strrchr(output_dir, '/');
                    if (last_slash) {
                        *last_slash = '\0';
                        create_directory(output_dir);
                    }

                    char *processed = process_template(content, NULL);
                    if (processed) {
                        FILE *output_file = fopen(output_path, "w");
                        if (output_file) {
                            fputs(processed, output_file);
                            fclose(output_file);
                        } else {
                            printf("Warning: Could not write file: %s\n", output_path);
                        }
                        free(processed);
                    }
                }

                free(content);
            }
        }
    }

    closedir(dir);
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 2 || strcmp(argv[1], "build") != 0) {
        printf("Usage: %s build\n", argv[0]);
        return 1;
    }

    printf("Building Jelly CMS...\n");

    // Create build directory
    if (create_directory("build") != 0) {
        printf("Error: Could not create build directory\n");
        return 1;
    }

    // Copy vendor directory
    if (access("vendor", F_OK) == 0) {
        printf("Copying vendor directory...\n");
        copy_directory_recursive("vendor", "build/vendor");
    } else {
        printf("No vendor directory found\n");
    }

    // Copy public directory contents
    if (access("public", F_OK) == 0) {
        printf("Copying public directory...\n");
        DIR *public_dir = opendir("public");
        if (public_dir) {
            struct dirent *entry;
            while ((entry = readdir(public_dir)) != NULL) {
                if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                    continue;

                char src_path[MAX_PATH];
                char dest_path[MAX_PATH];
                snprintf(src_path, sizeof(src_path), "public/%s", entry->d_name);
                snprintf(dest_path, sizeof(dest_path), "build/%s", entry->d_name);

                struct stat statbuf;
                if (stat(src_path, &statbuf) == 0) {
                    if (S_ISDIR(statbuf.st_mode)) {
                        copy_directory_recursive(src_path, dest_path);
                    } else {
                        copy_file(src_path, dest_path);
                    }
                }
            }
            closedir(public_dir);
        }
    } else {
        printf("No public directory found\n");
    }

    // Copy assets directory
    if (access("assets", F_OK) == 0) {
        printf("Copying assets directory...\n");
        copy_directory_recursive("assets", "build/assets");
    } else {
        printf("No assets directory found\n");
    }

    // Load locales
    LocaleData locale_data;
    if (load_locales(&locale_data) != 0) {
        printf("Error loading locales\n");
        return 1;
    }

    if (locale_data.has_locales && locale_data.count > 0) {
        printf("Found %d locales: ", locale_data.count);
        for (int i = 0; i < locale_data.count; i++) {
            printf("%s ", locale_data.locales[i].code);
        }
        printf("\n");
    } else {
        printf("No locales found, building single language version\n");
    }

    // Process pages
    if (access("src/pages", F_OK) == 0) {
        printf("Processing pages...\n");
        if (process_pages_directory("src/pages", "build", "", &locale_data) != 0) {
            printf("Error processing pages\n");
            return 1;
        }
    } else {
        printf("No src/pages directory found\n");
    }

    printf("Build completed successfully!\n");
    return 0;
}