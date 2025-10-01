#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include<sys/mman.h>
#include <sys/stat.h>

#define MAX_LINES 1000
#define MAX_LINE_LENGTH 1024

int line_count = 0;
int fd = -1;


// Структура для хранения позиций и длин строк
typedef struct {
    off_t offset;    // Позиция начала строки в файле
    size_t length;   // Длина строки (без \n)
    char buffer_data[MAX_LINE_LENGTH];
} LineInfo;

LineInfo lines[MAX_LINES];

static void alarmHandler(int signo){
    printf("\nВремя кончилось!\n");

    printf("Содержимое файла: \n");
    for (int i = 0; i < line_count; i++){
        printf("%s\n", lines[i].buffer_data);
    }
    
    if (fd != -1) {
        close(fd);
    }

    exit(1);
}

typedef struct FileMapping {
  int fd;
  size_t fsize;
  unsigned char* dataPtr;
} FileMapping;


int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Использование: %s <имя_файла>\n", argv[0]);
        return 1;
    }

    // Открываем файл
    int fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror("Ошибка открытия файла");
        return 1;
    }
    struct stat statbuf;
    if (fstat(fd, &statbuf) < 0){ 
        perror("fstat error");
    }
    // определить размер входного файла
    size_t fsize = (size_t)statbuf.st_size;
    

    // создаем отображение файла в память
    unsigned char* dataPtr = (unsigned char*)mmap(NULL, fsize, PROT_READ, MAP_PRIVATE, fd, 0);
    if(dataPtr == MAP_FAILED) {
        close(fd);
        return 1;
    }

    off_t current_offset = 0;
    char buffer[MAX_LINE_LENGTH];
    size_t bytes_read;
    size_t line_start = 0;
    // Построение таблицы отступов и длин строк
    printf("=== Построение таблицы отступов ===\n");
    printf("%zu\n", fsize);
    for (off_t i = 0; i < fsize; i++) {
        if (dataPtr[i] == '\n') {
            // Записываем информацию о строке
            if (line_count < MAX_LINES) {
                lines[line_count].offset = line_start;
                lines[line_count].length = i - line_start;
                
                // Копируем данные строки в буфер
                if (lines[line_count].length < MAX_LINE_LENGTH) {
                    memcpy(lines[line_count].buffer_data, &dataPtr[line_start], lines[line_count].length);
                    lines[line_count].buffer_data[lines[line_count].length] = '\0';
                } else {
                    // Если строка слишком длинная, копируем только часть
                    memcpy(lines[line_count].buffer_data, &dataPtr[line_start], MAX_LINE_LENGTH - 1);
                    lines[line_count].buffer_data[MAX_LINE_LENGTH - 1] = '\0';
                    lines[line_count].length = MAX_LINE_LENGTH - 1;
                }
                
                printf("Строка %d: offset=%d, length=%zu\n", 
                       line_count + 1, 
                       lines[line_count].offset, 
                       lines[line_count].length);
                
                line_count++;
            }
            
            line_start = i + 1; // Начало следующей строки
        }
    }



    if (line_start < fsize && line_count < MAX_LINES) {
        lines[line_count].offset = line_start;
        lines[line_count].length = fsize - line_start;
        memcpy(lines[line_count].buffer_data, 
               dataPtr + line_start, 
               lines[line_count].length);
        lines[line_count].buffer_data[lines[line_count].length] = '\0';

        printf("Строка %d: offset=%ld, length=%zu\n", 
                           line_count + 1, 
                           lines[line_count].offset, 
                           lines[line_count].length);

        line_count++;
    }

    signal(SIGALRM, alarmHandler); // устанавливаем обработчик сигнала
    // Основной цикл запросов
    while (1) {
        int line_num;
        printf("\nВведите номер строки (0 для выхода). У вас есть 5 секунд чтобы ввести данные:");
        alarm(5);

        if (scanf("%d", &line_num) != 1) {
            printf("Ошибка ввода\n");
            while (getchar() != '\n'); // Очистка буфера
            alarm(0);
            continue;
        }

        alarm(0); // отключение таймера

        if (line_num == 0) {
            printf("Завершение работы\n");
            break;
        }

        if (line_num < 1 || line_num > line_count) {
            printf("Некорректный номер строки");
            continue;
        }

        
        int index = line_num - 1;
        
        if (lines[index].length > 0) {
            printf("Строка %d: %s\n", line_num, lines[index].buffer_data);
        } else if (lines[index].length == 0) {
            printf("Строка пустая\n");
        } else {
            printf("Ошибка чтения строки\n");
        }
    }

    close(fd);
    return 0;
}