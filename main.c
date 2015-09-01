#include "myLib.h"

#include "tiles.h"
#include "letters.h"
#include "s1.h"
#include "s2.h"
#include "bgMusic.h"
#include "bgMusic2.h"
#include "gunshot.h"
#include "pirateDeath.h"
#include "arrowSound.h"
#include <stdio.h>
#include "raptorDeath.h"
#include "splash.h"
#include "airDinoDeath.h"
#include "tiles2.h"
#include "s3.h"
#include "tiles3.h"
#include "clink.h"
#include "explode.h"
#include "bossMusic.h"
#include "menuMusic.h"
#include "victory.h"

unsigned int buttons;
unsigned int oldButtons;

int hOff = 0;
int vOff = 0;
int realHOff = 0;
int hOff2 = 0;
int vOff2 = 0;
int tempOff = 0;
int hOff3 = 0;
int vOff3 = 0;
int time = 0;
int day = 0;
int side = 0;
int started = 0;
int flip = 0;
int atkDelay = 0;
int changeAttack = 0;
int tempVar = 0;
int leftBound = 100;
int startT = 0;
int pauseDelay = 0;
int traveled = 0;
int charRow = 20;
int charCol = 20;
int charWidth = 16;
int charHeight = 32;
int charVelY = 0;
int moveCounter = 0;
int frame = 0;
int hp = 0;
int enemy1Delay = 0;
int hpLossDelay = 0;
int currentPalette = 0;
int currentSpritePalette = 0;
int invuln = 0;
int test;
int controlDelay = 0;
int stopFix = 0;
int projDelay;
int enemy2Delay = 0;
int corpseDelay1 = 0;
int corpseDelay2 = 0;
int enemyFrameCount = 0;
int level = 0;
int bgwait = 0;
int bgCurrentPalette = 0;
int crossed;
int expDelay = 0;
int airCorpseDelay1 = 0;
int airCorpseDelay2 = 0;
enemyGround boss;
int upDelay = 0;
int explosion = 0;
int bossDeathDelay = 0;
int initialDrop = 0;
int pq = 0;
int victPlayed = 0;


arrow arrows[10];
int arrowCount = 0;
laser lasers1[10];
int l1count = 0;
laser lasers2[10];
int l2count = 0;

enemyGround groundEnemies[2];
enemyRaptorCorpse groundCorpses[2];
enemyAir airEnemies[2];
int airFrames[10] = {0, 1, 2, 1, 0, 3, 5, 4, 5, 3};
projectile projectiles[4];
int projNum = 0;

int state;

OBJ_ATTR shadowOAM[128];

#define ROWMASK 0xFF
#define COLMASK 0x1FF

enum {
    IDLE, RIGHT, LEFT, ATTACK, JUMP, ATTACKJUMP
};

enum {
    STARTSCREEN, GAMESCREEN, PAUSESCREEN, LOSESCREEN, WINSCREEN, CONTROLS
};

typedef struct {
    unsigned char* data;
    int length;
    int frequency;
    int isPlaying;
    int loops;
    int duration;
} SOUND;

SOUND soundA;
SOUND soundB;
int vbCountA;
int vbCountB;
SOUND sounds[100];
int numSounds = 0;
int curSounds = 0;

void setupSounds() {
    REG_SOUNDCNT_X = SND_ENABLED;

    REG_SOUNDCNT_H = SND_OUTPUT_RATIO_100 |
            DSA_OUTPUT_RATIO_100 |
            DSA_OUTPUT_TO_BOTH |
            DSA_TIMER0 |
            DSA_FIFO_RESET |
            DSB_OUTPUT_RATIO_100 |
            DSB_OUTPUT_TO_BOTH |
            DSB_TIMER1 |
            DSB_FIFO_RESET;

    REG_SOUNDCNT_L = 0;
}

void playSoundA(const unsigned char* sound, int length, int frequency) {


    dma[1].cnt = 0;
    vbCountA = 0;

    int interval = 16777216 / frequency;

    DMANow(1, sound, REG_FIFO_A, DMA_DESTINATION_FIXED | DMA_AT_REFRESH | DMA_REPEAT | DMA_32);

    REG_TM0CNT = 0;

    REG_TM0D = -interval;
    REG_TM0CNT = TIMER_ON;

    soundA.data = sound;
    soundA.length = length;
    soundA.frequency = frequency;
    soundA.duration = ((60 * length) / frequency) - ((length / frequency)*3) - 1;
    soundA.isPlaying = 1;
    soundA.loops = 1;
}

/*
void playSoundB(const unsigned char* sound, int length, int frequency) {
    if (soundB.isPlaying == 1) {
        SOUND tempSound;
        tempSound.data = sound;
        tempSound.length = length;
        tempSound.frequency = frequency;
        tempSound.isPlaying = 0;
        sounds[numSounds] = tempSound;
        numSounds++;
        if (numSounds == 100) {
            numSounds = 0;
        }
    } else if (soundB.isPlaying == 0) {
        playSoundB(sound, length, frequency);
    }
}
*/

void clearBQ() {
    if (soundB.isPlaying == 0) {
        playSoundB(sounds[curSounds].data, sounds[curSounds].length, sounds[curSounds].frequency);

        curSounds++;

        if (curSounds == 100) {
            curSounds = 0;
        }
    }
}

void playSoundB(const unsigned char* sound, int length, int frequency) {

    //dma[2].cnt = 0;
    dma[2].cnt = 0;
    vbCountB = 0;
    int interval = (16777216 / frequency);
    DMANow(2, sound, REG_FIFO_B, DMA_DESTINATION_FIXED | DMA_AT_REFRESH | DMA_32 | DMA_REPEAT);

    REG_TM1CNT = 0;
    REG_TM1D = -interval;
    REG_TM1CNT = TIMER_ON;

    soundB.data = sound;
    soundB.length = length;
    soundB.frequency = frequency;
    soundB.isPlaying = 1;
    soundB.loops = 0;
    soundB.duration = ((length * 60) / frequency)-((length / frequency)*3) - 1;
}

void setupInterrupts() {
    REG_IME = 0;
    REG_INTERRUPT = (unsigned int) interruptHandler;
    // accept interrupts
    REG_IE |= INT_VBLANK;
    // turn on interrupt sending
    REG_DISPSTAT |= INT_VBLANK_ENABLE;

    REG_IME = 1;
}

void interruptHandler() {
    REG_IME = 0;
    if (REG_IF & INT_VBLANK) {
        vbCountA++;
        vbCountB++;

        if (vbCountA == soundA.duration) {
            dma[1].cnt = 0;
            REG_TM0CNT = 0;
            soundB.isPlaying = 0;
            vbCountA = 0;
            playSoundA(soundA.data, soundA.length, soundA.frequency);
        }
        if (vbCountB >= soundB.duration && vbCountB >= 12) {
            dma[2].cnt = 0;
            REG_TM1CNT = 0;
            soundB.isPlaying = 0;
            vbCountB = 0;
        }
        REG_IF = INT_VBLANK;
    }

    REG_IME = 1;
}

void stopSounds() {
    dma[1].cnt = 0;
    dma[2].cnt = 0;
}

void muteSounds() {
    REG_SOUNDCNT_X = 0;
}

void unmuteSounds() {
    REG_SOUNDCNT_X = SND_ENABLED;
}

int main() {
    state = STARTSCREEN;
    int ar;
    for (ar = 0; ar < 10; ar++) {
        arrow newArrow;
        newArrow.x = -50;
        newArrow.y = -50;
        newArrow.gravity = 0;
        newArrow.tempY = 0;
        arrows[ar] = newArrow;
    }
    setupInterrupts();
    setupSounds();

    while (1) {
        switch (state) {
            case STARTSCREEN:
                start(0);
                break;
            case GAMESCREEN:
                game();
                break;
            case PAUSESCREEN:
                pause();
                break;
            case LOSESCREEN:
                lose();
                break;
            case CONTROLS:
                controls();
                break;
        }

    }
    return 0;
}
// Enable sprites
// Check in mylib.h for the correct #define for enabling sprites

void pause() {
    muteSounds();
    REG_DISPCTL = MODE0 | BG0_ENABLE;
    pauseDelay = 30;
    clearText();
    while (1) {
        
        drawString0(5, 5, "PAUSED");
        drawString0(7,5, "PRESS START TO RESUME");
        if (BUTTON_HELD(BUTTON_START)) {
            wait(100);
            clearText();
            state = GAMESCREEN;
            REG_DISPCTL = MODE0 | BG0_ENABLE | BG1_ENABLE | BG2_ENABLE | BG3_ENABLE | SPRITE_ENABLE | SPRITE_MODE_1D;
            unmuteSounds();
            break;
        }
    }
}

void controls() {
    muteSounds();
    REG_DISPCTL = MODE0 | BG0_ENABLE;
    while (1) {
        drawString0(1, 1, "CONTROLS");
        drawString0(3, 1, "A IS SHOOT");
        drawString0(5, 1, "B IS JUMP");
        drawString0(7, 1, "START IS PAUSE");
        drawString0(9, 1, "SELECT IS INVULNERABILITY");
        drawString0(11, 1, "PRESS START TO RETURN TO");
        drawString0(12, 1, "MENU");
        if (BUTTON_HELD(BUTTON_START)) {
            controlDelay = 100;
            wait(100);
            clearText();
            unmuteSounds();
            state = STARTSCREEN;
            start(0);
            break;

        }
    }
}

void win() {
    REG_DISPCTL = MODE0 | BG0_ENABLE;
    clearText();
    stopSounds();
    while (1) {
        groundEnemies[0].x = -100;
        groundEnemies[0].y = -100;
        groundEnemies[1].x = -100;
        groundEnemies[1].y = -100;
        if (level == 1) {
            drawString0(6, 8, "LEVEL 2");
            if (BUTTON_HELD(BUTTON_START)) {
                groundEnemies[0].health = 0;
                groundEnemies[1].health = 0;
                airEnemies[0].health = 0;
                airEnemies[1].health = 0;

                wait(100);
                clearText();
                soundA.isPlaying = 0;
                stopSounds();
                state = STARTSCREEN;
                startT = 0;
                start(1);
                break;
            }
        }
        if (level == 2) {
            drawString0(6, 8, "LEVEL 3");
            if (BUTTON_HELD(BUTTON_START)) {
                groundEnemies[0].health = 0;
                groundEnemies[1].health = 0;
                airEnemies[0].health = 0;
                airEnemies[1].health = 0;
                wait(100);
                clearText();
                soundA.isPlaying = 0;
                stopSounds();
                state = STARTSCREEN;
                startT = 0;
                start(2);
                break;
            }
        }
        if (level == 3) {
            if (victPlayed != 1) {
                drawString0(6, 8, "VICTORY");
                playSoundB(victory, VICTORYLEN, VICTORYFREQ);
                victPlayed = 1;
                
            }
            int ll;
            for (ll = 0; ll < 10; ll++) {
                lasers1[ll].x = -50;
                lasers2[ll].x = -50;
                lasers1[ll].y = -50;
                lasers2[ll].y = -50;
                lasers1[ll].active = 0;
                lasers2[ll].active = 0;
            }
            if (BUTTON_HELD(BUTTON_START)) {
                groundEnemies[0].health = 0;
                groundEnemies[1].health = 0;
                airEnemies[0].health = 0;
                airEnemies[1].health = 0;
                wait(100);
                clearText();
                soundA.isPlaying = 0;
                stopSounds();
                state = STARTSCREEN;
                startT = 0;
                level = 0;
                start(0);
                
                break;
            }
        }
    }
}

void lose() {
    REG_DISPCTL = MODE0 | BG0_ENABLE;
    clearText();
    stopSounds();
    while (1) {
        drawString0(6, 8, "YOU LOSE");
        if (BUTTON_HELD(BUTTON_START)) {
            groundEnemies[0].health = 0;
            groundEnemies[1].health = 0;
            groundEnemies[0].x = -100;
            groundEnemies[0].y = -100;
            groundEnemies[1].x = -100;
            groundEnemies[1].y = -100;
            airEnemies[0].health = 0;
            airEnemies[1].health = 0;
            int ll;
            for (ll = 0; ll < 10; ll++) {
                lasers1[ll].x = -50;
                lasers2[ll].x = -50;
                lasers1[ll].y = -50;
                lasers2[ll].y = -50;
                lasers1[ll].active = 0;
                lasers2[ll].active = 0;
            }
            wait(100);
            clearText();
            soundA.isPlaying = 0;
            state = STARTSCREEN;
            startT = 0;
            level = 0;
            start(0);
            break;
        }
    }
}

void start(int lvl) {
    if (lvl == 0) {
        DMANow(3, (unsigned int*) splashTiles, &CHARBLOCKBASE[3], splashTilesLen / 2);
        DMANow(3, (unsigned int*) splashMap, &SCREENBLOCKBASE[16], splashMapLen / 2);
        DMANow(3, (unsigned int*) lettersTiles, &CHARBLOCKBASE[1], lettersTilesLen / 2);
        DMANow(3, (unsigned int*) lettersMap, &SCREENBLOCKBASE[19], lettersMapLen / 2);

        loadPalette(splashPal);
        REG_DISPCTL = MODE0 | BG0_ENABLE | BG1_ENABLE;
        REG_BG0CNT = BG_SIZE1 | SBB(17) | CBB(1); //text and ui
        REG_BG1CNT = BG_SIZE0 | SBB(16) | CBB(3); //splash

        drawString0(6, 10, "WELCOME");
        drawString0(8, 4, "PRESS ENTER TO START");
        drawString0(10, 2, "PRESS SELECT FOR CONTROLS");
        unmuteSounds();
        playSoundA(menuMusic, MENUMUSICLEN, MENUMUSICFREQ);
    }

    realHOff = 0;
    hOff2 = 0;
    vOff2 = 0;
    tempOff = 0;
    hOff3 = 0;
    vOff3 = 0;
    time = 0;
    day = 0;
    side = 0;
    started = 0;
    flip = 0;
    atkDelay = 0;
    changeAttack = 0;
    tempVar = 0;
    leftBound = 100;
    startT = 0;
    pauseDelay = 0;
    traveled = 0;
    charWidth = 16;
    charHeight = 32;
    charVelY = 0;
    hp = 10;
    enemy1Delay = 0;
    hpLossDelay = 0;
    currentPalette = 0;
    currentSpritePalette = 0;
    invuln = 0;
    groundCorpses[0].frame = 5;
    groundCorpses[1].frame = 5;
    REG_BG1HOFS = hOff3;
    REG_BG1VOFS = vOff3;
    controlDelay = 0;
    stopFix = 0;
    projDelay = 0;
    enemy2Delay = 0;
    corpseDelay1 = 0;
    corpseDelay2 = 0;
    enemyFrameCount = 0;
    projNum = 0;
    numSounds = 0;
    curSounds = 0;
    crossed = 0;
    level = lvl + 1;
    enemyAir t1;
    t1.x = -100;
    t1.y = -100;
    t1.health = 0;
    enemyGround t3;
    t3.x = -50;
    t3.y = -50;
    t3.health = 0;
    t3.frame = 0;
    groundEnemies[0] = t3;
    groundEnemies[1] = t3;
    airEnemies[0] = t1;
    airEnemies[1] = t1;
    expDelay = 0;
    airCorpseDelay1 = 0;
    airCorpseDelay2 = 0;
    explosion = 0;
    bossDeathDelay = 0;
    initialDrop = 0;
    pq = 0;
    victPlayed = 0;

    projectile t2;
    t2.y = -100;
    t2.x = -100;
    t2.active = 0;
    int rq;
    for (rq = 0; rq < 4; rq++) {
        projectiles[rq] = t2;
    }


    hOff = 0;
    vOff = 0;
    moveCounter = 0;
    frame = 0;

    charRow = 100;
    charCol = 104;
    while (1) {
/* COLOR CHANGING START SCREEN
                bgwait++;
                if (bgwait % 20000 == 19999) {
                    bgCurrentPalette++;
                }
                if (bgCurrentPalette == 4) {
                    bgCurrentPalette = 0;
                }
                swapPalette(splashPal, bgCurrentPalette);
*/
        if (level >= 2) {
            state = GAMESCREEN;
            unmuteSounds();
            game();
            break;
        }
        if (BUTTON_HELD(BUTTON_START)) {
            REG_DISPCTL = MODE0;

            clearText();
            changeMap(level);
            state = GAMESCREEN;
            stopSounds();
            unmuteSounds();
            game();
            break;
        }
        if (BUTTON_HELD(BUTTON_SELECT)) {
            clearText();
            state = CONTROLS;
            controls();
        }
    }
}

void game() {
    if (startT == 0) {
        clearText();
        if (level == 1) {
            loadPalette(tilesPal);
            DMANow(3, (unsigned int*) tilesTiles, &CHARBLOCKBASE[0], tilesTilesLen / 2);
            drawSun();

            DMANow(3, (unsigned int*) lettersTiles, &CHARBLOCKBASE[1], lettersTilesLen / 2);
            DMANow(3, (unsigned int*) lettersMap, &SCREENBLOCKBASE[19], lettersMapLen / 2);
        }
        if (level == 2) {
            loadPalette(tiles2Pal);
            DMANow(3, (unsigned int*) tiles2Tiles, &CHARBLOCKBASE[0], tiles2TilesLen / 2);
            drawSun();
            DMANow(3, (unsigned int*) lettersTiles, &CHARBLOCKBASE[1], lettersTilesLen / 2);
            DMANow(3, (unsigned int*) lettersMap, &SCREENBLOCKBASE[19], lettersMapLen / 2);
        }
        if (level == 3) {
            DMANow(3, (unsigned int*) tiles3Tiles, &CHARBLOCKBASE[0], tiles3TilesLen / 2);
            DMANow(3, (unsigned int*) lettersTiles, &CHARBLOCKBASE[1], lettersTilesLen / 2);
            DMANow(3, (unsigned int*) lettersMap, &SCREENBLOCKBASE[19], lettersMapLen / 2);
        }


        REG_BG3CNT = BG_SIZE3 | SBB(23) | CBB(0); //sky and clouds

        REG_BG2CNT = BG_SIZE3 | SBB(28) | CBB(0); //trees

        REG_BG1CNT = BG_SIZE0 | SBB(19) | CBB(0); //sun

        REG_BG0CNT = BG_SIZE0 | SBB(17) | CBB(1); //TEXT,UI

        changeMap(level);
        if (level != 3) {
            drawSun();
        }
        REG_DISPCTL = MODE0;
        startT++;
    }
    if (level == 1) {
        DMANow(3, s1Tiles, &CHARBLOCKBASE[4], s1TilesLen / 2);

        DMANow(3, s1Pal, SPRITE_PALETTE, 256);
    } else if (level == 2) {
        DMANow(3, s2Tiles, &CHARBLOCKBASE[4], s1TilesLen / 2);

        DMANow(3, s2Pal, SPRITE_PALETTE, 256);

    } else if (level == 3) {
        DMANow(3, s3Tiles, &CHARBLOCKBASE[4], s3TilesLen / 2);
        DMANow(3, s3Pal, SPRITE_PALETTE, 256);
        boss.x = 156;
        boss.y = -64;
        boss.dir = 0; //xdirection
        boss.health = 1000;
        boss.frame = 1; //ydirection
    }


    shadowOAM[0].attr0 = (ROWMASK & charRow) | ATTR0_4BPP | ATTR0_TALL;
    shadowOAM[0].attr1 = (COLMASK & charCol) | ATTR1_SIZE32;
    shadowOAM[0].attr2 = 1;

    buttons = BUTTONS;

    REG_DISPCTL = MODE0 | BG0_ENABLE | BG1_ENABLE | BG2_ENABLE | BG3_ENABLE | SPRITE_ENABLE | SPRITE_MODE_1D;

    int charState = RIGHT;
    int prevCharState = RIGHT;
    int q;
    for (q = 1; q < 128; q++) {
        shadowOAM[q].attr0 = ATTR0_HIDE;
    }
    DMANow(3, shadowOAM, OAM, 512);

    if (level == 1) {
        swapPalette(tilesPal, 0);
        playSoundA(bgMusic, BGMUSICLEN, BGMUSICFREQ);
    }
    if (level == 2) {
        swapPalette(tiles2Pal, 0);
        playSoundA(bgMusic2, BGMUSIC2LEN, BGMUSIC2FREQ);
    }
    if (level == 3) {
        swapPalette(tiles3Pal, 0);
        playSoundA(bossMusic, BOSSMUSICLEN, BOSSMUSICFREQ);
    }
    while (1) {
        if (traveled == 6 && level != 3) {
            state = WINSCREEN;
            win();
        }
        if (hp <= 0) {
            state = LOSESCREEN;
            lose();
        }

        displayHealth(hp);
        enemy1Delay++;
        expDelay++;
        hpLossDelay--;
        projDelay++;
        enemy2Delay++;
        enemyFrameCount++;
        upDelay++;
        if (level == 1) {
            if (groundCorpses[0].frame <= 2) {
                corpseDelay1++;
            }
            if (groundCorpses[1].frame <= 2) {
                corpseDelay2++;
            }
        }
        if (level == 2) {
            if (groundEnemies[0].frame <= 11 && groundEnemies[0].frame >= 10) {
                corpseDelay1++;
            }

            if (groundEnemies[1].frame <= 11 && groundEnemies[1].frame >= 10) {
                corpseDelay2++;
            }
            if (airEnemies[0].frame <= 7 && airEnemies[0].frame >= 5) {
                airCorpseDelay1++;
            }
            if (airEnemies[1].frame <= 7 && airEnemies[1].frame >= 5) {
                airCorpseDelay2++;
            }
        }
        if (expDelay == 19) {
            shadowOAM[21].attr0 = ATTR0_HIDE;
        }
        if (hpLossDelay == 19) {
            if (level == 1) {
                swapPalette(tilesPal, 4);
            }
            if (level == 2) {
                swapPalette(tiles2Pal, 4);
            }
            if (level == 3) {
                swapPalette(tiles3Pal, 4);
            }
        }
        if (hpLossDelay == 0) {
            if (level == 1) {
                swapPalette(tilesPal, currentPalette);
            }
            if (level == 2) {
                swapPalette(tiles2Pal, currentPalette);
            }
            if (level == 3) {
                swapPalette(tiles3Pal, 0);
            }
        }
        if (enemy1Delay >= 250) {
            if (rand() % 5 == 1) {
                spawn1Enemy1(level);
                enemy1Delay = 0;
            } else if (rand() % 15 == 1) {
                spawn1Enemy2(level);
                enemy1Delay = 0;
            }
        }
        if (enemy2Delay >= 150) {
            if (rand() % 5 == 1) {
                spawnAir1(level);
                enemy2Delay = 0;
            } else if (rand() % 5 == 1) {
                spawnAir2(level);
                enemy2Delay = 0;
            }
        }


        if (groundEnemies[0].health > 0) {
            groundEnemies[0].x -= groundEnemies[0].dir * 2;
        }

        //works dino L->R
        if (groundEnemies[1].health > 0) {
            groundEnemies[1].x -= groundEnemies[1].dir * 2;
        }
        if (groundEnemies[1].x >= 272) {
            groundEnemies[1].health = 0;
        }
        if (groundEnemies[0].x <= -32) {
            groundEnemies[0].health = 0;
        }

        if (airEnemies[0].health > 0) {
            airEnemies[0].x -= airEnemies[0].dir * 2;
        }
        if (airEnemies[1].health > 0) {
            airEnemies[1].x -= airEnemies[1].dir * 2;
        }
        if (level == 1) {
            if (airEnemies[1].x > 272) {
                airEnemies[1].health = 0;
            }
            if (airEnemies[0].x < -64) {
                airEnemies[0].health = 0;
            }
        }
        if (level == 2) {
            if (airEnemies[1].x >= 272) {
                airEnemies[1].health = 0;
                airEnemies[1].x = -50;
                airEnemies[1].y = -50;
            }
            if (airEnemies[0].x <= -64) {
                airEnemies[0].health = 0;
                airEnemies[0].x = -50;
                airEnemies[0].y = -50;
            }
        }


        int arrws;
        for (arrws = 0; arrws < 10; arrws++) {
            if ((arrows[arrws].x >= 240 || (arrows[arrws].x <= 0)) && arrows[arrws].x != -100) {
                arrows[arrws].x = -100;
                arrows[arrws].y = -100;
            }
        }
        time += 4;

        pauseDelay--;
        atkDelay--;
        if (charRow < 100) {
            charVelY -= 1;
        }
        if (charVelY > 0 || charVelY < 0) {
            charRow -= charVelY / 8;
            if (charRow >= 100) {
                charRow = 100;
            }
        } else {
            charVelY = 0;
        }

        if (moveCounter % 4 == 0) {

            // change the frame here
            // remember that there are only 3 frames 
            if ((charState != JUMP) && (charState != ATTACKJUMP) && (charState != ATTACK)) {
                if (frame <= 3) frame++;
                else if (frame >= 4) frame = 1;
            } else if (charState == ATTACK || charState == ATTACKJUMP) {
                if (changeAttack == 1) {
                    frame = 6;
                    changeAttack = 0;
                    charState = ATTACK;
                } else if (changeAttack == 2) {
                    frame = 9;
                    changeAttack = 0;
                    charState = JUMP;
                }
            } else if (charState == JUMP) {
                frame = 8;
            }


        }
        if (groundEnemies[0].health > 0) {
            if (enemyFrameCount % 10 == 0) {
                groundEnemies[0].frame++;
            }
            if (level == 1) {
                if (groundEnemies[0].frame == 3) groundEnemies[0].frame = 0;
            } else if (level == 2) {
                if (groundEnemies[0].frame == 6) groundEnemies[0].frame = 0;
                if (groundEnemies[0].frame == 10)groundEnemies[0].frame = 0;
            }
        }
        if (groundEnemies[1].health > 0) {
            if (enemyFrameCount % 10 == 0) {
                groundEnemies[1].frame++;
            }
            if (level == 1) {
                if (groundEnemies[1].frame == 3) groundEnemies[1].frame = 0;
            } else if (level == 2) {
                if (groundEnemies[1].frame == 6) groundEnemies[1].frame = 0;
                if (groundEnemies[1].frame == 10) groundEnemies[1].frame = 0;
            }
        }
        if (airEnemies[0].health > 0) {
            if (enemyFrameCount % 10 == 0) {
                airEnemies[0].frame++;
            }
            if (level == 1) {
                if (airEnemies[0].frame == 10) airEnemies[0].frame = 0;
            }
            if (level == 2) {
                if (airEnemies[0].frame == 2) airEnemies[0].frame = 0;
                if (airEnemies[0].frame == 5) airEnemies[0].frame = 0;
            }
        }
        if (airEnemies[1].health > 0) {
            if (enemyFrameCount % 10 == 0) {
                airEnemies[1].frame++;
            }
            if (level == 1) {
                if (airEnemies[1].frame == 10) airEnemies[1].frame = 0;
            }
            if (level == 2) {
                if (airEnemies[1].frame == 2) airEnemies[1].frame = 0;
                if (airEnemies[1].frame == 5) airEnemies[1].frame = 0;
            }
        }
        if (charState != IDLE) {
            prevCharState = charState;
        }
        if ((charRow == 100 && charState != ATTACK) || (charRow == 100 && tempVar == 10)) {
            charState = IDLE;
            tempVar = 0;
        }
        if (frame == 6 && charState == ATTACK) {
            tempVar++;
        }


        if (BUTTON_HELD(BUTTON_UP)) {
        }
        if (BUTTON_HELD(BUTTON_DOWN)) {
        }
        if (BUTTON_HELD(BUTTON_SELECT)) {

        }

        if (BUTTON_HELD(BUTTON_LEFT)) {
            if (charState != JUMP && charState != ATTACK && charState != ATTACKJUMP) {
                charState = LEFT;
            }

            flip = 1;
            if (leftBound == 100) {
                leftBound--;
            } else if (leftBound <= 99 && leftBound >= 0) {
                leftBound--;
                charCol--;

            } else {
                /*
                                hOff--;
                                realHOff--;
                                tempOff--;
                 */
            }
        }
        if (BUTTON_HELD(BUTTON_RIGHT)) {

            if (charState != JUMP && charState != ATTACK && charState != ATTACKJUMP) {
                charState = RIGHT;
            }
            flip = 0;
            if (leftBound <= 99) {
                leftBound++;
                charCol++;
            } else {
                hOff++;
                realHOff++;
                tempOff++;
            }
        }
        //FIRE
        if (BUTTON_HELD(BUTTON_A)) {
            if (atkDelay <= 0) {
                if ((charState == JUMP || charState == ATTACKJUMP)) {
                    charState = ATTACKJUMP;
                    changeAttack = 2;
                    frame = 7;
                    
                    fire(charCol, charRow, arrowCount);
                    arrowCount++;
                    playSoundB(arrowSound, ARROWSOUNDLEN, ARROWSOUNDFREQ);
                    atkDelay = 30;
                } else if (charState != ATTACK) {
                    frame = 5;
                    changeAttack = 1;
                    charState = ATTACK;
                    fire(charCol, charRow, arrowCount);
                    arrowCount++;
                    playSoundB(arrowSound, ARROWSOUNDLEN, ARROWSOUNDFREQ);
                    atkDelay = 30;
                }
            }
        }
        //JUMP
        if (BUTTON_HELD(BUTTON_B)) {
            if (charRow == 100) {
                charState = JUMP;
                frame = 8;
                charVelY += 42;
            }
        }

        if (BUTTON_HELD(BUTTON_START)) {
            if (pauseDelay <= 0 && time >= 100) {
                wait(50);
                state = PAUSESCREEN;
                pause();
            }
        }
        if (BUTTON_HELD(BUTTON_SELECT)) {

            wait(50);
            if (invuln == 0) invuln = 1;

            else {
                invuln = 0;
                //currentSpritePalette = 0;
                //swapSpritePalette(s1Pal, currentSpritePalette);
            }

        }
        if (tempOff >= 10) {
            hOff2++;
            tempOff = -10;
        } else if (tempOff <= -10) {
            hOff2--;
            tempOff = 10;
        }
        if (time % 10 == 0) {
            hOff3++;
        }
        if (time % 100 == 0) {
            if (time <= 2300) {
                vOff3++;
            } else if (time <= 4800) {
                vOff3--;
            }
        }
        if (time % 500 == 0) {
            hOff2++;
        }
        if (time == 3800 && day == 0) {
            if (level == 1) {
                swapPalette(tilesPal, 1);
            }
            if (level == 2) {
                swapPalette(tiles2Pal, 1);
            }
            currentPalette = 1;
            //ORANGE SKY
        }
        if (time == 600 && day == 0) {
            if (level == 1) {
                swapPalette(tilesPal, 0);
            }
            if (level == 2) {
                swapPalette(tiles2Pal, 0);
            }
            currentPalette = 0;
            //dawn
        }
        if (time == 4900 && day == 0) {
            if (level == 1) {
                swapPalette(tilesPal, 2);
            }
            if (level == 2) {
                swapPalette(tiles2Pal, 2);
            }
            currentPalette = 2;
            vOff3 = 0;
            hOff3 = 0;
            time = 0;
            day = 1;
            //NIGHT SKY
        }
        if (time == 4900 && day == 1) {
            if (level == 1) {
                swapPalette(tilesPal, 3);
            }
            if (level == 2) {
                swapPalette(tiles2Pal, 3);
            }
            currentPalette = 3;
            vOff3 = 0;
            hOff3 = 0;
            time = 0;
            day = 0;
        }
        if (charState == IDLE) {
            frame = 0;
        } else {
            moveCounter++;
        }
        // Update the shadowOAM for 
        shadowOAM[0].attr0 = (ROWMASK & charRow) | ATTR0_4BPP | ATTR0_TALL;
        if (flip == 0) {
            shadowOAM[0].attr1 = (COLMASK & charCol) | ATTR1_SIZE32;
        }
        if (flip == 1) {
            shadowOAM[0].attr1 = (COLMASK & charCol) | ATTR1_SIZE32 | ATTR1_HFLIP;
        }
        shadowOAM[0].attr2 = 8 * frame;
        if (arrowCount == 10) {
            arrowCount = 0;
        }


        checkCollisionTwo();
        checkCollisionThree();
        checkCollisionOne();
        if (level == 3 && boss.health > 0) {
            laserCol();
            bossCol();
        }
        if ((curSounds < numSounds) || (numSounds == 0 && curSounds == 99)) {
            //clearBQ();
        }



        int rq = 0;
        while (rq < 10) {
            if (arrows[rq].tempY == 1) {
                arrows[rq].x += 4 * arrows[rq].dir;
                if (arrows[rq].x % 6 == 4) {
                    arrows[rq].gravity += 1;
                }
                if (arrows[rq].gravity == 2) {
                    arrows[rq].y = arrows[rq].y += 2;
                    arrows[rq].gravity = 0;
                }
                if (arrows[rq].dir == -1) {
                    shadowOAM[rq + 1].attr1 = (COLMASK & (arrows[rq].x) - 14) | ATTR1_SIZE8 | ATTR1_HFLIP;
                    shadowOAM[rq + 1].attr0 = (ROWMASK & (arrows[rq].y) + 4) | ATTR0_4BPP | ATTR0_WIDE;
                } else {
                    shadowOAM[rq + 1].attr1 = (COLMASK & (arrows[rq].x) + 0) | ATTR1_SIZE8;
                    shadowOAM[rq + 1].attr0 = (ROWMASK & (arrows[rq].y) + 4) | ATTR0_4BPP | ATTR0_WIDE;
                }
                shadowOAM[rq + 1].attr2 = 80;
            }
            if (arrows[rq].x >= 240 || arrows[rq].x <= -20) {
                shadowOAM[rq + 1].attr0 = ATTR0_HIDE;
                arrows[rq].tempY = 0;
            }
            rq++;

        }
        if (invuln == 1) {
            drawString0(2, 0, "INVINCIBLE");
        }
        if (level == 1) {
            if (groundEnemies[0].health > 0) {
                shadowOAM[11].attr0 = (ROWMASK & groundEnemies[0].y) | ATTR0_4BPP | ATTR0_TALL;
                shadowOAM[11].attr1 = (COLMASK & groundEnemies[0].x) | ATTR1_SIZE64 | ATTR1_HFLIP;
                shadowOAM[11].attr2 = 82 + (groundEnemies[0].frame * (32));
            } else {
                shadowOAM[11].attr0 = ATTR0_HIDE;
            }
            if (groundEnemies[1].health > 0) {
                shadowOAM[12].attr0 = (ROWMASK & groundEnemies[1].y) | ATTR0_4BPP | ATTR0_TALL;
                shadowOAM[12].attr1 = (COLMASK & groundEnemies[1].x) | ATTR1_SIZE64;
                shadowOAM[12].attr2 = 82 + (groundEnemies[1].frame * (32));
            } else {
                shadowOAM[12].attr0 = ATTR0_HIDE;
            }
            if (groundCorpses[0].frame <= 2) {

                shadowOAM[13].attr0 = (ROWMASK & groundCorpses[0].y) | ATTR0_4BPP | ATTR0_WIDE;
                shadowOAM[13].attr1 = (COLMASK & groundCorpses[0].x + (groundCorpses[0].dir) - realHOff) | ATTR1_SIZE64 | ATTR1_HFLIP;
                if (corpseDelay1 >= 50) {
                    groundCorpses[0].frame = 3;
                } else if (corpseDelay1 >= 30) {
                    groundCorpses[0].frame = 2;
                } else if (corpseDelay1 >= 15) {
                    groundCorpses[0].frame = 1;
                }
                shadowOAM[13].attr2 = 210 + (groundCorpses[0].frame * (32));

            } else {
                shadowOAM[13].attr0 = ATTR0_HIDE;
                corpseDelay1 = 0;
            }
            if (groundCorpses[1].frame <= 2) {
                shadowOAM[14].attr0 = (ROWMASK & groundCorpses[1].y) | ATTR0_4BPP | ATTR0_WIDE;

                /*
                                if (groundCorpses[1].dir - realHOff <= 0) {
                                    groundCorpses[1].dir += 20;
                                    fix = 0;
                                }
                 */
                shadowOAM[14].attr1 = (COLMASK & groundCorpses[1].x + (groundCorpses[1].dir) - realHOff) | ATTR1_SIZE64;
                //shadowOAM[13].attr1 = (COLMASK & raptorCorpses[1].x - (realHOff - raptorCorpses[1].dir)) | ATTR1_SIZE64 | ATTR1_HFLIP;
                if (corpseDelay2 >= 50) {
                    groundCorpses[1].frame = 3;
                } else if (corpseDelay2 >= 30) {
                    groundCorpses[1].frame = 2;
                } else if (corpseDelay2 >= 15) {
                    groundCorpses[1].frame = 1;
                }
                shadowOAM[14].attr2 = 210 + (groundCorpses[1].frame * (32));

            } else {
                shadowOAM[14].attr0 = ATTR0_HIDE;
                corpseDelay2 = 0;
            }

            if (airEnemies[0].health > 0) {
                if (rand() % 40 == 0 && time > 100) {
                    shootProjectile(airEnemies[0]);
                }
                shadowOAM[15].attr0 = (ROWMASK & airEnemies[0].y) | ATTR0_4BPP | ATTR0_WIDE;
                shadowOAM[15].attr1 = (COLMASK & airEnemies[0].x) | ATTR1_SIZE64 | ATTR1_HFLIP;
                shadowOAM[15].attr2 = 306 + (airFrames[airEnemies[0].frame] * (32));

            } else if (airEnemies[0].y == 112) {
                airEnemies[0].x = -100;
                airEnemies[0].y = -100;
                shadowOAM[15].attr0 = ATTR0_HIDE;
            } else if (airEnemies[0].health <= 0 && airEnemies[0].x >= -64 && airEnemies[0].x <= 272) {
                airEnemies[0].y++;
                shadowOAM[15].attr0 = (ROWMASK & airEnemies[0].y) | ATTR0_4BPP | ATTR0_WIDE;
                shadowOAM[15].attr1 = (COLMASK & airEnemies[0].x + (airEnemies[0].dir) - realHOff) | ATTR1_SIZE64;
                shadowOAM[15].attr2 = 306 + (6 * (32));
            }

            if (airEnemies[1].health > 0) {
                if (projDelay % 40 == 0 && time > 100) {
                    shootProjectile(airEnemies[1]);
                }
                shadowOAM[16].attr0 = (ROWMASK & airEnemies[1].y) | ATTR0_4BPP | ATTR0_WIDE;
                shadowOAM[16].attr1 = (COLMASK & airEnemies[1].x) | ATTR1_SIZE64;
                shadowOAM[16].attr2 = 306 + (airFrames[airEnemies[1].frame] * (32));
            } else if (airEnemies[1].y == 112) {
                airEnemies[1].x = -100;
                airEnemies[1].y = -100;
                stopFix = 0;
                shadowOAM[16].attr0 = ATTR0_HIDE;
            } else if (airEnemies[1].health <= 0 && airEnemies[1].x >= -64 && airEnemies[1].x <= 240) {
                airEnemies[1].y++;
                if (airEnemies[1].dir - realHOff <= 0 && stopFix == 0) {
                    airEnemies[1].dir += 20;
                    stopFix = 1;
                }
                shadowOAM[16].attr0 = (ROWMASK & airEnemies[1].y) | ATTR0_4BPP | ATTR0_WIDE;
                shadowOAM[16].attr1 = (COLMASK & airEnemies[1].x + (airEnemies[1].dir) - realHOff) | ATTR1_SIZE64;
                shadowOAM[16].attr2 = 306 + (6 * (32));
            }
            int bb;
            for (bb = 0; bb < 4; bb++) {
                if (level == 1) {
                    if (projectiles[bb].active == 1) {
                        projectiles[bb].y += 2;
                        shadowOAM[17 + bb].attr0 = (ROWMASK & projectiles[bb].y) | ATTR0_4BPP | ATTR0_WIDE;
                        shadowOAM[17 + bb].attr1 = (COLMASK & projectiles[bb].x + ((projectiles[bb].dir - realHOff))) | ATTR1_SIZE64;
                        shadowOAM[17 + bb].attr2 = 530;
                        if (projectiles[bb].y >= 120) {
                            projectiles[bb].active = 0;
                            projectiles[bb].x = -50;
                            projectiles[bb].y = -50;
                            shadowOAM[17 + bb].attr0 = ATTR0_HIDE;
                        }
                    } else {
                        shadowOAM[17 + bb].attr0 = ATTR0_HIDE;
                    }
                }
            }
        } else if (level == 2) {
            if (groundEnemies[0].health > 0) {
                shadowOAM[11].attr0 = (ROWMASK & groundEnemies[0].y) | ATTR0_4BPP | ATTR0_SQUARE;
                shadowOAM[11].attr1 = (COLMASK & groundEnemies[0].x) | ATTR1_SIZE32;
                shadowOAM[11].attr2 = 82 + (groundEnemies[0].frame * (16));
            } else {
                shadowOAM[11].attr0 = ATTR0_HIDE;
            }
            if (groundEnemies[1].health > 0) {
                shadowOAM[12].attr0 = (ROWMASK & groundEnemies[1].y) | ATTR0_4BPP | ATTR0_SQUARE;
                shadowOAM[12].attr1 = (COLMASK & groundEnemies[1].x) | ATTR1_SIZE32 | ATTR1_HFLIP;
                shadowOAM[12].attr2 = 82 + (groundEnemies[1].frame * (16));
            } else {
                shadowOAM[12].attr0 = ATTR0_HIDE;
            }
            if (groundEnemies[0].frame >= 10 && groundEnemies[0].frame <= 12 && groundEnemies[0].health <= 0 && groundEnemies[0].x >= -32 && groundEnemies[0].x <= 272) {
                shadowOAM[11].attr0 = (ROWMASK & groundEnemies[0].y) | ATTR0_4BPP | ATTR0_SQUARE;
                shadowOAM[11].attr1 = (COLMASK & groundEnemies[0].x + (groundEnemies[0].dir) - realHOff) | ATTR1_SIZE32;

                if (corpseDelay1 >= 50) {
                    corpseDelay1 = 0;
                    shadowOAM[11].attr0 = ATTR0_HIDE;
                    groundEnemies[0].x = -50;
                    groundEnemies[0].y = -50;
                } else if (corpseDelay1 >= 25) {
                    groundEnemies[0].frame = 11;
                }
                shadowOAM[11].attr2 = 258 + ((groundEnemies[0].frame - 10) * (16));

            }
            if (groundEnemies[0].y <= -50) {
                shadowOAM[13].attr0 = ATTR0_HIDE;
            }
            if (groundEnemies[1].frame >= 10 && groundEnemies[1].frame <= 12 && groundEnemies[1].health <= 0 && groundEnemies[1].x >= -32 && groundEnemies[1].x <= 272) {
                shadowOAM[12].attr0 = (ROWMASK & groundEnemies[1].y) | ATTR0_4BPP | ATTR0_SQUARE;
                shadowOAM[12].attr1 = (COLMASK & groundEnemies[1].x + (groundEnemies[1].dir) - realHOff) | ATTR1_SIZE32;
                if (corpseDelay2 >= 50) {
                    shadowOAM[12].attr0 = ATTR0_HIDE;
                    corpseDelay2 = 0;
                    groundEnemies[1].x = -50;
                    groundEnemies[1].y = -50;
                } else if (corpseDelay2 >= 25) {
                    groundEnemies[1].frame = 11;
                }
                shadowOAM[12].attr2 = 258 + ((groundEnemies[1].frame - 10) * (16));

            }
            if (groundEnemies[1].y <= -50) {
                shadowOAM[14].attr0 = ATTR0_HIDE;
            }

            if (airEnemies[0].health > 0) {
                if (projDelay % 90 == 89 && time > 100) {
                    shootProjectile(airEnemies[0]);
                    if (level == 2) {
                        airEnemies[0].frame = 2;
                    }
                }
                shadowOAM[15].attr0 = (ROWMASK & airEnemies[0].y) | ATTR0_4BPP | ATTR0_SQUARE;
                shadowOAM[15].attr1 = (COLMASK & airEnemies[0].x) | ATTR1_SIZE32;
                shadowOAM[15].attr2 = 306 + (airEnemies[0].frame * (16));

            } else if (airEnemies[0].health <= 0 && airEnemies[0].x >= -32 && airEnemies[0].x <= 272 && airEnemies[0].frame <= 7) {

                if (airCorpseDelay1 >= 60) {
                    shadowOAM[15].attr0 = ATTR0_HIDE;
                    airCorpseDelay1 = 0;
                    airEnemies[0].x = -50;
                    airEnemies[0].y = -50;

                } else if (airCorpseDelay1 >= 40) {
                    airEnemies[0].frame = 7;
                } else if (airCorpseDelay1 >= 20) {
                    airEnemies[0].frame = 6;
                }
                shadowOAM[15].attr1 = (COLMASK & (airEnemies[0].x + airEnemies[0].dir - realHOff)) | ATTR1_SIZE32;
                shadowOAM[15].attr0 = (ROWMASK & airEnemies[0].y) | ATTR0_4BPP | ATTR0_SQUARE;
                shadowOAM[15].attr2 = 306 + ((airEnemies[0].frame) * (16));
            }

            if (airEnemies[1].health > 0) {
                if (projDelay % 90 == 89 && time > 100) {
                    shootProjectile(airEnemies[1]);
                    if (level == 2) {
                        airEnemies[1].frame = 2;
                    }
                }
                shadowOAM[16].attr0 = (ROWMASK & airEnemies[1].y) | ATTR0_4BPP | ATTR0_SQUARE;
                shadowOAM[16].attr1 = (COLMASK & airEnemies[1].x) | ATTR1_SIZE32 | ATTR1_HFLIP;
                shadowOAM[16].attr2 = 306 + (airEnemies[1].frame * (16));
            } else if (airEnemies[1].health <= 0 && airEnemies[1].x >= -32 && airEnemies[1].x <= 272 && airEnemies[1].frame <= 7) {
                if (airCorpseDelay2 >= 60) {
                    shadowOAM[16].attr0 = ATTR0_HIDE;
                    airCorpseDelay2 = 0;
                    airEnemies[1].x = -50;
                    airEnemies[1].y = -50;

                } else if (airCorpseDelay2 >= 40) {
                    airEnemies[1].frame = 7;
                } else if (airCorpseDelay2 >= 20) {
                    airEnemies[1].frame = 6;
                }
                shadowOAM[16].attr1 = (COLMASK & (airEnemies[1].x + airEnemies[1].dir - realHOff)) | ATTR1_SIZE32 | ATTR1_HFLIP;
                shadowOAM[16].attr0 = (ROWMASK & airEnemies[1].y) | ATTR0_4BPP | ATTR0_SQUARE;
                shadowOAM[16].attr2 = 306 + ((airEnemies[1].frame) * (16));
            } else if (airEnemies[1].health <= 0) {
                airEnemies[1].x = -50;
                airEnemies[1].y = -50;
                stopFix = 0;
                shadowOAM[16].attr0 = ATTR0_HIDE;
            }
            /*
                         else if (airEnemies[1].health <= 0 && airEnemies[1].x >= -64 && airEnemies[1].x <= 240) {
                            airEnemies[1].y++;
                            if (airEnemies[1].dir - realHOff <= 0 && stopFix == 0) {
                                airEnemies[1].dir += 20;
                                stopFix = 1;
                            }
                            shadowOAM[16].attr0 = (ROWMASK & airEnemies[1].y) | ATTR0_4BPP | ATTR0_WIDE;
                            shadowOAM[16].attr1 = (COLMASK & airEnemies[1].x + (airEnemies[1].dir) - realHOff) | ATTR1_SIZE64;
                            shadowOAM[16].attr2 = 306 + (6 * (32));
                        }
             */
            int bb;
            for (bb = 0; bb < 4; bb++) {
                if (projectiles[bb].active == 1) {
                    projectiles[bb].x -= projectiles[bb].dir * 4;
                    shadowOAM[17 + bb].attr0 = (ROWMASK & projectiles[bb].y) | ATTR0_4BPP | ATTR0_SQUARE;
                    shadowOAM[17 + bb].attr1 = (COLMASK & projectiles[bb].x) | ATTR1_SIZE8;
                    shadowOAM[17 + bb].attr2 = 434;
                    if (projectiles[bb].x >= 272 || projectiles[bb].x <= -32) {

                        projectiles[bb].active = 0;
                        projectiles[bb].x = -50;
                        projectiles[bb].y = -50;
                        shadowOAM[17 + bb].attr0 = ATTR0_HIDE;
                    }
                } else {
                    shadowOAM[17 + bb].attr0 = ATTR0_HIDE;
                }
            }
        } else if (level == 3) {
            initialDrop++;
            if (boss.health > 0) {
                int rq;
                for (rq = 0; rq < 30; rq++) {
                    shadowOAM[rq + 31].attr0 = ATTR0_HIDE;
                }
                if (initialDrop <= 200) {
                    boss.frame = 1;
                } else {
                    if (boss.y + 80 >= charRow && boss.y + 70 <= charRow) {
                        boss.frame = 0;
                    } else if (boss.y <= charRow && boss.y + 70 >= charRow) {
                        boss.frame = -1;
                    } else if (boss.y + 64 <= 100) { //( boss.y + 64 >= charRow) {
                        boss.frame = 1;
                    }
                }
                if (boss.x <= charCol + 74 && boss.x >= charCol + 64) {
                    boss.dir = 0;
                } else if (boss.x <= charCol + 64) {
                    boss.dir = 1;
                } else {
                    boss.dir = -1;
                }
                if (projDelay % 125 == 124) {
                    shootl1(boss.y, boss.x);
                }
                if (projDelay % 85 == 84) {
                    shootl2(boss.y, boss.x);
                }
                if (upDelay % 2 == 0) {
                    boss.y = boss.y + boss.frame;
                    boss.x = boss.x + boss.dir;
                    upDelay = 0;
                }
                shadowOAM[11].attr0 = (ROWMASK & boss.y) | ATTR0_4BPP | ATTR0_SQUARE;
                shadowOAM[11].attr1 = (COLMASK & boss.x) | ATTR1_SIZE64;
                if (boss.health >= 750) {
                    shadowOAM[11].attr2 = 82;
                } else if (boss.health >= 500) {
                    shadowOAM[11].attr2 = 82 + 4 + 64;
                } else if (boss.health >= 250) {
                    shadowOAM[11].attr2 = 82 + 4 + 64 + 64;

                } else if (boss.health >= 0) {
                    shadowOAM[11].attr2 = 82 + 4 + 64 + 64 + 64;
                }

                int l1;
                int l2;
                for (l1 = 12; l1 < 22; l1++) {
                    if (lasers1[l1 - 12].active == 1) {
                        lasers1[l1 - 12].y += lasers1[l1 - 12].ydir;
                        lasers1[l1 - 12].x += lasers1[l1 - 12].xdir;
                        shadowOAM[l1].attr0 = (ROWMASK & lasers1[l1 - 12].y) | ATTR0_4BPP | ATTR0_SQUARE;
                        shadowOAM[l1].attr1 = (COLMASK & lasers1[l1 - 12].x) | ATTR1_SIZE16;
                        shadowOAM[l1].attr2 = 146;
                        if (lasers1[l1 - 12].x <= -16 || lasers1[l1 - 12].y >= 140) {
                            lasers1[l1 - 12].active = 0;
                        }
                    } else {
                        shadowOAM[l1].attr0 = ATTR0_HIDE;
                    }
                }
                for (l2 = 22; l2 < 32; l2++) {
                    if (lasers2[l2 - 22].active == 1) {
                        lasers2[l2 - 22].x += lasers2[l2 - 22].xdir;
                        shadowOAM[l2].attr0 = (ROWMASK & lasers2[l2 - 22].y) | ATTR0_4BPP | ATTR0_SQUARE;
                        shadowOAM[l2].attr1 = (COLMASK & lasers2[l2 - 22].x) | ATTR1_SIZE16;
                        shadowOAM[l2].attr2 = 146;
                        if (lasers2[l2 - 22].x <= -16) {
                            lasers2[l2 - 22].active = 0;
                        }
                    } else {
                        shadowOAM[l2].attr0 = ATTR0_HIDE;

                    }

                }


            }
            else if (explosion != 1) {
                shadowOAM[11].attr0 = ATTR0_HIDE;
                shadowOAM[43].attr0 = (ROWMASK & boss.y) | ATTR0_4BPP | ATTR0_SQUARE;
                shadowOAM[43].attr1 = (COLMASK & boss.x) | ATTR1_SIZE64;
                shadowOAM[43].attr2 = 82 + 4 + 64 + 64 + 64;
                explosion = 1;
                bossDeathDelay = 0;
                playSoundB(explode, EXPLODELEN, EXPLODEFREQ);
                int qq;
                for (qq = 0; qq < 20; qq++) {
                    shadowOAM[12 + qq].attr0 = ATTR0_HIDE;
                }
            }
            if (boss.health <= 0) {
                bossDeathDelay++;
            }
            if (bossDeathDelay > 0 && bossDeathDelay < 22) {
                int temp2 = (bossDeathDelay % 9) + 32;
                shadowOAM[temp2].attr0 = (ROWMASK & boss.y + ((rand() % 8)*7)) | ATTR0_4BPP | ATTR0_SQUARE;
                shadowOAM[temp2].attr1 = (COLMASK & boss.x + ((rand() % 8)*7)) | ATTR1_SIZE16;
                shadowOAM[temp2].attr2 = 342;
            }
            if (bossDeathDelay == 22) {
                int qq;
                for (qq = 0; qq < 9; qq++) {
                    shadowOAM[qq + 32].attr0 = ATTR0_HIDE;
                }
            }
            if (bossDeathDelay > 22 && bossDeathDelay < 45) {
                shadowOAM[41].attr0 = (ROWMASK & boss.y+16) | ATTR0_4BPP | ATTR0_SQUARE;
                shadowOAM[41].attr1 = (COLMASK & boss.x+16) | ATTR1_SIZE32;
                shadowOAM[41].attr2 = 346;
            }
            if (bossDeathDelay == 45) {
                shadowOAM[41].attr0 = ATTR0_HIDE;
            }
            if (bossDeathDelay > 45 && bossDeathDelay <= 70) {
                shadowOAM[42].attr0 = (ROWMASK & boss.y) | ATTR0_4BPP | ATTR0_SQUARE;
                shadowOAM[42].attr1 = (COLMASK & boss.x) | ATTR1_SIZE64;
                shadowOAM[42].attr2 = 362;
            }
            if (bossDeathDelay == 71) {
                shadowOAM[42].attr0 = ATTR0_HIDE;
                shadowOAM[43].attr0 = ATTR0_HIDE;
                //spawnPortal(496);
            }
            if (bossDeathDelay >= 161) {
                state = WINSCREEN;
                win();
            }
        }



        DMANow(3, shadowOAM, OAM, 512);

        waitForVblank();

        // copy the shadowOAM into the actual OAM (USE DMA!)


        REG_BG2HOFS = hOff;
        REG_BG2VOFS = vOff;
        REG_BG3HOFS = hOff2;
        REG_BG3VOFS = vOff2;
        REG_BG1HOFS = hOff3;
        REG_BG1VOFS = vOff3;
        if (realHOff == 276 && crossed == 0) {

            traveled++;
            alterMap(side, level, traveled);
            started = 1;
            crossed = 1;
            if (side == 0) side = 1;
        } else if (realHOff == 24 && started == 1 && crossed == 1) {

            traveled++;
            alterMap(side, level, traveled);
            crossed = 0;
            if (side == 1) side = 0;
        }
        if (realHOff == 512) realHOff = 0;
    }

}

void fire(int playerX, int playerY, int arrowCount) {
    arrows[arrowCount].x = playerX;
    arrows[arrowCount].y = playerY;
    arrows[arrowCount].gravity = 0;
    arrows[arrowCount].tempY = 1;
    if (flip == 1) {
        arrows[arrowCount].dir = -1;
    } else {
        arrows[arrowCount].dir = 1;
    }
}

void spawn1Enemy1(int i) {
    if (i == 1) {
        enemyGround newEnemy;
        if (groundEnemies[0].health <= 0) {
            newEnemy.x = 272;
            newEnemy.y = 64;
            newEnemy.dir = 1;
            newEnemy.frame = 0;
            newEnemy.health = 50;
            groundEnemies[0] = newEnemy;
        }
    } else if (i == 2) {
        enemyGround newEnemy;
        if (groundEnemies[0].health <= 0) {
            newEnemy.x = 272;
            newEnemy.y = 96;
            newEnemy.dir = 1;
            newEnemy.frame = 0;
            newEnemy.health = 50;
            groundEnemies[0] = newEnemy;
        }
    }

}

void spawn1Enemy2(int i) {
    enemyGround newEnemy;
    if (i == 1) {
        if (groundEnemies[1].health <= 0) {
            newEnemy.x = -32;
            newEnemy.y = 64;
            newEnemy.dir = -1;
            newEnemy.frame = 0;
            newEnemy.health = 50;
            groundEnemies[1] = newEnemy;
        }
    } else if (i == 2) {
        if (groundEnemies[1].health <= 0) {
            newEnemy.x = -32;
            newEnemy.y = 96;
            newEnemy.dir = -1;
            newEnemy.frame = 0;
            newEnemy.health = 50;
            groundEnemies[1] = newEnemy;
        }
    }
}

void spawnAir1(int i) {
    enemyAir newAir;
    if (i == 1) {
        if (airEnemies[0].health <= 0) {
            newAir.x = 272;
            newAir.y = 16;
            newAir.dir = 1;
            newAir.frame = 0;
            newAir.health = 50;
            airEnemies[0] = newAir;
        }
    } else if (i == 2) {
        if (airEnemies[0].health <= 0) {
            newAir.x = 272;
            newAir.y = 96;
            newAir.dir = 1;
            newAir.frame = 0;
            newAir.health = 50;
            airEnemies[0] = newAir;
        }
    }

}

void spawnAir2(int i) {
    enemyAir newAir;
    if (i == 1) {
        if (airEnemies[1].health <= 0) {
            newAir.x = -32;
            newAir.y = 16;
            newAir.dir = -1;
            newAir.frame = 0;
            newAir.health = 50;
            airEnemies[1] = newAir;
        }
    } else if (i == 2) {
        if (airEnemies[1].health <= 0) {
            newAir.x = -32;
            newAir.y = 96;
            newAir.dir = -1;
            newAir.frame = 0;
            newAir.health = 50;
            airEnemies[1] = newAir;
        }
    }

}

void shootl1(int row, int col) {
    if (boss.health > 0) {
        laser newLaser;
        newLaser.x = col + 6;
        newLaser.y = row + 52;
        newLaser.xdir = -3;
        newLaser.ydir = 1;
        newLaser.active = 1;
        lasers1[l1count] = newLaser;
        l1count++;
        if (l1count == 10) l1count = 0;
    }

}

void shootl2(int row, int col) {
    if (boss.health > 0) {
        laser newLaser;
        newLaser.x = col;
        newLaser.y = row + 32;
        newLaser.xdir = -3;
        newLaser.ydir = 0;
        newLaser.active = 1;
        lasers2[l2count] = newLaser;
        l2count++;
        if (l2count == 10) l2count = 0;
    }
}

void laserCol() {
    int dr;
    for (dr = 0; dr < 10; dr++) {
        if ((charCol <= lasers1[dr].x && (charCol + 16) >= lasers1[dr].x && charRow <= lasers1[dr].y && charRow + 32 >= (lasers1[dr].y + 16)) ||
                (charCol >= lasers1[dr].x && charCol <= (lasers1[dr].x + 32) && charRow <= lasers1[dr].y && charRow + 32 >= (lasers1[dr].y + 16))) {
            if (hpLossDelay <= 0) {
                if (invuln == 0) {
                    hp--;
                }
                hpLossDelay = 20;
            }
        }
    }
    int dr2;
    for (dr2 = 0; dr2 < 10; dr2++) {
        if ((charCol <= lasers2[dr2].x && (charCol + 16) >= lasers2[dr2].x && charRow <= lasers2[dr2].y && charRow + 32 >= (lasers2[dr2].y + 16)) ||
                (charCol >= lasers2[dr2].x && charCol <= (lasers2[dr2].x + 32) && charRow <= lasers2[dr2].y && charRow + 32 >= (lasers2[dr2].y + 16))) {
            if (hpLossDelay <= 0) {
                if (invuln == 0) {
                    hp--;
                }
                hpLossDelay = 20;
            }
        }
    }
}

void bossCol() {
    int ar;
    for (ar = 0; ar < 10; ar++) {
        if ((arrows[ar].x <= boss.x && (arrows[ar].x + 16) >= boss.x && arrows[ar].y >= boss.y && arrows[ar].y <= (boss.y + 64) ||
                (arrows[ar].x >= boss.x && arrows[ar].x <= (boss.x + 64) && arrows[ar].y >= boss.y && arrows[ar].y <= (boss.y + 64)))) {
            //(arrows[ar].x <= raptors[dr].x && arrows[ar].x + 16 >= raptors[dr].x && arrows[ar].y >= raptors[dr].y && arrows[ar].y <= raptors[dr].y) ||
            //(arrows[ar].x <= raptors[dr].x && arrows[ar].x + 16 >= raptors[dr].x && arrows[ar].y >= raptors[dr].y && arrows[ar].y <= raptors[dr].y)
            boss.health -= 75;
            if (boss.health <= 0) {

            }
            playSoundB(clink, CLINKLEN, CLINKFREQ);

            arrows[ar].x = -100;
            arrows[ar].y = -100;
        }
    }
}

void shootProjectile(enemyAir x) {
    if (projectiles[0].active == 0 || projectiles[1].active == 0 || projectiles[2].active == 0 && x.health >= 0) {
        projectiles[projNum].active = 1;
        if (level == 1) {
            if (x.dir == 1) {
                projectiles[projNum].x = x.x + 12;
            } else {
                projectiles[projNum].x = x.x + 20;
            }
            projectiles[projNum].y = x.y + 24;
            projectiles[projNum].dir = realHOff;
        } else if (level == 2) {
            if (x.dir == 1) {
                projectiles[projNum].x = x.x + 12;
                shadowOAM[21].attr1 = (COLMASK & (x.x - 20)) | ATTR1_SIZE8;
            } else {
                projectiles[projNum].x = x.x + 20;
                shadowOAM[21].attr1 = (COLMASK & (x.x + 44)) | ATTR1_SIZE8;
            }
            projectiles[projNum].y = x.y + 12;
            projectiles[projNum].dir = x.dir;
            playSoundB(gunshot, GUNSHOTLEN, GUNSHOTFREQ);
            shadowOAM[21].attr0 = (ROWMASK & (x.y + 8)) | ATTR0_4BPP | ATTR0_SQUARE;
            shadowOAM[21].attr2 = 435;
            expDelay = 0;
        }
        projNum++;
        if (projNum == 3) projNum = 0;
    }
}

void checkCollisionOne() {
    int dr;
    for (dr = 0; dr < 2; dr++) {
        if (groundEnemies[dr].health >= 1) {
            if (level == 1) {
                if ((charCol <= groundEnemies[dr].x && (charCol + 16) >= groundEnemies[dr].x && charRow >= groundEnemies[dr].y && charRow <= (groundEnemies[dr].y + 64)) ||
                        (charCol >= groundEnemies[dr].x && charCol <= (groundEnemies[dr].x + 32) && charRow >= groundEnemies[dr].y && charRow <= (groundEnemies[dr].y + 64))) {
                    if (hpLossDelay <= 0) {
                        if (invuln == 0) {
                            hp--;
                        }
                        hpLossDelay = 20;
                    }
                }
            } else if (level == 2) {
                if ((charCol <= groundEnemies[dr].x && (charCol + 16) >= groundEnemies[dr].x && charRow >= groundEnemies[dr].y && charRow <= (groundEnemies[dr].y + 32)) ||
                        (charCol >= groundEnemies[dr].x && charCol <= (groundEnemies[dr].x + 32) && charRow >= groundEnemies[dr].y && charRow <= (groundEnemies[dr].y + 32))) {
                    if (groundEnemies[dr].frame <= 5) {
                        groundEnemies[dr].frame = 6;
                    }
                    if (hpLossDelay <= 0) {
                        if (invuln == 0) {
                            hp--;
                        }
                        hpLossDelay = 20;
                    }
                }
            }
        }
    }
    int rr;
    for (rr = 0; rr < 4; rr++) {
        if (level == 1 || level == 2) {
            if ((projectiles[rr].x >= charCol && projectiles[rr].x <= (charCol + 8) && projectiles[rr].y >= charRow && projectiles[rr].y <= (charRow + 32)) ||
                    (projectiles[rr].x + 8 >= charCol && projectiles[rr].x + 8 <= (charCol + 8) && projectiles[rr].y >= charRow && projectiles[rr].y <= (charRow + 32))
                    ) {
                if (hpLossDelay <= 0) {
                    if (invuln == 0) {
                        hp--;
                    }
                    hpLossDelay = 20;

                }
                projectiles[rr].active = 0;
                projectiles[rr].x = -50;
                projectiles[rr].y = -50;
            }
        }
    }
}

void checkCollisionTwo() {
    int ar;
    for (ar = 0; ar < 10; ar++) {
        int dr;
        for (dr = 0; dr < 2; dr++) {
            if (groundEnemies[dr].health >= 1) {
                if (level == 1) {
                    if ((arrows[ar].x <= groundEnemies[dr].x && (arrows[ar].x + 16) >= groundEnemies[dr].x && arrows[ar].y >= groundEnemies[dr].y && arrows[ar].y <= (groundEnemies[dr].y + 64) ||
                            (arrows[ar].x >= groundEnemies[dr].x && arrows[ar].x <= (groundEnemies[dr].x + 32) && arrows[ar].y >= groundEnemies[dr].y && arrows[ar].y <= (groundEnemies[dr].y + 64)))) {
                        //(arrows[ar].x <= raptors[dr].x && arrows[ar].x + 16 >= raptors[dr].x && arrows[ar].y >= raptors[dr].y && arrows[ar].y <= raptors[dr].y) ||
                        //(arrows[ar].x <= raptors[dr].x && arrows[ar].x + 16 >= raptors[dr].x && arrows[ar].y >= raptors[dr].y && arrows[ar].y <= raptors[dr].y)
                        groundEnemies[dr].health -= 50;
                        if (groundEnemies[dr].health == 0) {
                            playSoundB(raptorDeath, RAPTORDEATHLEN, RAPTORDEATHFREQ);
                            enemyRaptorCorpse deadRaptor;
                            if (dr == 0) {
                                deadRaptor.x = groundEnemies[dr].x;
                                corpseDelay1 = 0;
                            } else {
                                corpseDelay2 = 0;
                                deadRaptor.x = groundEnemies[dr].x;
                            }
                            deadRaptor.y = groundEnemies[dr].y + 32;
                            deadRaptor.dir = realHOff;
                            deadRaptor.frame = 0;
                            groundCorpses[dr] = deadRaptor;
                            groundEnemies[dr].x = -50;
                            groundEnemies[dr].y = -50;
                        }

                        arrows[ar].x = -100;
                        arrows[ar].y = -100;

                    }
                } else if (level == 2) {
                    if ((arrows[ar].x <= groundEnemies[dr].x && (arrows[ar].x + 16) >= groundEnemies[dr].x && arrows[ar].y >= groundEnemies[dr].y && arrows[ar].y <= (groundEnemies[dr].y + 32) ||
                            (arrows[ar].x >= groundEnemies[dr].x && arrows[ar].x <= (groundEnemies[dr].x + 32) && arrows[ar].y >= groundEnemies[dr].y && arrows[ar].y <= (groundEnemies[dr].y + 32)))) {
                        //(arrows[ar].x <= raptors[dr].x && arrows[ar].x + 16 >= raptors[dr].x && arrows[ar].y >= raptors[dr].y && arrows[ar].y <= raptors[dr].y) ||
                        //(arrows[ar].x <= raptors[dr].x && arrows[ar].x + 16 >= raptors[dr].x && arrows[ar].y >= raptors[dr].y && arrows[ar].y <= raptors[dr].y)
                        groundEnemies[dr].health -= 50;
                        if (groundEnemies[dr].health <= 0) {
                            playSoundB(pirateDeath, PIRATEDEATHLEN, PIRATEDEATHFREQ);
                            if (dr == 0) {
                                corpseDelay1 = 0;
                            } else if (dr == 1) {
                                corpseDelay2 = 0;
                            }
                            groundEnemies[dr].dir = realHOff;
                            groundEnemies[dr].frame = 10;
                        }

                        arrows[ar].x = -100;
                        arrows[ar].y = -100;

                    }
                }
            }
        }
    }
}

void checkCollisionThree() {
    int ar;
    for (ar = 0; ar < 10; ar++) {
        int dr;
        if (level == 1) {


            for (dr = 0; dr < 2; dr++) {
                if (airEnemies[dr].health > 0) {
                    if ((arrows[ar].x <= airEnemies[dr].x && (arrows[ar].x + 16) >= airEnemies[dr].x && arrows[ar].y >= airEnemies[dr].y && arrows[ar].y <= (airEnemies[dr].y + 32) ||
                            (arrows[ar].x >= airEnemies[dr].x && arrows[ar].x <= (airEnemies[dr].x + 64) && arrows[ar].y >= airEnemies[dr].y && arrows[ar].y <= (airEnemies[dr].y + 32)))) {
                        //(arrows[ar].x <= raptors[dr].x && arrows[ar].x + 16 >= raptors[dr].x && arrows[ar].y >= raptors[dr].y && arrows[ar].y <= raptors[dr].y) ||
                        //(arrows[ar].x <= raptors[dr].x && arrows[ar].x + 16 >= raptors[dr].x && arrows[ar].y >= raptors[dr].y && arrows[ar].y <= raptors[dr].y)
                        airEnemies[dr].health -= 50;
                        if (airEnemies[dr].health <= 0) {
                            playSoundB(airDinoDeath, AIRDINODEATHLEN, AIRDINODEATHFREQ);
                            airEnemies[dr].frame = 6;
                            airEnemies[dr].dir = realHOff;
                        }
                        arrows[ar].x = -100;
                        arrows[ar].y = -100;

                    }
                }
            }
        } else if (level == 2) {
            for (dr = 0; dr < 2; dr++) {
                if (airEnemies[dr].health > 0) {
                    if ((arrows[ar].x <= airEnemies[dr].x && (arrows[ar].x + 16) >= airEnemies[dr].x && arrows[ar].y >= airEnemies[dr].y && arrows[ar].y <= (airEnemies[dr].y + 32) ||
                            (arrows[ar].x >= airEnemies[dr].x && arrows[ar].x <= (airEnemies[dr].x + 32) && arrows[ar].y >= airEnemies[dr].y && arrows[ar].y <= (airEnemies[dr].y + 32)))) {
                        //(arrows[ar].x <= raptors[dr].x && arrows[ar].x + 16 >= raptors[dr].x && arrows[ar].y >= raptors[dr].y && arrows[ar].y <= raptors[dr].y) ||
                        //(arrows[ar].x <= raptors[dr].x && arrows[ar].x + 16 >= raptors[dr].x && arrows[ar].y >= raptors[dr].y && arrows[ar].y <= raptors[dr].y)
                        airEnemies[dr].health -= 50;
                        if (airEnemies[dr].health <= 0) {
                            playSoundB(pirateDeath, PIRATEDEATHLEN, PIRATEDEATHFREQ);
                            if (dr == 0) {
                                airCorpseDelay1 = 0;
                            } else if (dr == 1) {
                                airCorpseDelay2 = 0;
                            }
                            airEnemies[dr].dir = realHOff;
                            airEnemies[dr].frame = 5;
                        }
                        arrows[ar].x = -100;
                        arrows[ar].y = -100;


                    }
                }
            }
        }
    }
}
