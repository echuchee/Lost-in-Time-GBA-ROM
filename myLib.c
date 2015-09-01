#include "myLib.h"
#include "tiles.h"
#include "letters.h"
#include <stdio.h>

unsigned short *videoBuffer = (u16 *) 0x6000000;

unsigned short *frontBuffer = (u16 *) 0x6000000;
unsigned short *backBuffer = (u16 *) 0x600A000;

int lc;
DMA *dma = (DMA *) 0x40000B0;

void setPixel3(int row, int col, unsigned short color) {
    videoBuffer[OFFSET(row, col, SCREENWIDTH)] = color;
}

void drawRect3(int row, int col, int height, int width, unsigned short color) {
    unsigned short c = color;

    int i;
    for (i = 0; i < height; i++) {
        DMANow(3, &c, &videoBuffer[OFFSET(row + i, col, SCREENWIDTH)], (width) | DMA_SOURCE_FIXED);
    }
}

void drawImage3(const unsigned short* image, int row, int col, int height, int width) {
    int i;
    for (i = 0; i < height; i++) {
        DMANow(3, (unsigned short*) &image[OFFSET(i, 0, width)], &videoBuffer[OFFSET(row + i, col, SCREENWIDTH)], (width));
    }
}

void fillScreen3(unsigned short color) {
    unsigned short c = color;

    DMANow(3, &c, videoBuffer, (240 * 160) | DMA_SOURCE_FIXED);

}

void setPixel4(int row, int col, unsigned char colorIndex) {
    unsigned short pixels = videoBuffer[OFFSET(row, col / 2, SCREENWIDTH / 2)];

    if (col % 2 == 0) // even
    {
        pixels &= 0xFF << 8;
        videoBuffer[OFFSET(row, col / 2, SCREENWIDTH / 2)] = pixels | colorIndex;
    } else // odd
    {
        pixels &= 0xFF;
        videoBuffer[OFFSET(row, col / 2, SCREENWIDTH / 2)] = pixels | colorIndex << 8;
    }
}

void drawRect4(int row, int col, int height, int width, unsigned char colorIndex) {
    unsigned short pixels = colorIndex << 8 | colorIndex;

    int r;
    for (r = 0; r < height; r++) {
        if (col % 2 == 0) // even starting col
        {
            DMANow(3, &pixels, &videoBuffer[OFFSET(row + r, col / 2, SCREENWIDTH / 2)], (width / 2) | DMA_SOURCE_FIXED);
            if (width % 2 == 1) // if width is odd
            {
                setPixel4(row + r, col + width - 1, colorIndex);
            }
        } else // old starting col
        {
            setPixel4(row + r, col, colorIndex);

            if (width % 2 == 1) // if width is odd
            {
                DMANow(3, &pixels, &videoBuffer[OFFSET(row + r, (col + 1) / 2, SCREENWIDTH / 2)], (width / 2) | DMA_SOURCE_FIXED);
            } else // width is even
            {
                DMANow(3, &pixels, &videoBuffer[OFFSET(row + r, (col + 1) / 2, SCREENWIDTH / 2)], ((width / 2) - 1) | DMA_SOURCE_FIXED);
                setPixel4(row + r, col + width - 1, colorIndex);
            }
        }
    }

}

void fillScreen4(unsigned char colorIndex) {
    unsigned short pixels = colorIndex << 8 | colorIndex;
    DMANow(3, &pixels, videoBuffer, ((240 * 160) / 2) | DMA_SOURCE_FIXED);
}

void drawBackgroundImage4(const unsigned short* image) {
    DMANow(3, (unsigned short*) image, videoBuffer, ((240 * 160) / 2));
}

void drawImage4(const unsigned short* image, int row, int col, int height, int width) {
    if (col % 2) {
        col++;
    }

    int r;
    for (r = 0; r < height; r++) {
        DMANow(3, (unsigned short*) &image[OFFSET(r, 0, width / 2)], &videoBuffer[OFFSET(row + r, col / 2, SCREENWIDTH / 2)], width / 2);
    }
}

void drawSubImage4(const unsigned short* sourceImage, int sourceRow, int sourceCol, int sourceWidth,
        int row, int col, int height, int width) {
    if (sourceCol % 2) {
        sourceCol++;
    }

    if (col % 2) {
        col++;
    }

    int r;
    for (r = 0; r < height; r++) {
        DMANow(3, (unsigned short*) &sourceImage[OFFSET(sourceRow + r, sourceCol / 2, width / 2)], &videoBuffer[OFFSET(row + r, col / 2, SCREENWIDTH / 2)], width / 2);
    }
}

void loadPalette(const unsigned short* palette) {
    DMANow(3, (unsigned short*) palette, PALETTE, 256);
}

void swapPalette(const unsigned short* palette, int i) {
    DMANow(3, (unsigned short*) &palette[16 * i], PALETTE, (256 - (16 * i)));
}

void swapSpritePalette(const unsigned short* palette, int i) {
    DMANow(3, (unsigned short*) &palette[16 * i], SPRITE_PALETTE, (256 - (16 * i)));
}

void DMANow(int channel, void* source, void* destination, unsigned int control) {
    dma[channel].src = source;
    dma[channel].dst = destination;
    dma[channel].cnt = DMA_ON | control;
}

void waitForVblank() {
    while (SCANLINECOUNTER > 160);
    while (SCANLINECOUNTER < 160);
}

void flipPage() {
    if (REG_DISPCTL & BACKBUFFER) {
        REG_DISPCTL &= ~BACKBUFFER;
        videoBuffer = backBuffer;
    } else {
        REG_DISPCTL |= BACKBUFFER;
        videoBuffer = frontBuffer;
    }
}

void drawChar0(int row, int col, char ch) {
    if (ch >= 64) {
        char new = ch - 64;
        SCREENBLOCKBASE[17].tilemap[(row * 32) + col] = (new & 0xff);
    } else if (ch >= 48 && ch <= 57) {
        char new = 35 + ch - 48;
        SCREENBLOCKBASE[17].tilemap[(row * 32) + col] = (new & 0xff);
    }


}

void drawString0(int row, int col, char *str) {
    while (*str != '\0') {
        drawChar0(row, col, *str);
        str++;
        col += 1;
        if (col == 28) {
            row += 1;
            col = 2;
        }
    }
}

void clearText() {
    int i;
    for (i = 0; i < 1024; i++) {
        SCREENBLOCKBASE[17].tilemap[i] = (0 & 0xff);
    }
}

void clearBG1() {
    int i;
    for (i = 0; i < 1024 * 16; i++) {
        SCREENBLOCKBASE[0].tilemap[i] = (149 & 0xff);
    }
}

void clearSun() {
    int i;
    for (i = 0; i < 1024; i++) {
        SCREENBLOCKBASE[19].tilemap[i] = (149 & 0xff);
    }
}

void displayHealth(int hp) {
    clearText();
    drawString0(0, 0, "HP");
    if (hp >= 2) {
        int temphp = hp;
        SCREENBLOCKBASE[17].tilemap[2] = (32 & 0xff);
        while (hp >= 2) {
            hp--;
            SCREENBLOCKBASE[17].tilemap[2 + hp] = (33 & 0xff);
        }
        SCREENBLOCKBASE[17].tilemap[2 + temphp] = (34 & 0xff);
    } else if (hp == 1) {
        SCREENBLOCKBASE[17].tilemap[2] = (31 & 0xff);

    }
}

void drawCloud(int z, int r) {
    if (r == 0) {
        SCREENBLOCKBASE[23].tilemap[z] = (0 & 0xff);
        SCREENBLOCKBASE[23].tilemap[z + 1] = (1 & 0xff);
        SCREENBLOCKBASE[23].tilemap[z + 2] = (2 & 0xff);
        SCREENBLOCKBASE[23].tilemap[z + 32] = (32 & 0xff);
        SCREENBLOCKBASE[23].tilemap[z + 33] = (33 & 0xff);
        SCREENBLOCKBASE[23].tilemap[z + 34] = (34 & 0xff);

    } else if (r == 1) {
        SCREENBLOCKBASE[23].tilemap[z] = (7 & 0xff);
        SCREENBLOCKBASE[23].tilemap[z + 1] = (8 & 0xff);
        SCREENBLOCKBASE[23].tilemap[z + 2] = (9 & 0xff);
        SCREENBLOCKBASE[23].tilemap[z + 32] = (39 & 0xff);
        SCREENBLOCKBASE[23].tilemap[z + 33] = (40 & 0xff);
        SCREENBLOCKBASE[23].tilemap[z + 34] = (41 & 0xff);
    } else if (r == 2) {
        SCREENBLOCKBASE[23].tilemap[z] = (3 & 0xff);
        SCREENBLOCKBASE[23].tilemap[z + 1] = (4 & 0xff);
        SCREENBLOCKBASE[23].tilemap[z + 2] = (5 & 0xff);
        SCREENBLOCKBASE[23].tilemap[z + 3] = (6 & 0xff);
        SCREENBLOCKBASE[23].tilemap[z + 32] = (35 & 0xff);
        SCREENBLOCKBASE[23].tilemap[z + 33] = (36 & 0xff);
        SCREENBLOCKBASE[23].tilemap[z + 34] = (37 & 0xff);
        SCREENBLOCKBASE[23].tilemap[z + 35] = (38 & 0xff);
    }
    else if (r ==4) {
        if (rand() %2 == 1) {
            SCREENBLOCKBASE[23].tilemap[z] = (10 & 0xff);
            SCREENBLOCKBASE[23].tilemap[z + 1] = (11 & 0xff);
            SCREENBLOCKBASE[23].tilemap[z + 32] = (42 & 0xff);
            SCREENBLOCKBASE[23].tilemap[z + 33] = (43 & 0xff);
        }
        else {
            SCREENBLOCKBASE[23].tilemap[z] = (21 & 0xff);
            SCREENBLOCKBASE[23].tilemap[z + 1] = (22 & 0xff);
            SCREENBLOCKBASE[23].tilemap[z + 2] = (23 & 0xff);
            SCREENBLOCKBASE[23].tilemap[z + 32] = (53 & 0xff);
            SCREENBLOCKBASE[23].tilemap[z + 33] = (54 & 0xff);
            SCREENBLOCKBASE[23].tilemap[z + 34] = (5 & 0xff);
        }
    }
}

void drawTree(int z, int r) {
    int i;
    int j;
    if (r == 0) {

        for (j = 0; j < 13; j++) {
            for (i = 0; i < 5; i++) {
                SCREENBLOCKBASE[28].tilemap[z + 64 - (32 * j) + i] = (448 + i - (32 * j));
            }
        }
    } else if (r == 1) {
        for (j = 0; j < 11; j++) {
            for (i = 0; i < 4; i++) {
                SCREENBLOCKBASE[28].tilemap[z + 64 - (32 * j) + i] = (453 + i - (32 * j));
            }
        }
    } else if (r == 2) {
        for (j = 0; j < 9; j++) {
            for (i = 0; i < 4; i++) {
                SCREENBLOCKBASE[28].tilemap[z + 64 - (32 * j) + i] = (457 + i - (32 * j));
            }
        }

    } else if (r == 3) {
        for (j = 0; j < 7; j++) {
            for (i = 0; i < 2; i++) {
                SCREENBLOCKBASE[28].tilemap[z + 64 - (32 * j) + i] = (461 + i - (32 * j));
            }
        }

    } else if (r == 4) {
        for (j = 0; j < 6; j++) {
            for (i = 0; i < 4; i++) {
                SCREENBLOCKBASE[28].tilemap[z + 64 - (32 * j) + i] = (463 + i - (32 * j));
            }
        }

    }
}

void drawSun() {
    clearSun();
    int i;
    for (i = 0; i < 1024; i++) {
        SCREENBLOCKBASE[19].tilemap[i] = (149 & 0xff);
    }
    SCREENBLOCKBASE[19].tilemap[92] = (10 & 0xff);
    SCREENBLOCKBASE[19].tilemap[93] = (11 & 0xff);
    SCREENBLOCKBASE[19].tilemap[124] = (42 & 0xff);
    SCREENBLOCKBASE[19].tilemap[125] = (43 & 0xff);
}

void clearMap() {
    int i;
    for (i = 0; i < 4096; i++) {
        SCREENBLOCKBASE[28].tilemap[i] = (149 & 0xff);
    }
}

void alterMap(int side, int level, int traveled) {
    int i;
    int test;
    int tempj = 5;
    if (side == 0) {
        for (i = 0; i < 800; i++) {
            SCREENBLOCKBASE[28].tilemap[i] = (149 & 0xff);
        }
        for (i = 800; i > 479; i--) {
            if ((i == 528 - 32) && (traveled == 5)) {
                if (level == 1 || level == 2) {
                        spawnPortal(i);
                }
            }
            if (i >= 512 + 32) {
                test = (rand() % 8) + 44;
                SCREENBLOCKBASE[28].tilemap[i] = (test & 0xff);
            } else if (i >= 512) {
                test = (rand() % 8) + 12;
                SCREENBLOCKBASE[28].tilemap[i] = (test & 0xff);
            }
            if (((rand() % 3) >= 1) && i < 508 && tempj >= 5 && traveled != 5) {
                drawTree(i, rand() % 5);
                tempj = 0;
            } else {
                tempj++;
            }

        }
    }
    if (side == 1) {
        for (i = 1024; i < 1824; i++) {
            SCREENBLOCKBASE[28].tilemap[i] = (149 & 0xff);
        }
        for (i = 1823; i > 1503; i--) {
            if (i >= 1568) {
                test = (rand() % 8) + 44;
                SCREENBLOCKBASE[28].tilemap[i] = (test & 0xff);
            } else if (i >= 1536) {
                test = (rand() % 8) + 12;
                SCREENBLOCKBASE[28].tilemap[i] = (test & 0xff);
            }
            if (((rand() % 3) >= 1) && i < 1532 && tempj >= 5 && traveled != 5) {
                drawTree(i, rand() % 5);
                tempj = 0;
            } else {
                tempj++;
            }

        }
    }
}
void spawnPortal(int z) {
        int i;
        int j;
        int k;
        int l;
        int m;
        int n;
        int o;
        int p;
        for (i = 0; i < 8; i++) {
            SCREENBLOCKBASE[28].tilemap[z + i - (32 * 7)] = (24 + i & 0xff);
        }
        for (j = 0; j < 8; j++) {
            SCREENBLOCKBASE[28].tilemap[z + j - (32 * 6)] = (56 + j & 0xff);
        }

        for (k = 0; k < 8; k++) {
            SCREENBLOCKBASE[28].tilemap[z + k - (32 * 5)] = (88 + k & 0xff);
        }
        for (l = 0; l < 8; l++) {
            SCREENBLOCKBASE[28].tilemap[z + l - (32 * 4)] = (120 + l & 0xff);
        }

        for (m = 0; m < 8; m++) {
            SCREENBLOCKBASE[28].tilemap[z + m - (32 * 3)] = (152 + m & 0xff);
        }
        for (n = 0; n < 8; n++) {
            SCREENBLOCKBASE[28].tilemap[z + n - 64] = (184 + n & 0xff);
        }
        for (o = 0; o < 8; o++) {
            SCREENBLOCKBASE[28].tilemap[z + o - 32] = (216 + o & 0xff);
        }
        for (p = 0; p < 8; p++) {
            SCREENBLOCKBASE[28].tilemap[z + p] = (248 + p & 0xff);
        }
}

void changeMap(int level) {
    if (level == 1 || level ==2 || level ==3) {
        clearMap();
        int i;
        int test;
        int q;
        int tempi = 36;
        int tempj = 5;
        int temp3 = 0;
        if (level ==1) {
                swapPalette(tilesPal, 3);
        }
        for (temp3 = 0; temp3 < 4096; temp3++) {
            SCREENBLOCKBASE[23].tilemap[temp3] = (20 & 0xff);
        }
        clearSun();
        if (level != 3) {
                drawSun();
        }
        alterMap(0, level, 0);
        alterMap(1, level, 0);

        for (q = 0; q < 2048; q += 512) {
            for (i = 0 + q; i < 800 + q; i++) {
                if (i <= 440 + q) {
                    tempi++;
                    if (level != 3) {
                        if ((rand() % 15 == 1) && (tempi >= 36) && (i % 32 <= 25)) {
                            tempi = 0;

                            drawCloud(i + q, rand() % 3);

                        }
                    } else if (tempi >= 5 && level == 3) {
                        tempi = 0;
                        drawCloud(i + q, rand() % 3);
                    }
                }
            }
            if (level == 3) {
                for (i = 0 + q; i < 800 + q; i++) {
                    if (i <= 440 + q) {
                        tempi++;
                            if (tempi >= 36 && rand() % 5 == 4 && (rand() % 25 >= 15)) {
                                tempi = 0;
                                drawCloud(i + q, 4);
                        }
                    }
                }
            }
        }
    }
}


void wait(int i) {
    volatile int k = 0;
    for (k = 0; k < i * 1000; k++) {

    }
}