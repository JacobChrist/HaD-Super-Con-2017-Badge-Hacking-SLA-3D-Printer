#include "cambadge.h"
#include "globals.h"

// file browser   

#define brmax 10 // no of files displayed 
//states 
#define s_startbrowse 0
#define s_restartbrowse 1
#define s_showbrowse 2
#define s_waitbrowse 3
#define s_infobrowse 4
#define s_ibwait 5
#define s_gobrowse 6
#define s_aviplaying 7
#define s_gbwait 8     
#define s_quitbrowse 9
#define s_newdir 10
#define s_delete 11
#define s_avipause 12
#define s_startshow 13
#define s_waitshow 14
#define s_nextshow 15
#define s_quitshow 16

#define s_slice 17
#define s_slice_wait 18
#define s_slice_break 19
#define s_slice_start 20
#define s_slice_start_wait 21

#define build_up u1txstring("/0 MI -150000\r");

char* sla_printer(unsigned int action) {
    unsigned i, j, y;

    static unsigned int brstate = s_startbrowse;
    static unsigned int brscroll, brsel, brlast, brnfiles, brendflag, showtime, showtimer, shown;

    static unsigned int slice = 0;
    static unsigned int slice_timer = 0;
    
    static char brname[15];
    static unsigned int brattrs, brlen, brtime, brtype;

    if (action == act_name) return ("3D PRINTER");
    else if (action == act_help) return ("Displays files on\nSD Card");
    if (action == act_start) brstate = s_startbrowse;
    if (action != act_poll) return (0);
#if debug_dma==1
    printf("Debug_DMA option set" del);
    brstate = s_quitbrowse;
#else       
    if (!cardmounted) {
        printf(cls inv "NO CARD" inv del);
        brstate = s_quitbrowse;
    } // card removed
#endif
    switch (brstate) {

        case s_startbrowse:
          FSchdir("\\JACOB\\");
          brendflag = 0;
          slice = 0;
          brstate = s_slice_start;
          break;

        case s_slice_start:
//            slice_timer = 0;
          printf(cls red "\n\n\n\n\n" whi);
          printf(red "** PRESS BUTTON 1 **\n" whi);
          printf(red "** TO LOWER PLATE **\n" whi);
          brstate = s_slice_start_wait;
          u1txstring("\r");
          u1txstring("/0 H+\r");

          break;

        case s_slice_start_wait:
          if (!butpress) break;
          if (butpress & (powerbut | but2)) {
              build_up;
              brstate = s_quitbrowse;
              break;
          } // exit

          if (butpress & (but1)) { // but3 is the go button.
              u1txstring("/0 HM 0\r");
              printf(cls);
              brstate = s_slice;
              break;
          }

          break;

        case s_slice:
          sprintf(brname, "jolly%d.bmp", slice);
          //slice +=9;
          if( slice > 180 ) slice = 0;
          // load the slice
          //printf(cls);
          //printf(cls inv "%s" inv del, brname);

          i = loadbmp(brname, 2);
          //printf(inv "%s" inv del, brname);

          if (i) {
              printf(red "Error:\n%s" whi, avierrors[i]);
              //break;
          }

          brstate = s_slice_wait;
          break;
            
        case s_slice_wait:
            if(!tick) return(0);
            //if(!tick) 
            slice_timer++;
            if( slice_timer > 250) { // 250 = 5s, 750 = 15s, 
                printf(cls);
                u1txstring("/0 layer\r");
                brstate = s_slice_break;
                slice_timer = 0;
            }

            if (!butpress) break;
              if (butpress & (powerbut | but2)) {
                  build_up;
                  brstate = s_quitbrowse;
                  break;
            } // exit

            if (butpress & (but3 | but5)) { // but3 is the go button.
                brstate = s_slice;
                break;
            }
            break;
            
        case s_slice_break:
            if(!tick) return(0);
            //if(!tick) 
            slice_timer++;
            if( slice_timer > 300) {
                brstate = s_slice;
                slice_timer = 0;
            }

            if (!butpress) break;
              if (butpress & (powerbut | but2)) {
                  build_up;
                  brstate = s_quitbrowse;
                  break;
            } // exit

            if (butpress & (but3 | but5)) { // but3 is the go button.
                brstate = s_slice;
                break;
            }
            break;
            
///////////////////////////////////////////////////////////////////////////////////
// Old browser code (not used now/yet)
///////////////////////////////////////////////////////////////////////////////////
            
        case s_newdir:

            brsel = 0;
            brscroll = 0;
            brendflag = 1;

        case s_restartbrowse: // return to previous brstate after doing something with a file

            brnfiles = 0;
            i = FindFirst("*.*", ATTR_MASK &~ATTR_VOLUME, &searchfile);
            if (i) {
                printf(cls inv "No files" inv del);
                brstate = s_quitbrowse;
                break;
            }
            do brnfiles++; while (FindNext(&searchfile) == 0);
            printf(cls);
            if (brsel > brnfiles - 1) brsel = brnfiles - 1; // in case file deleted
            brstate = s_showbrowse;

            if (!brendflag) break;
            // if not at root, when entering a dir set position at last file for quick access to photos
            FSgetcwd(brname, 4);
            if (brname[1] == 0) break; // at root ( 1 char path long name) 
            brsel = brnfiles - 1;
            if (brsel > brmax) brscroll = brnfiles - brmax;
            brendflag = 0;
            break;



        case s_showbrowse:
            i = FindFirst("*.*", ATTR_MASK&~ATTR_VOLUME, &searchfile);
            y = 0;
            for (i = 0; i != brscroll; i++) FindNext(&searchfile); // scroll offset
            printf(tabx0 taby1);
            dispy -= 3; // vertically centre
            do {
                bgcol = c_blk;
                fgcol = (searchfile.attributes & ATTR_DIRECTORY) ? c_yel : c_whi;
                if (y + brscroll == brsel) { // currently selected file - grab the file info
                    bgcol = fgcol;
                    fgcol = c_blk;
                    brattrs = searchfile.attributes;
                    brlen = searchfile.filesize;
                    brtime = searchfile.timestamp;
                    for (brtype = 0, j = 0, i = 0; i != 13; i++) {
                        brname[i] = searchfile.filename[i];
                        if (j) if ((brtype & 0xFF0000) == 0) brtype = (brtype << 8) | (unsigned int) searchfile.filename[i]; // file extension as word for easy comparison
                        if (searchfile.filename[i] == '.') j = 1; //flag to start copying extension

                    }
                }

                printf("\n%-20s", searchfile.filename);
                //while(dispx<15*charwidth) dispchar(' '); // pad to erase any previous text after scroll
            } while (((i = FindNext(&searchfile)) == 0) && (++y < brmax));


            printf(top butcol "EXIT" whi " %3d Items" butcol tabx16, brnfiles);
            printf((brattrs & ATTR_DIRECTORY) ? "Show" : "Info");

            printf(bot butcol "  " uarr "         " darr"      Go" whi);
            brstate = s_waitbrowse;
            break;

        case s_waitbrowse: // wait for button

            if (!butpress) break;
            if (butpress & (powerbut)) {
                brstate = s_quitbrowse;
                break;
            } // exit
            if (brnfiles == 0) break;
            if (butpress & but4) {
                brstate = (brattrs & ATTR_DIRECTORY) ? s_startshow : s_infobrowse;
                break;
            }
            if (butpress & (but3 | but5)) { // but3 is the go button.
                if (brattrs & ATTR_DIRECTORY) {
                    FSchdir(brname);
                    brstate = s_newdir;
                    break;
                }// enter dir, start at ..
                else {
                    brstate = s_gobrowse;
                    break;
                }
            }

            if (butpress & but1) { // up
                if (brsel) brsel--;
                if (brsel < brscroll) brscroll--;
            }

            if (butpress & but2) if (brsel < brnfiles - 1) { // down
                    brsel++;
                    if (brsel - brscroll >= brmax) brscroll++;
                }

            brstate = s_showbrowse;
            break;

        case s_infobrowse: // show file info
            printf(cls whi "%s\n\n", brname);
            const char months[16][4] = {"???", "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec", "???", "???", "???"};
            printf("%02d %s %04d  %02d:%02d\n\n", (brtime >> 16) & 0x1f, months[((brtime >> 21)&0x0f)], 1980 + ((brtime >> 25)&0x7f), (brtime >> 11) &0x1f, (brtime >> 5)&0x3f);
            printf("Length %d\n\n", brlen);
            switch (brtype) {
                case filetype('A', 'V', 'I'):
                    i = openavi(brname);
                    FSfclose(fptr);

                    if (i) printf("\nError:\n%d %s", i, avierrors[i]);
                    else {
                        printf("%d x %d, %2d Bpp\n\n", avi_width, avi_height, avi_bpp * 8);
                        printf("%d Frames %d FPS\n", avi_frames, 1000000 / avi_frametime);
                    }
                    break;

                case filetype('B', 'M', 'P'):
                    i = loadbmp(brname, 0);
                    printf("%d x %d, %2d Bpp\n\n", avi_width, avi_height, avi_bpp * 8);
                    if (i) {
                        printf("Error:\n%s", avierrors[i]);
                        break;
                    }

                    break;

                case filetype('T', 'X', 'T'):

                    break;

            } // switch brtype

            printf(butcol bot "Back    Delete     Go" whi);
            brstate = s_ibwait;

            break;

        case s_ibwait:

            if (!butpress) break;
            brstate = s_restartbrowse;
            if (butpress & but2) brstate = s_delete;
            if (butpress & but3)brstate = s_gobrowse;

            break;



        case s_gobrowse:


            brstate = s_gbwait; //default
            printf(cls whi);
            switch (brtype) {
                case filetype('A', 'V', 'I'):


                    i = openavi(brname);

                    if (i) {
                        FSfclose(fptr);
                        printf(red "Open error:\n%s\n" whi, avierrors[i]);
                        brstate = s_gbwait;
                        break;
                    }


                    i = avi_frametime * (clockfreq / 1000000) / 256;
                    T2CON = 0b1000000001110000; // prescale 256
                    PR2 = i;
                    TMR2 = 0;
                    IFS0SET = _IFS0_T2IF_MASK; // force first
                    brstate = s_aviplaying;
                    printf(bot butcol "Pause    Delete  Back");

                    if (avi_height > 96) delayus(300000); // display prompt before overwritten by image 
                    break;

                case filetype('B', 'M', 'P'):
                    printf(cls);
                    i = loadbmp(brname, 2);

                    if (i) {
                        printf(red "Error:\n%s" whi, avierrors[i]);
                        break;
                    }
                    printf(whi bot butcol tabx8 "Delete   Back");
                    break;


                default:
                    printf(cls "Don't know what to\ndo with that filetype");
                    printf(whi bot butcol tabx8 "Delete   Back");
                    break;

            } // switch brtype

            break;

        case s_gbwait:

            if (!butpress) break;
            brstate = s_restartbrowse;

            if (butpress & but2) brstate = s_delete;

            break;


        case s_delete:
            printf(bot butcol "Hold to delete       " del);
            readbuttons();
            if (butstate & but2) {
                printf(bot "Deleting        ");
                FSremove(brname);
                if(brscroll) brscroll--;
            } else printf(bot "Not deleted");
            
            brstate = s_restartbrowse;

            break;

        case s_avipause:
            if (!butpress) break;
            if (butpress & but1) {
                printf(bot butcol "Pause");
                butpress = 0;
            } // avoid pause loop
            // drop back into playing to other button functions work
            brstate = s_aviplaying;

        case s_aviplaying:
            if (butpress & but2) {
                FSfclose(fptr);
                brstate = s_delete;
                break;
            } //delete
            if (butpress & but1) {
                brstate = s_avipause;
                printf(bot butcol inv "Pause" inv);
                break;
            }

            if (butpress) {
                FSfclose(fptr);
                brstate = s_restartbrowse;
                break;
            }
            if (IFS0bits.T2IF == 0) break;
            IFS0CLR = _IFS0_T2IF_MASK;

            i = showavi();
            if (avi_height <= 96) printf(top yel "%4d/%-4d %4ds", avi_framenum, avi_frames, avi_framenum * avi_frametime / 1000000);
            if (i) {
                FSfclose(fptr);
                printf(cls red "Play Error %s" whi);
                brstate = s_ibwait;
                break;
            }
            break;


        case s_quitbrowse:
            return ("");
            break;

        case s_startshow:
            brstate = s_quitshow; // assume error

            FSchdir(brname);
            i = FindFirst("*.BMP", ATTR_MASK&~ATTR_VOLUME, &searchfile);
            if (i) {
                printf(cls whi "BMP Slideshow\n\nNo .BMP files" del del);
                break;
            }

            showtime = 2000000 / ticktime;
            showtimer = showtime;
            printf(cls butcol "EXIT" bot "Slower   Faster  Next" tabx0 taby3 whi "BMP Slideshow" del del);
            shown = 0;
            brstate = s_waitshow;
            break;


        case s_waitshow:
            if (butpress & powerbut) brstate = s_quitshow;
            if (butpress & but3) showtimer = showtime;
            if (butpress & but1) if (showtime > 1000000 / ticktime) showtime -= (1000000 / ticktime);
            if (butpress & but2) if (showtime < 10000000 / ticktime) showtime += 1000000 / ticktime;
            if (butpress & (but2 | but1)) printf(bot tabx4 whi "%2d", showtime / (1000000 / ticktime));
            if (!tick) break;
            showtimer += tick;
            if (showtimer < showtime) break;
            printf(cls);
            do {
                printf(top grey "%s " whi, searchfile.filename);
                i = loadbmp(searchfile.filename, 2);
                if (i == 0) shown = 1; // found at least one good file

                if (FindNext(&searchfile)) { // no more files

                    if (shown) i = FindFirst("*.BMP", ATTR_MASK&~ATTR_VOLUME, &searchfile);
                    else {
                        i = 0; //force exit
                        printf(whi tabx0 taby3 "\nNo suitable\nFiles found" del del);
                        brstate = s_quitshow;
                    }

                } // end of files
            }// while bad file
            while (i);

            showtimer = 0;


            break;


        case s_quitshow:
            FSchdir("..");
            brstate = s_restartbrowse;
            break;

        default: brstate = s_startbrowse;
    } //switch(brstate


    return (NULL);

}
