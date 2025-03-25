#ifndef DADOS_H
#define DADOS_H

#define NUM_WII_MENU 3
#define NUM_USB_LOADER 2
#define NUM_HBC 2

typedef struct {
    char nome[50];
    char descricao[100];
    char url_download[200];
} Tema;

static Tema WiiMenu[NUM_WII_MENU] = {
    {"Tema 1", "Tema clássico do Wii", "https://example.com/wii1"},
    {"Tema 2", "Tema escuro para Wii", "https://example.com/wii2"},
    {"Tema 3", "Tema alternativo", "https://example.com/wii3"}
};

static Tema USBLoaderGX[NUM_USB_LOADER] = {
    {"Tema 1", "Tema padrão do USB Loader GX", "https://example.com/usbA"},
    {"Tema 2", "Tema minimalista", "https://example.com/usbB"}
};

static Tema HBC[NUM_HBC] = {
    {"Tema 1", "Tema azul do HBC", "https://example.com/hbcX"},
    {"Tema 2", "Tema moderno para HBC", "https://example.com/hbcY"}
};

#endif
