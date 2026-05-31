#ifndef HUFFMAN_H
#define HUFFMAN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>

typedef struct HFMnode{
    unsigned char ch;
    unsigned long long weight;
    struct HFMnode *left, *right;
}HFMnode;

typedef struct{
    unsigned char code[256];
    unsigned char len;
}HFMcode;

void creatcode(HFMnode *node,unsigned char *buf,int len,HFMcode codes[256]);
void encode(HFMnode *root,HFMcode codes[256]);
HFMnode* buildtree(unsigned long long freq[256]);
void freetree(HFMnode *root);
int compress(char in[100][256],int num,char *out);
int decompress(char *in,char *out);

#endif