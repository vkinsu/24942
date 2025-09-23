#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#define MAX_LEN 1024 // максимальная длина строки

// структура хранения строк
typedef struct ListNode { 
    char * data;
    int num_str;
    struct ListNode *next;
} ListNode;

// Функция для добавлния узла
void add_node(ListNode ** list, char * new_str){
    ListNode * new_node = malloc(sizeof(ListNode)); 
    new_node->data = (char*)malloc(strlen(new_str) + 1);
    if (new_node == NULL || new_node->data == NULL) {
        perror("Ошибка выделения памяти");
        exit(EXIT_FAILURE);
    }

    strcpy(new_node->data, new_str);
    new_node->next = NULL;
    if (*list == NULL) {
        *list = new_node;     
    } else {
        ListNode * curr = *list;
        while (curr->next != NULL){
            curr = curr->next;
        }
        new_node->num_str = curr->num_str + 1;
        curr->next = new_node;
    }
}

// Функция для вывода всех строк из списка
void print_list(ListNode* list) {
    ListNode* current = list;
    int count = 1;
    printf("=== Все строки ===\n");
    while (current != NULL) {
        printf("%d: %s\n", count, current->data);
        current = current->next;
        count++;
    }
}


void open_file(const char *file_name, ListNode **list) {
    int file = open(file_name, O_RDONLY);
    if (file == -1) {
        perror("Ошибка при открытии файла");
        exit(EXIT_FAILURE);
    }

    char buffer[MAX_LEN];
    char line[MAX_LEN];
    ssize_t bytes_read;
    int line_index = 0;

    while ((bytes_read = read(file, buffer, sizeof(buffer) - 1)) > 0) {
        for (int i = 0; i < bytes_read; i++) {
            if (buffer[i] == '\n' || line_index >= MAX_LEN - 1) {
                line[line_index] = '\0';
                add_node(list, line);
                line_index = 0;
            } else {
                line[line_index++] = buffer[i];
            }
        }
    }

    // Обработка последней строки, если файл не заканчивается на '\n'
    if (line_index > 0) {
        line[line_index] = '\0';
        add_node(list, line);
    }

    if (bytes_read == -1) {
        perror("Ошибка при чтении файла");
    }

    if (close(file) == -1) {
        perror("Ошибка при закрытии файла");
    }
}

// Функция для поиска строки по номеру
void find_str(ListNode *list, int num) {
    ListNode *current = list;
    while (current != NULL) {
        if (current->num_str == num) {
            printf("Строка %d: %s\n", num, current->data);
            return;
        }
        current = current->next;
    }
    printf("Строка с номером %d не найдена\n", num);
}

// Функция для освобождения памяти
int free_memory(ListNode* list){
    ListNode * curr = list;
    while (curr != NULL){
        ListNode* next = curr->next;
        free(curr->data);
        free(curr);
        curr = next;
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Использование: %s <имя_файла>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    const char *file_name = argv[1];
    ListNode *list = NULL;

    open_file(file_name, &list);
    print_list(list);

    char input[20];
    int num;
    
    printf("Введите номер строки: ");
    if (scanf("%19s", input) != 1) {
        printf("Ошибка ввода\n");
        free_memory(list);
        return 1;
    }
    
    num = atoi(input);
    while (num <= 0) {
        if (num == 0){
            printf("Окончание работы\n");
            return 0;
        }
        
        printf("Введите корректный номер строки : ");
        if (scanf("%19s", input) != 1) {
            printf("Ошибка ввода\n");
            break;
        }
        num = atoi(input);
    }
    
    if (num > 0) {
        find_str(list, num-1);
    }

    free_memory(list);
    return 0;
}