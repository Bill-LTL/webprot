//-----------兼容定義---
#ifdef _WIN32
    #define SYSTEM "windows"
    #define POPEN _popen
    #define PCLOSE _pclose
#elif defined(__linux__) || defined(__apple__)
    #define SYSTEM "linux"
    #define POPEN popen
    #define PCLOSE pclose
#endif
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include "readconfig.h"

const char* get_now();//獲取當前時間函數

char *run_cmd(const char *time, const char cmd[]);//執行命令並回傳輸出結果函數


const char *getipv6;  //於填充函數中獲得賦值「void auto_fill(char *key, char *val)」




//獲取時間的函數
const char* get_now() {
    static char buffer[32];
    time_t rawtime;
    struct tm *info;

    time(&rawtime);
    info = localtime(&rawtime);
    
    // 格式：[00.0.00,00:00:00]
    strftime(buffer, sizeof(buffer), "[%y.%-m.%-d,%H:%M:%S]", info);
    
    return buffer;
}

//讀取命令輸出
char *run_cmd(const char *time, const char cmd[]) {
    printf("%s 執行命令: %s\n", time, cmd);
    FILE *fp = POPEN(cmd, "r");
    if (fp == NULL) {
        printf("%s 無法執行命令: %s\n",time, cmd);
        return NULL;
    }
    static char buffer[1024];
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        printf("%s 執行讀取: %s\n", time, buffer);
    }
    PCLOSE(fp);
    
    return buffer;
}
//-----------