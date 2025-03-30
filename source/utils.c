#include <stdio.h>
#include <stdlib.h>
#include "utils.h"

char* lerArquivo(const char* caminho) {
    FILE *file = fopen(caminho, "r");
    if (!file) return NULL;
    
    fseek(file, 0, SEEK_END);
    long tamanho = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    char *conteudo = (char*)malloc(tamanho + 1);
    fread(conteudo, 1, tamanho, file);
    conteudo[tamanho] = '\0';
    
    fclose(file);
    return conteudo;
}