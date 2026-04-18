#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>

const char *getipv6;
//-----------兼容定義---
#ifdef _WIN32
    #define POPEN _popen
    #define PCLOSE _pclose
    #define  GET_IPV6 "ipconfig"
#elif defined(__linux__) || defined(__apple__)
    #define POPEN popen
    #define PCLOSE pclose
    #define  GET_IPV6 getipv6
#endif


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


//實現填充功能，將key對應到結構體成員並賦值
struct Config {
    char api[64];
    char system[64];
    char cmd_get6[128];
} cfg;

struct {
    char *name;
    size_t offset;
} map[] = {
    {"api",  offsetof(struct Config, api)},
    {"system", offsetof(struct Config, system)},
    {"cmd_get6", offsetof(struct Config, cmd_get6)}
};

//匹配key到變量,賦值value,,,,獲取IPv6命令（無則默認）
void auto_fill(char *key, char *val) {
    
    for (int i = 0; i < sizeof(map) / sizeof(map[0]); i++) {
        if (strcmp(key, map[i].name) == 0) {            
            char *target_address = (char *)&cfg + map[i].offset;
            strcpy(target_address, val);   
        }
    }
    //優先自定義獲取ipv6命令
    if(strcmp(cfg.cmd_get6, "default") == 0){
        getipv6 = "ip -6 addr show enp6s0 | grep 'scope global' | grep '/128' | awk '{print $2}' | cut -d'/' -f1";
        printf("%s 使用預設獲取IPv6命令: %s\n", get_now(), GET_IPV6);  
            
    }else{
        getipv6 = cfg.cmd_get6;
        printf("%s 使用配置文件獲取IPv6命令: %s\n", get_now(), getipv6);  
    }
    return;
}

//讀取命令輸出
char *run_cmd(const char cmd[]) {
    printf("%s 執行命令: %s\n", get_now(), cmd);
    FILE *fp = POPEN(cmd, "r");
    if (fp == NULL) {
        printf("%s 無法執行命令: %s\n", get_now(), cmd);
        return NULL;
    }
    static char buffer[128];
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        printf("%s 執行讀取: %s", get_now(), buffer);
    }
    PCLOSE(fp);
    
    return buffer;
}
//-----------