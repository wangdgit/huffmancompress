#include "huffman.h"

void findf(char *tpf,char files[100][256],int *cnt);
void getfile(char *str,char files[100][256],int *cnt);
unsigned long long getsize(char *path);

//通配符*
void findf(char *tpf,char files[100][256],int *cnt){
    WIN32_FIND_DATAA fd;
    HANDLE find=FindFirstFileA(tpf, &fd);
    *cnt=0;
    if(find==INVALID_HANDLE_VALUE) return;
    //提取子文件夹
    char p[256]={0};
    char *last=strrchr(tpf,'\\');
    if(last){
        int len=last-tpf+1;
        strncpy(p,tpf,len);
        p[len]='\0';
    }

    while(1){
        //跳过.和..
        if(strcmp(fd.cFileName,".")==0||strcmp(fd.cFileName,"..")==0){
            if(!FindNextFileA(find, &fd)) break;
            continue;
        }
        //跳过文件夹找文件
        if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)){
            sprintf(files[*cnt],"%s%s",p,fd.cFileName);
            (*cnt)++;
        }
        if(!FindNextFileA(find, &fd)) break;
    }
    FindClose(find);
}
//读输入
void getfile(char *str,char files[100][256],int *cnt){
    int num=0;
    int idx=0;
    char tmp[256]={0};
    for(int i=0;str[i]!='\0';i++){
        if(str[i]==','){
            tmp[idx]='\0';
            strcpy(files[num++],tmp);
            idx=0;
        }else{
            tmp[idx++]=str[i];
        }
    }
    if(idx){
        tmp[idx]='\0';
        strcpy(files[num++],tmp);
    }
    *cnt=num;
}

//文件大小
unsigned long long getsize(char *path){
    FILE *fp=fopen(path,"rb");
    if(!fp) return 0;
    fseek(fp,0,SEEK_END);
    unsigned long long size=ftell(fp);
    fclose(fp);
    return size;
}

int main(int argc,char *argv[]){
    SetConsoleOutputCP(65001);//UTF-8
    //命令行
    if(argc>1){
        clock_t start,end;
        double ut;
        //压缩
        if(strcmp(argv[1],"-c")==0&&argc>=4){
            char files[100][256];
            int num=0;
            if(strchr(argv[3],'*')) findf(argv[3],files,&num);
            else getfile(argv[3],files,&num);
            if(!num){
                printf("未找到任何文件！压缩失败！\n");
                return 1;
            }
            start=clock();
            printf("正在压缩 %d 个文件到 [%s]...\n",num,argv[2]);
            compress(files,num,argv[2]);
            end=clock();
            unsigned long long rsize=0,cmpsize=0;
            for(int i=0;i<num;i++) rsize+=getsize(files[i]);
            cmpsize=getsize(argv[2]);
            ut=(double)(end-start)/1000;
            printf("压缩完成！耗时：%.2f 秒，压缩比：%.2f%%\n",ut,cmpsize*100.0/rsize);
            return 0;
        }
        //解压
        else if(strcmp(argv[1],"-d")==0&&argc>=3){
            char out[1024]={0};
            if(argc>=4) strncpy(out,argv[3],1023);//自定义目录
            start=clock();
            printf("正在解压 [%s]...\n",argv[2]);
            decompress(argv[2],out);
            end=clock();
            ut=(double)(end-start)/1000;
            printf("解压完成！耗时：%.2f 秒\n",ut);
            return 0;
        }
        else{
            printf("用法：\n");
            printf("压缩: %s -c <输出压缩包> <输入文件或通配符...>\n",argv[0]);
            printf("解压: %s -d <压缩包> [输出目录]\n",argv[0]);
            return 0;
        }
    }    
    //死循环
    int d;
    while(1){
        printf("\n======== 霍夫曼压缩工具 ========\n");
        printf("0： 退出程序\n");
        printf("1： 压缩文件\n");
        printf("2： 解压文件\n");
        printf("========================\n");
        printf("选择：\n");
        scanf("%d",&d);
        getchar();

        // 退出
        if(!d) break;

        // 压缩
        if(d==1){
            printf("\n----- 压缩模式 -----\n");
            char in[1024];//输入路径
            char files[100][256];//文件路径
            int num=0;//文件数
            char out[1024];//输出路径
            clock_t start,end;//计时
            unsigned long long rsize=0,cmpsize=0;//大小
            double ut;//耗时
            double b;//比

            printf("可使用通配符*\n");
            printf("输入文件路径(多个用英文逗号分隔)：\n");
            fgets(in,1024,stdin);
            in[strcspn(in,"\n")]=0;//去换行
            
            if(strchr(in,'*')) findf(in,files,&num);
            else getfile(in,files,&num);

            if(!num){
                printf("未找到任何文件！压缩失败！\n");
                system("pause"); // 暂停
                continue;
            }

            int way;
            printf("压缩包位置：1:完整路径 2:仅名称(同级目录)：\n");
            scanf("%d",&way);
            getchar();

            if(way==1){
                printf("请输入完整压缩包路径+名称：");
                fgets(out,1024,stdin);
            }else{
                printf("请输入压缩包名称：");
                fgets(out,1024,stdin);
            }
            out[strcspn(out,"\n")] = 0;//同上

            //计时
            start=clock();
            compress(files,num,out);
            end=clock();
            ut=(double)(end-start)/1000;

            //计算
            for(int i=0;i<num;i++) rsize+=getsize(files[i]);
            cmpsize=getsize(out);
            printf("\n----- 压缩结果 -----\n");
            printf("压缩文件个数：%d\n",num);
            printf("压缩耗时：%.2f 秒\n",ut);
            printf("压缩比：%.2f%%\n",cmpsize*100.0/rsize);
            printf("压缩完成\n");
        }
        //解压
        else if(d==2){
            printf("\n----- 解压模式 -----\n");
            char in[1024];//输入路径
            char out[1024];//输出路径
            clock_t start,end;//计时
            double ut;//耗时
            
            printf("请输入压缩包路径：");
            fgets(in,1024,stdin);
            in[strcspn(in,"\n")]=0;

            int way;
            printf("解压位置：1:指定路径 2:程序同级目录：");
            scanf("%d",&way);
            getchar();

            if(way==1){
                printf("请输入目标文件夹路径：");
                fgets(out,1024,stdin);
                out[strcspn(out,"\n")]=0;
            }else out[0]=0;

            start=clock();
            decompress(in, out);
            end=clock();
            ut=(double)(end-start)/1000;

            printf("\n----- 解压结果 -----\n");
            printf("解压耗时：%.2f 秒\n",ut);
            printf("解压完成！\n");
        }
        else{
            printf("输入错误！\n");
            system("pause");
        }
        
    }
    return 0;
}
