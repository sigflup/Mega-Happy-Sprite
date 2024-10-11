/*
 * Mega Happy Sprite is released under the BSD 3-Clause license.
 * read LICENSE for more info
 */


extern unsigned char buffer[0xffff];

int uudec(char *filename, char *name);
int uuenc(FILE *fp, char *name, unsigned char *buf, int len);
