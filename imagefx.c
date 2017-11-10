
// some simple image processing demos

#include "cambadge.h"
#include "globals.h"

// states used by this application

#define s_start 0
#define s_run 1
#define s_quit 2


#define bufsize (128*96+1)
#define bufstart 8
#define linelength 129

char* imagefx(unsigned int action) {
    static unsigned int state, effect, page, val1, val2;
    unsigned int x, y, i, d, r, g, b;


    switch (action) {
        case act_name: return ("IMAGEFX");
        case act_help: return ("Camera effects");

        case act_start:
            // called once when app is selected from menu
            state = s_start;
            effect = 0;
            cam_enable(cammode_128x96_z1_mono);
            cam_grabenable(camen_start, bufstart - 1, 0);
            page = 1;
            val1 = val2 = 0;

            return (0);
    } //switch

    if (action != act_poll) return (0);


    if (butpress & powerbut) state = s_quit; // exit with nonzero value to indicate we want to quit 
    if (butpress & but1) {
        effect++;
        val1 = val2 = 0;
        state = s_start;
    }
    if (butpress & but4) if (led1) led1_on;
        else led1_off;
    switch (state) {
        case s_start:
            printf(cls top butcol "EXIT  " inv "IMAGEFX" inv "  LIGHT" bot "Effect");
            state = s_run;
            for (i = 0; i != cambufsize; cambuffer[i++] = 0);

        case s_run:
            if (!cam_newframe) break;

            printf(tabx0 taby11 yel);
            switch (effect) {

                case 0: // update one line per frame
                    printf("Slowscan");
                    monopalette(0, 255);
                    plotblock(0, 11 + ypixels - val1, xpixels, 1, c_grn);
                    dispimage(0, 12 + ypixels - val1, xpixels, 1, (img_mono | img_revscan), cambuffer + bufstart + val1 * xpixels);
                    if (++val1 == ypixels - 1) val1 = 0;

                    break;
                    unsigned char* charptr;
                    unsigned short* shortptr;

                case 1: // temporal FIR filter

                    printf("Ghost");
                    charptr = cambuffer + bufstart;
                    shortptr = cambuffer_s + (bufstart + bufsize) / 2; // shorts to store pixel:fraction as fixed-point to avoid rounding errors
                    y = 250;
                    for (i = 0; i != xpixels * ypixels; i++) {
                        x = *charptr++;
                        d = x * (255 - y)+((unsigned int) *shortptr * y) / 256; // mix proportion of old and new pixel
                        *shortptr++ = d;
                    }
                    monopalette(0, 255);
                    // img_skip skips over the lsbytes, displaying the MSbyte
                    dispimage(0, 12, xpixels, ypixels, (img_mono | img_revscan | img_skip1), cambuffer + bufsize + bufstart);

                    break;


                case 2: // use camera capture start parameters to de-stabilise
                    printf("Unstabilise");
                    monopalette(0, 255);
                    xstart = 30 + randnum(-15, 15);
                    ystart = 30 + randnum(-15, 15);

                    dispimage(0, 12, xpixels, ypixels, (img_mono | img_revscan), cambuffer + bufstart);

                    break;



                default: effect = 0;

            }//switch effect

            cam_grabenable(camen_start, bufstart - 1, 0); // buffer swap for new frame    

            break;

        case s_quit:
            cam_grabdisable();
            return ("");

            break;


    }

    return (0);


}


