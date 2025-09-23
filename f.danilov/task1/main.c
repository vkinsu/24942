#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // getuid, getgid, setpgid и тд (системные вызовы)
#include <sys/resource.h>
#include <string.h>
#define PATH_MAX 2048
#define llu unsigned long long

extern char **environ;
extern int optind, opterr, optopt;
extern char *optarg;

void print_usage() {
    printf("Опции:  \n");
    printf("  -i  Печатает реальные и эффективные идентификаторы пользователя и группы\n");
    printf("  -s  Процесс становится лидером группы\n");
    printf("  -p  Печатает идентификаторы процесса, процесса-родителя и группы процессов\n");
    printf("  -u  Печатает значение ulimit\n");
    printf("  -U new_ulimit  Изменяет значение ulimit\n");
    printf("  -c  Печатает размер в байтах core-файла\n");
    printf("  -C size  Изменяет размер core-файла\n");
    printf("  -d  Печатает текущую рабочую директорию\n");
    printf("  -v  Распечатывает переменные среды и их значения\n");
    printf("  -V name=value  Вносит новую переменную в среду или изменяет значение\n");
    printf("\n");
}

void user_group_ids() {
    printf("Реальный UID: %d\n", getuid());  // user id
    printf("Эффективный UID: %d\n", geteuid());

    printf("Реальный GID: %d\n", getgid()); // group id
    printf("Эффективный GID: %d\n", getegid());
}

void group_leader() {
    if (setpgid(0, 0) == -1) {
        perror("Ошибка при установке лидера группы");
    } else {
        printf("Процесс стал лидером группы. Новый PGID: %d\n", getpgid(0));
    }
}

void process_ids() {
    printf("PID: %d\n", getpid()); // идентификаторы процесса
    printf("PPID: %d\n", getppid()); // идентификаторы процесса-родителя 
    printf("PGID: %d\n", getpgid(0)); // идентификаторы группы процессов
}

void ulimit() {
    struct rlimit rlim;
    
    if (getrlimit(RLIMIT_FSIZE, &rlim) == 0) {
        printf("Текущий ulimit: %llu\n", (llu)rlim.rlim_cur);
        printf("Максимальный ulimit: %llu\n", (llu)rlim.rlim_max);
    } else perror("Ошибка при получении ulimit");
}

int new_ulimit(const char *value) {
    if (atol(value) <= 0) {
        fprintf(stderr, "Неверное значение ulimit: %s\n", value);
        return 1;
    }
    
    llu new_ulimit = atol(value);
    struct rlimit rlim;
    rlim.rlim_cur = new_ulimit;
    rlim.rlim_max = new_ulimit;
    
    if (setrlimit(RLIMIT_FSIZE, &rlim) == -1) {
        perror("Ошибка при установке ulimit");
        return 1;
    }
    
    printf("Ulimit установлен на: %llu\n", new_ulimit);
    return 0;
}

void core_size() {
    struct rlimit rlim;
    
    if (getrlimit(RLIMIT_CORE, &rlim) == 0) {
        printf("Размер core-файла: %llu байт\n", (llu)rlim.rlim_cur);
        printf("Максимальный размер core-файла: %llu байт\n", (llu)rlim.rlim_max);
    } 
    else perror("Ошибка при получении размера core-файла");
}

int set_core_size(const char *value) {
    if (atol(value) < 0) {
        fprintf(stderr, "Неверный размер core-файла: %s\n", value);
        return 1;
    }
    core_size();
    printf("\n");
    llu new_size = atol(value);
    struct rlimit rlim;
    rlim.rlim_cur = new_size;
    rlim.rlim_max = new_size;
    
    if (setrlimit(RLIMIT_CORE, &rlim) == -1) {
        perror("Ошибка при установке размера core-файла");
        return 1;
    }
    
    printf("Размер core-файла установлен на: %llu байт\n", new_size);
    return 0;
}

void current_dir() {
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd))) {
        printf("Текущая рабочая директория: %s\n", cwd);
    } 
    else perror("Ошибка при получении текущей директории");
}

void env() {
    for (char **env = environ; *env != NULL; env++) {
        printf("  %s\n", *env);
    }
}

int env_variable(const char *arg) {
    char *equals = strchr(arg, '=');
    if (!equals || equals == arg) {
        fprintf(stderr, "Неверный формат переменной среды: %s\n", arg);
        fprintf(stderr, "Используйте: -Vname=value\n");
        return 1;
    }
    
    // Выделяем имя и значение
    size_t name_len = equals - arg;
    char *name = malloc(name_len + 1);
    strncpy(name, arg, name_len);
    name[name_len] = '\0';
    
    char *value = equals + 1;
    
    if (setenv(name, value, 1) == -1) {
        perror("Ошибка при установке переменной среды");
        free(name);
        return 1;
    }
    
    printf("Установлена переменная среды: %s=%s\n", name, value);
    free(name);
    return 0;
}


int main(int argc, char *argv[]) {
    if (argc == 1) {
        printf("Опции не указаны.\n");
        print_usage();
        exit(EXIT_FAILURE);
    }

    // Создаем перевернутый массив аргументов
    char **reversed_argv = malloc(argc * sizeof(char*));
    reversed_argv[0] = argv[0];  // имя программы остается на месте
    
    // Переворачиваем аргументы (считываем справа налево)
    for (int i = 1; i < argc; i++) {
        reversed_argv[i] = argv[argc - i];  // argv[argc] == NULL, поэтому начинаем с argc-1
    }

    // Теперь optind будет работать с перевернутыми аргументами
    int opt;
    opterr = 0;  // Отключаем стандартные сообщения об ошибках
    
    while ((opt = getopt(argc, reversed_argv, "ispuU:cC:dvV:")) != -1) {
        switch (opt) {
            case 'i':
                printf("=== Информация о пользователях и группах ===\n");
                user_group_ids();
                printf("\n");
                break;
            case 's':
                printf("=== Установка лидера группы ===\n");
                group_leader();
                printf("\n");
                break;
            case 'p':
                printf("=== Идентификаторы процессов ===\n");
                process_ids();
                printf("\n");
                break;
            case 'u':
                printf("=== Текущий ulimit ===\n");
                ulimit();
                printf("\n");
                break;
            case 'U':
                printf("=== Установка нового ulimit ===\n");
                if (new_ulimit(optarg)) {
                    fprintf(stderr, "Не удалось установить ulimit\n");
                }
                printf("\n");
                break;
            case 'c':
                printf("=== Размер core-файла ===\n");
                core_size();
                printf("\n");
                break;
            case 'C':
                printf("=== Установка размера core-файла ===\n");
                if (set_core_size(optarg)) {
                    fprintf(stderr, "Не удалось установить размер core-файла\n");
                }
                printf("\n");
                break;
            case 'd':
                printf("=== Текущая рабочая директория ===\n");
                current_dir();
                printf("\n");
                break;
            case 'v':
                printf("=== Переменные среды ===\n");
                env();
                printf("\n");
                break;
            case 'V':
                printf("=== Установка переменной среды ===\n");
                if (env_variable(optarg)) {
                    fprintf(stderr, "Не удалось установить переменную среды\n");
                }
                printf("\n");
                break;
            case '?':
                fprintf(stderr, "Неизвестная опция или отсутствует аргумент: %c\n", optopt);
                print_usage();
                free(reversed_argv);
                exit(EXIT_FAILURE);
        }
    }

    free(reversed_argv);
    return 0;
}