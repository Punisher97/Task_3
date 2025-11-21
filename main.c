#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* ==== Объявляем то, что реализовано в list.c (без заголовка) ==== */
extern char **lst;
extern int sizelist;
extern int curlist;

void null_list(void);
void clearlist(void);
void termlist(void);
void sortlist(void);
void printlist(void);
int  list_ensure_capacity(int want);
int  list_append(char *s);

/* ==== Буфер текущего слова и ввод ==== */
#define SIZE 16
#define BLK  64

int c;          /* текущий символ (int для EOF) */
char *buf;      /* буфер текущего слова */
int sizebuf;    /* ёмкость буфера слова */
int curbuf;     /* длина слова */

/* Чтение блоками через fscanf + getsym() */
FILE *IN;
int rd_eof, rd_have_nl, rd_last, rd_pos;
char rd_block[BLK+1];

int refill(void){
    rd_last = 0; rd_pos = 0;
    if (rd_eof) return 0;

    char fmt[32];
    snprintf(fmt, sizeof(fmt), "%%%d[^\n]", BLK);
    {
        int matched = fscanf(IN, fmt, rd_block); /* до BLK, не включая '\n' */
        if (matched == 1) {
            rd_last = (int)strlen(rd_block);
            int ch = fgetc(IN);
            if (ch == '\n') rd_have_nl = 1;
            else if (ch == EOF) rd_eof = 1;
            else ungetc(ch, IN);
            return 1;
        }
    }
    {
        int ch = fgetc(IN);
        if (ch == '\n') { rd_have_nl = 1; return 1; }
        if (ch == EOF) { rd_eof = 1; return 0; }
        rd_block[0] = (char)ch; rd_block[1] = '\0'; rd_last = 1;
        ch = fgetc(IN);
        if (ch == '\n') rd_have_nl = 1;
        else if (ch == EOF) rd_eof = 1;
        else ungetc(ch, IN);
        return 1;
    }
}

int getsym(void){
    if (rd_pos < rd_last) return (unsigned char)rd_block[rd_pos++];
    if (rd_have_nl) { rd_have_nl = 0; rd_last = 0; rd_pos = 0; return '\n'; }
    if (!refill()) return EOF;
    if (rd_pos < rd_last) return (unsigned char)rd_block[rd_pos++];
    if (rd_have_nl) { rd_have_nl = 0; return '\n'; }
    return getsym(); /* на случай пустого сегмента */
}

/* Буфер слова  */
void nullbuf(void){
    buf=NULL; sizebuf=0; curbuf=0;
}
void addsym(void){
    if (curbuf > sizebuf-1) {
        char *tmp = (char*)realloc(buf, (size_t)(sizebuf += SIZE));
        if (!tmp) { fprintf(stderr,"error: OOM (grow word)\n"); exit(1); }
        buf = tmp;
    }
    buf[curbuf++] = (char)c;
}
void addword(void){
    if (curbuf > sizebuf-1) {
        char *tmp = (char*)realloc(buf, (size_t)(sizebuf += 1));
        if (!tmp) { fprintf(stderr,"error: OOM (ensure NUL)\n"); exit(1); }
        buf = tmp;
    }
    buf[curbuf++] = '\0';
    {
        char *tight = (char*)realloc(buf, (size_t)curbuf);
        if (tight) { buf = tight; sizebuf = curbuf; }
    }
    if (list_append(buf) != 0) {
        fprintf(stderr, "error: OOM (list append)\n");
        free(buf);
    }
    nullbuf();
}

void print_shortest_words(void) {
    
    /* Находим минимальную длину */
    int min_length = -1;

    for (int i = 0; i < curlist; i++) {
        if (lst[i] != NULL) {
            int len = strlen(lst[i]);
            if (min_length == -1 || len < min_length) {
                min_length = len;
            }
        }
    }


    /* Выводим все слова минимальной длины */
    printf("Shortest words (length %d):\n", min_length);
    for (int i = 0; i < curlist; i++) {
        if (lst[i] != NULL && strlen(lst[i]) == min_length) {
            printf("%s\n", lst[i]);
        }
    }
    printf("\n");
}
/* «Простой» символ: всё, что не разделитель и не спец-оператор */
int symset(int ch){
    return ch!='\n' && ch!=' ' && ch!='\t' &&
           ch!='|'  && ch!='&' && ch!=';' &&
           ch!='>'  && ch!='<' && ch!='(' && ch!=')' &&
           ch!=EOF;
}


/* ===== main: конечный автомат ===== */
int main(void){
    /* init ввода */
    IN = stdin; rd_eof=0; rd_have_nl=0; rd_last=0; rd_pos=0; rd_block[0]='\0';

    typedef enum {Start, Word, Pipe, Pipe2, Amp, Amp2, Greater, Greater2, SingleSpec, Newline, Stop} vertex;
    vertex V=Start;

    c = getsym();
    null_list();
    nullbuf();

    while(1==1) switch(V){case Start:
        if (c==' ' || c=='\t') { c=getsym(); }
        else if (c==EOF) {
            if (curlist == 0){
                clearlist();
                V=Stop;
                break;
            }
            termlist();
            /* исходный порядок */
            printf("=== Original order ===\n");
            printlist();
            /* отсортированный порядок */
            sortlist();
            printf("=== Sorted order ===\n");
            printlist();
            /* слова наименьшей длины */
            printf("=== Shortest words ===\n");
            print_shortest_words();
    
            clearlist();  // Очищаем ПОСЛЕ вывода
            V=Stop;
        }
        else if (c=='\n') {
            termlist();
            if (curlist>0){
                printf("=== Original order ===\n");
                printlist();
                printf("=== Sorted order ===\n");
                sortlist();
                printlist();
                printf("=== Shortest words ===\n");
                print_shortest_words();
            }
            clearlist();  
            V=Start;      
            c=getsym();
        }
        else {
            nullbuf();
            addsym();
            if      (c=='|') V=Pipe;
            else if (c=='&') V=Amp;
            else if (c=='>') V=Greater;
            else if (c=='<' || c==';' || c=='(' || c==')') V=SingleSpec;
            else V=Word;
            c=getsym();
        }
        break;

    case Word:
        if (symset(c)) { addsym(); c=getsym(); }
        else { V=Start; addword(); }
        break;

    case Pipe:
        if (c=='|') { addsym(); c=getsym(); V=Pipe2; }
        else { V=Start; addword(); }
        break;

    case Pipe2:
        V=Start; addword(); break;

    case Amp:
        if (c=='&') { addsym(); c=getsym(); V=Amp2; }
        else { V=Start; addword(); }
        break;

    case Amp2:
        V=Start; addword(); break;

    case Greater:
        if (c=='>') { addsym(); c=getsym(); V=Greater2; }
        else { V=Start; addword(); }
        break;

    case Greater2:
        V=Start; addword(); break;

    case SingleSpec:
        V=Start; addword(); break;

    case Newline:
        clearlist();
        V=Start;
        break;

    case Stop:
        exit(0);
        break;
    }
}
