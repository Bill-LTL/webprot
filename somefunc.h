#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
const char* get_now();//獲取當前時間函數
void read_config();   //讀取配置文件函數
void auto_fill(char *key, char *val);//自動填充配置結構體函數
char *run_cmd(const char cmd[]);//執行命令並回傳輸出結果函數


const char *getipv6;  //於填充函數中獲得賦值「void auto_fill(char *key, char *val)」

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

/*-------- 配置結構體 ----------*/
struct Config {
    char api[64];
    //char system[64];
    char cmd_get6[128];
    char zoneID[64];
    char recordID[64];
    char DNS[64];
} cfg;
//-----------配置變量偏移量表------------
struct {
    char *name;
    size_t offset;
} map[] = {
    {"api",  offsetof(struct Config, api)},
    //{"system", offsetof(struct Config, system)},
    {"cmd_get6", offsetof(struct Config, cmd_get6)},
    {"zoneID", offsetof(struct Config, zoneID)},
    {"recordID", offsetof(struct Config, recordID)},
    {"DNS", offsetof(struct Config, DNS)}
};
//-------------------------------------

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
void read_config() {
//讀取配置文件
    char key[64];
    char value[64];
    char line[256];
    FILE *fp = fopen(".config", "r");
    while (fgets(line, sizeof(line), fp)) {
        if(sscanf(line, " %[^ \t=] = %s", key, value) == 2){
            printf("%s 已解析: key=[%s], value=[%s]\n", get_now(), key, value);
            auto_fill(key, value);
        }
    }
    fclose(fp);
    printf("%s api: %s\n", get_now(), cfg.api);
    printf("%s system: %s\n", get_now(), SYSTEM);
}    

//實現填充功能，將key對應到結構體成員並賦值



//匹配key到變量,賦值value,,,,獲取IPv6命令（無則默認）
void auto_fill(char *key, char *val) {
    
    for (int i = 0; i < sizeof(map) / sizeof(map[0]); i++) {
        if (strcmp(key, map[i].name) == 0) {            
            char *target_address = (char *)&cfg + map[i].offset;
            strcpy(target_address, val);   
        }
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