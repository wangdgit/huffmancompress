#include "huffman.h"

//生成编码
void creatcode(HFMnode *node,unsigned char *buf,int len,HFMcode codes[256]){
    if(!node->left && !node->right){
        memcpy(codes[node->ch].code,buf,len);
        codes[node->ch].len=len;
        return;
    }
    if(node->left){
        buf[len]=0;
        creatcode(node->left,buf,len+1,codes);
    }
    if(node->right){
        buf[len]=1;
        creatcode(node->right,buf,len+1,codes);
    }
}
//编码表
void encode(HFMnode *root,HFMcode codes[256]){
    memset(codes,0,256*sizeof(HFMcode));
    unsigned char buf[256];
    creatcode(root,buf,0,codes);
}
//树
HFMnode* buildtree(unsigned long long freq[256]){
    int num=0;
    for(int i=0;i<256;i++) if(freq[i]>0) num++;
    //空
    if(num==0) return NULL;
    //特殊情况，单字符（aaa.txt）
    if(num==1){
        int ch=0;
        for(int i=0;i<256;i++) if(freq[i]>0){
            ch=i;
            break;
        }
        HFMnode *leaf=malloc(sizeof(HFMnode));
        leaf->ch = ch;
        leaf->weight = freq[ch];
        leaf->left = leaf->right = NULL;

        HFMnode *root=malloc(sizeof(HFMnode));
        root->ch = 0;
        root->weight = freq[ch];
        root->left = leaf;
        root->right = NULL;
        return root;
    }

    HFMnode **nodes=malloc(num*sizeof(HFMnode*));
    int f=0;
    for(int i=0;i<256;i++) if(freq[i]>0){
        nodes[f]=malloc(sizeof(HFMnode));
        nodes[f]->ch = i;
        nodes[f]->weight = freq[i];
        nodes[f]->left = nodes[f]->right = NULL;
        f++;
    }

    int n=num;
    while(n>1){
        for(int i=0;i<n-1;i++) for(int j=i+1;j<n;j++){
            if(nodes[i]->weight > nodes[j]->weight){
                HFMnode *t=nodes[i];
                nodes[i]=nodes[j];
                nodes[j]=t;
            }
        }

        HFMnode *parent=malloc(sizeof(HFMnode));
        parent->ch = 0;
        parent->weight = nodes[0]->weight + nodes[1]->weight;
        parent->left = nodes[0];
        parent->right = nodes[1];

        nodes[0]=parent;
        nodes[1]=nodes[n-1];
        n--;
    }

    HFMnode *root=nodes[0];
    free(nodes);
    return root;
}
//释放树
void freetree(HFMnode *root){
    if(!root) return;
    freetree(root->left);
    freetree(root->right);
    free(root);
}
//压缩
int compress(char in[100][256],int num,char *out){
    remove("tmp.tmp");
    FILE *outfp=fopen(out,"wb");
    if(!outfp) return -1;
    fwrite(&num,sizeof(int),1,outfp);

    for(int i=0;i<num;i++){
        FILE *infp=fopen(in[i],"rb");
        if(!infp) continue;
        const char *n=strrchr(in[i],'\\');
        if(!n) n=strrchr(in[i],'/');
        if(n) n++;
        else n=in[i];
        fseek(infp,0,SEEK_END);
        unsigned long long fsize=ftell(infp);
        fseek(infp,0,SEEK_SET);

        char nbuf[256]={0}; 
        strncpy(nbuf,n,255);
        fwrite(nbuf,256,1,outfp);
        fwrite(&fsize,8,1,outfp);

        unsigned long long freq[256]={0};
        unsigned char buf[4096];
        int rlen;
        while((rlen=fread(buf,1,4096,infp))>0)
            for(int j=0;j<rlen;j++) freq[buf[j]]++;
        fseek(infp,0,SEEK_SET);

        HFMnode *root=buildtree(freq);
        HFMcode table[256]; 
        encode(root,table);

        FILE *tmpfp=fopen("tmp.tmp","wb");
        unsigned char byte=0; 
        int bit=0;
        while(( rlen=fread(buf,1,4096,infp) )>0){
            for(int j=0;j<rlen;j++){
                unsigned char c=buf[j];
                for(int k=0; k<table[c].len; k++){
                    byte=(byte<<1)|table[c].code[k];
                    if(++bit==8){
                        fputc(byte,tmpfp);
                        byte=bit=0;
                    }
                }
            }
        }
        if(bit>0) fputc(byte<<(8-bit),tmpfp);
        fclose(tmpfp);

        FILE *tmpread=fopen("tmp.tmp","rb");
        fseek(tmpread,0,SEEK_END);
        unsigned long long tmpsize=ftell(tmpread);
        fseek(tmpread,0,SEEK_SET);

        fwrite(freq,sizeof(unsigned long long),256,outfp);
        fwrite(&tmpsize,8,1,outfp);
        while((rlen=fread(buf,1,4096,tmpread))>0)
            fwrite(buf,1,rlen,outfp);
        fclose(tmpread);
        fclose(infp);
        freetree(root);
    }
    fclose(outfp); 
    remove("tmp.tmp");
    return 0;
}
// 解压
int decompress(char *in,char *out){
    FILE *infp=fopen(in,"rb");
    if(!infp) return -1;
    int filenum;
    fread(&filenum,sizeof(int),1,infp);

    for(int i=0;i<filenum;i++){
        char filename[256]={0};
        unsigned long long fsize,tmpsize;
        unsigned long long freq[256];

        fread(filename,256,1,infp);
        fread(&fsize,8,1,infp);
        fread(freq,sizeof(unsigned long long),256,infp);
        fread(&tmpsize,8,1,infp);

        FILE *tmpfp=fopen("tmp.tmp","wb");
        unsigned char buf[4096];
        int rlen;
        unsigned long long readed=0;
        while(readed<tmpsize && (rlen=fread(buf,1,4096,infp))>0){
            unsigned long long need=tmpsize-readed;
            int towrite;
            if(rlen>need) towrite=need;
            else towrite=rlen;
            fwrite(buf,1,towrite,tmpfp); 
            readed+=towrite;
            if(rlen>towrite) fseek(infp,towrite-rlen,SEEK_CUR);
        }
        fclose(tmpfp);

        char path[1024];
        if(strlen(out)==0) strcpy(path,filename);
        else sprintf(path,"%s\\%s",out,filename);
        FILE *outfp=fopen(path,"wb");
        if(!outfp) continue;

        // 独立读临时文件解码
        FILE *tmpin=fopen("tmp.tmp","rb");
        HFMnode *root=buildtree(freq);
        HFMnode *f=root;
        unsigned long long num=0;
        
        while(num<fsize && (rlen=fread(buf,1,4096,tmpin))>0){
            for(int j=0;j<rlen && num<fsize;j++){
                unsigned char byte=buf[j];
                for(int k=0;k<8;k++){
                    if(byte&0x80) f=f->right;
                    else f=f->left;
                    byte<<=1;
                    if(!f->left && !f->right){
                        fputc(f->ch,outfp); 
                        num++; 
                        f=root;
                    }
                }
            }
        }
        fclose(outfp); 
        fclose(tmpin); 
        freetree(root);
    }
    fclose(infp); 
    remove("tmp.tmp");
    return 0;
}