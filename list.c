#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 16  /* шаг роста списка */

/* ===== Глобальные данные списка ===== */
char **lst = NULL;   /* массив указателей на строки */
int sizelist = 0;    /* ёмкость массива (элементов) */
int curlist  = 0;    /* текущее число строк (индекс свободной позиции) */

/* Инициализация (пустой список) */
void null_list(void){
    lst = NULL;
    sizelist = 0;
    curlist = 0;
}

/* Освобождение всех строк и массива; делаем список пустым */
void clearlist(void){
    int i;
    if (lst != NULL) {
        for (i = 0; i < curlist; ++i) free(lst[i]);
        free(lst);
    }
    lst = NULL;
    sizelist = 0;
    curlist = 0;
}

/* Внутренняя: обеспечить ёмкость не менее want */
int list_ensure_capacity(int want){
    if (want <= sizelist) return 0;

    /* растим блоками по SIZE */
    {
        int cap = (sizelist ? sizelist : SIZE);
        while (cap < want) cap += SIZE;
        char **tmp = (char**)realloc(lst, (size_t)cap * sizeof(*tmp));
        if (!tmp) return -1;
        lst = tmp;
        sizelist = cap;
    }
    return 0;
}

/* Добавить ГОТОВУЮ строку (владение переходит контейнеру). 0 — ок, -1 — OOM */
int list_append(char *s){
    if (list_ensure_capacity(curlist + 1 + 1) != 0) return -1; /* +1 под NULL-терминатор */
    lst[curlist++] = s;
    return 0;
}

/* Завершить список: поставить NULL-терминатор и (опц.) подрезать память */
void termlist(void){
    char **tmp;
    if (lst == NULL) return;
    if (list_ensure_capacity(curlist + 1) != 0) {
        fprintf(stderr, "error: OOM (termlist grow)\n");
        return;
    }
    lst[curlist] = NULL;

    /* Подрезка до точного размера (необязательно) */
    tmp = (char**)realloc(lst, (size_t)(curlist + 1) * sizeof(*tmp));
    if (tmp) { lst = tmp; sizelist = curlist + 1; }
}

/* Пузырьковая сортировка lst[0..curlist-1] по strcmp */
void sortlist(void){
    int n, i, swapped;
    if (curlist < 2) return;
    for (n = curlist; n > 1; --n){
        swapped = 0;
        for (i = 0; i + 1 < n; ++i){
            if (strcmp(lst[i], lst[i+1]) > 0){
                char *t = lst[i]; lst[i] = lst[i+1]; lst[i+1] = t;
                swapped = 1;
            }
        }
        if (!swapped) break;
    }
}

/* Печать: длина (curlist), затем строки по одной в строке */
void printlist(void){
    int i;
    printf("%d\n", curlist);
    for (i = 0; i < curlist; ++i) {
        printf("%s\n", lst[i] ? lst[i] : "");
    }
}
