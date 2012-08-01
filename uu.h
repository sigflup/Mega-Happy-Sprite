extern unsigned char buffer[0xffff];

int uudec(char *filename, char *name);
int uuenc(FILE *fp, char *name, unsigned char *buf, int len);
