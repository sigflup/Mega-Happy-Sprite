/*
 * Mega Happy Sprite is released under the BSD 3-Clause license.
 * read LICENSE for more info
 */

%{
#define BUF_LEN	1024
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <SDL2/SDL.h>
#include <libgui.h>
#include "mega.h"
#include "vdp.h"
#include "draw.h"
#include "proc.h"
#include "bottom.h"
#include "uu.h"

FILE *fp;
int a,b;
char c[BUF_LEN];
int i,j;

char vram[BUF_LEN];
char vsram[BUF_LEN];
char cram[BUF_LEN];
char load_message[BUF_LEN];


#define YY_INPUT(buf,result,max_size)\
{\
 int c = fgetc(fp); \
 result = (c == EOF) ? YY_NULL : (buf[0] = c, 1); \
}

#define SCAN_STRING(Q)\
 j = 0;\
 for(i=0;i<strlen(yytext);i++) {\
  if(yytext[i] == '=') {\
   while(yytext[++i]==' ') { }\
   if(yytext[i] == '\"') {\
    while(yytext[++i]!='\"') {\
     Q[j++] = yytext[i];\
    }\
    Q[j] = 0;\
    break;\
   } else {\
    while(yytext[i]!=' '&&yytext[i]!='\n') {\
     Q[j++] = yytext[i++];\
    }\
    Q[j] = 0;\
    break;\
   }\
  }\
 }

#define SCAN_NUMBERS\
 j = 0;\
 for(i=0;i<strlen(yytext);i++) {\
  if(yytext[i] == '=') {\
   i++;\
   while(yytext[i]!='\n'&&yytext[i]!=0)\
    if(yytext[i]!=',')\
     c[j++] = yytext[i++];\
    else {\
     i++;\
     c[j++] = ' ';\
    }\
   c[j] = 0;\
   break;\
  }\
 }


%}

%option noyywrap
%s uudecode
%s stops

DIGIT	[0-9]
HEX	[0-9a-fA-F]
COMMENT	(#.+\n)
HEAD	^\x20*
IS	\x20*=\x20*
TAIL	(\x20*(\n|({COMMENT}*)|<EOF>))
STRING	(\".+\")|[^\0x20\n<EOF>]+
STOP_ENTRY (\x20*0x{HEX}+\x20*\,*)

%%

{HEAD}{COMMENT} { }

{HEAD}load_message{IS}{STRING}{TAIL} {
 SCAN_STRING(load_message);
}


{HEAD}scroll_size{IS}{DIGIT}+\x20*\,\x20*{DIGIT}+{TAIL} {
 SCAN_NUMBERS;
 sscanf(c,"%d %d", &a,&b);
 if(a == 128)
  scroll_width = 2;
 if(a == 64)
  scroll_width = 1;
 if(a == 32)
  scroll_width = 0;
 if(b == 128)
  scroll_height = 2;
 if(b == 64)
  scroll_height = 1;
 if(b == 32)
  scroll_height = 0;
}

{HEAD}background_color{IS}{DIGIT}+\x20*\,\x20*{DIGIT}+{TAIL} {
 SCAN_NUMBERS;
 sscanf(c,"%d %d", &a, &b);
 if(a>=0 && a <= 3)
  current_vdp->bg_pal = a;
 if(b>=0 && b <= 15)
  current_vdp->bg_index = b;
}

{HEAD}vertical_scroll{IS}("SCREEN"|"TWOCELL"){TAIL} {
 SCAN_STRING(c);
 if(strncmp(c, "SCREEN",BUF_LEN)==0) 
  vertical_scroll_bar->param.d1 = VER_SCREEN;
 else 
  vertical_scroll_bar->param.d1 = VER_2CELLS;
}

{HEAD}tv_type{IS}("NTSC"|"PAL"){TAIL} {
 SCAN_STRING(c);
 if(strncmp(c,"NTSC", BUF_LEN)==0)
  current_vdp->tv_type = NTSC;
 else
  current_vdp->tv_type = PAL;
}

{HEAD}cell_w{IS}{DIGIT}+{TAIL} {
 SCAN_STRING(c);
 sscanf(c, "%d", &a);
 if(a == 32 || a == 40) 
  current_vdp->cell_w = a;
}

{HEAD}horizontal_scroll{IS}("SCREEN"|"CELL"|"LINE"){TAIL} {
 SCAN_STRING(c);								
 if(strncmp(c,"SCREEN",BUF_LEN)== 0) 
  horizontal_scroll_bar->param.d1 = HOR_SCREEN;
 if(strncmp(c,"CELL",BUF_LEN) == 0) 
  horizontal_scroll_bar->param.d1 = HOR_8LINES;
 if(strncmp(c,"LINE",BUF_LEN) == 0)
  horizontal_scroll_bar->param.d1 = HOR_LINES;
 
}

{HEAD}vram{IS}{STRING}{TAIL} {
 SCAN_STRING(vram);
}

{HEAD}cram{IS}{STRING}{TAIL} {
 SCAN_STRING(cram);
}

{HEAD}vsram{IS}{STRING}{TAIL} {
 SCAN_STRING(vsram);
}


{HEAD}scroll_a{IS}0x{HEX}+{TAIL} {
 SCAN_STRING(c);
 sscanf(c, "0x%x", &a);
 if( ((a&0xe000)==a))
  current_vdp->scroll_a = a;
}

{HEAD}scroll_b{IS}0x{HEX}+{TAIL} {
 SCAN_STRING(c);
 sscanf(c, "0x%x", &a);
 if( ((a&0xe000)==a))
  current_vdp->scroll_b = a;
}

{HEAD}window{IS}0x{HEX}+{TAIL} {
 SCAN_STRING(c);
 sscanf(c, "0x%x", &a);
 if((a&(current_vdp->cell_w == 40 ? 0xf000: 0xf8000))==a)
  current_vdp->window = a;
}

{HEAD}hscroll{IS}0x{HEX}+{TAIL} {
 SCAN_STRING(c);
 sscanf(c, "0x%x", &a);
 if((a&0xfc00)==a)
  current_vdp->hscroll = a;
}

{HEAD}sprite_table{IS}0x{HEX}+{TAIL} {
 SCAN_STRING(c);
 sscanf(c, "0x%x", &a);
 if((a&(current_vdp->cell_w == 40 ? 0xfc00 : 0xfe00))==a)
  current_vdp->sprite_table = a;
}


<stops>{HEAD}"end-stops"{TAIL} {
 BEGIN(0);
}

<stops>{HEAD}{STOP_ENTRY}+{TAIL} {
 j = 0;
 for(i = 0; i< strlen(yytext)-1;) {
  if(yytext[i] ==   '0' &&
     yytext[i+1] == 'x') {
   i+=2;
   j = 0;
   for(;;) {
    c[j] = yytext[i];
    if(c[j] == ' ' ||
       c[j] == ',' ||
       c[j] == '\n' ||
       c[j] == 0) {
     c[j] = 0;
     break;
    } 
    j++;
    i++;
    
   }
  } else {
   i++; 
   j = 0;
  }
  if(j!=0) {
   sscanf(c, "%x", &a);
   pat_stop[a] = TRUE;
  }
 }

}

<stops>{HEAD}{TAIL} { 
}

{HEAD}"start-stops"{TAIL} {
 for(i=1;i<0x400;i++)
   pat_stop[i] = FALSE; 
  pat_stop[0] = TRUE;
 BEGIN(stops);
}


{HEAD}begin(-base64)?\x20+{STRING}\x20+{STRING}\x20*\n {
 BEGIN(uudecode);
} 

<uudecode>{HEAD}end\x20*\n|<EOF> {
 BEGIN(0);
}

<uudecode>{HEAD}.+\n { }

<stops>^.+\n {
 printf("Stops command not recognized, ignoring: %s", yytext);
}

^.+\n {
 printf("Command not recognized, ignoring: %s", yytext);
}

\x20|\n { }

%%

int load_save_mega(struct select_file_t *selector, char *filename) {
 int i;
 int j;
 int found;
 unsigned char cram_buffer[128];
 if(selector->usr_flags == LOAD) {

  vram[0] = 0;
  vsram[0] = 0;
  cram[0] = 0;
  load_message[0] = 0; 

  fp = fopen(filename, "r");
  if(!fp)
   return NOPE_TRY_AGAIN;

  yylex();

  if(load_message[0] != 0) 
   alert(gui_screen->w/2, gui_screen->h/2, load_message, "Okay");

  fclose(fp);
  if(vram[0] != 0)
   if((uudec(filename, vram))==1)
    memcpy(current_vdp->vram, buffer, 0xffff);
  
  if(vsram[0] != 0)
   if((uudec(filename, vsram))==1)
    memcpy(current_vdp->vsram, buffer, 80);
  
  if(cram[0] != 0)
   if((uudec(filename, cram))==1)
    load_palette(current_vdp, buffer);
  render_vdp(0,vdp_h);
  selection_v1.x = NO_SELECTION;
 } else {
  fp = fopen(filename, "w");
  if(fp<0)
   return NOPE_TRY_AGAIN;
  fprintf(fp, "# This is a mega file for use with Mega-Happy-Sprite.\n"
              "# please read MANUAL or click on Help for a description\n"
	      "# -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n\n");
  if(load_message[0]!=0) 
   fprintf(fp, "load_message = \"%s\"\n\n", load_message);

  fprintf(fp, "scroll_size = %d,%d\n\n", 1<<(5+scroll_width), 1<<(5+scroll_height));
  fprintf(fp, "vertical_scroll = %s\n", 
    (vertical_scroll_bar->param.d1 == VER_SCREEN ? "SCREEN" : "TWOCELL"));
  fprintf(fp, "horizontal_scroll = ");
  if(horizontal_scroll_bar->param.d1 == HOR_SCREEN)
   fprintf(fp, "SCREEN\n\n");
  if(horizontal_scroll_bar->param.d1 == HOR_8LINES)
   fprintf(fp, "CELL\n\n");
  if(horizontal_scroll_bar->param.d1 == LINE)
   fprintf(fp, "LINE\n\n");
  fprintf(fp, "scroll_a = 0x%x\n", current_vdp->scroll_a);
  fprintf(fp, "scroll_b = 0x%x\n", current_vdp->scroll_b);
  fprintf(fp, "window = 0x%x\n", current_vdp->window);
  fprintf(fp, "sprite_table = 0x%x\n", current_vdp->sprite_table);
  fprintf(fp, "hscroll = 0x%x\n\n", current_vdp->hscroll);
  fprintf(fp, "background_color = %d,%d\n\n", current_vdp->bg_pal, current_vdp->bg_index);
  fprintf(fp, "vram = VRAM\n"
              "cram = CRAM\n"
	      "vsram = VSRAM\n\n");

  fprintf(fp, "start-stops\n");
  found = FALSE;

  for(i=1;i<0x400;i++)
   if( pat_stop[i] == TRUE) {
    found = TRUE;
    j = 0;
    for(;i<0x400;i++)
    if(pat_stop[i] == TRUE) {
     if(j!=0) {
      if(j<8) 
       fputc(',', fp);
      else {
       j = 0;
       fprintf(fp, "\n ");
      }
     } else
      fputc(' ', fp);
     fprintf(fp, "0x%03x", i);
     j++;
     }
    if(j<=8) 
     fputc('\n', fp);
    break;
   }

  if(found == FALSE) 
   fprintf(fp, " # A null stop segment means clears all the stops\n");
  fprintf(fp, "end-stops\n\n");
  uuenc(fp, "VSRAM", current_vdp->vsram, 80);
  store_palette(current_vdp, cram_buffer);
  uuenc(fp, "CRAM", cram_buffer, 128);
  uuenc(fp, "VRAM", current_vdp->vram, 0xffff);
  fclose(fp);
 }
 return LOAD_OK_QUIT;
}
