#include <stdio.h>
#include <stdlib.h>
#include <gccore.h>
#include <wiiuse/wpad.h>
#include <unistd.h>

static void *xfb = NULL;
static GXRModeObj *rmode = NULL;

int initialize() {
    VIDEO_Init();
	WPAD_Init();
	rmode = VIDEO_GetPreferredMode(NULL);

	xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));

	console_init(xfb,20,20,rmode->fbWidth-20,rmode->xfbHeight-20,rmode->fbWidth*VI_DISPLAY_PIX_SZ);

	VIDEO_Configure(rmode);
	VIDEO_SetNextFramebuffer(xfb);
	VIDEO_ClearFrameBuffer(rmode, xfb, COLOR_BLACK);
	VIDEO_SetBlack(false);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	if(rmode->viTVMode&VI_NON_INTERLACE) VIDEO_WaitVSync();
    return 0;
}

int initialize_network() {
    // Placeholder for network initialization
    return 0;
}

int fetch_data() {
    // Placeholder for Data Fetching
    return 0;
}

int parse_json() {
    // Placeholder for JSON parsing
    return 0;
}

int main() {
    initialize();

	printf("\x1b[2;0H");
	printf("Welcome to miiShop!\n");
    for (int i = 0; i < 5; i++) {
    printf("\rFetching the latest version"); // \r volta pro início
    for (int j = 0; j <= i; j++) printf(".");
    printf("\033[K"); // limpa resto da linha
    fflush(stdout);
    sleep(1);
}

	while(1) {
		WPAD_ScanPads();
		u32 pressed = WPAD_ButtonsDown(0);
		if ( pressed & WPAD_BUTTON_HOME ) exit(0);
		VIDEO_WaitVSync();
	}

	return 0;
}
