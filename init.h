/*-------------------------------------------------------
Copyright (C) 2026.7.25 by Bill_LTL

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program. If not, see <https://www.gnu.org/licenses/>.
---------------------------------------------------------*/


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
void check_env(); //檢查環境依賴函數
int is_cmd_exist(const char *cmd); //檢查命令是否存在函數

char *run_cmd(const char *time, const char cmd[]);//執行命令並回傳輸出結果函數


const char *getipv6;  //於填充函數中獲得賦值「void auto_fill(char *key, char *val)」
const char *ping6;  //於填充函數中獲得賦值「void auto_fill(char *key, char *val)」



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
    buffer[strcspn(buffer, "\r\n")] = 0;
    return buffer;
}



// 檢查所有必要的包
void check_env() {
    const char *dependencies[] = {"fping", "curl", "python3", "ip"};
    int missing = 0;

    printf("[系統] 正在檢查環境依賴...\n");

    for (int i = 0; i < sizeof(dependencies) / sizeof(dependencies[0]); i++) {
        if (is_cmd_exist(dependencies[i])) {
            printf("  [OK] %s 已安裝\n", dependencies[i]);
        } else {
            printf("  [錯誤] %s 未找到！\n", dependencies[i]);
            missing++;
        }
    }

    if (missing > 0) {
        printf("\n[錯誤] 缺少 %d 個必要組件。請執行以下命令安裝：\n", missing);
        printf("sudo apt update && sudo apt install fping curl python3 iproute2 -y\n");
        exit(1); // 強制退出程式
    }
    printf("[系統] 環境檢查通過，啟動程式...\n\n");
}
int is_cmd_exist(const char *cmd) {
    char check_cmd[128];
    
    snprintf(check_cmd, sizeof(check_cmd), "command -v %s >/dev/null 2>&1", cmd);
    return (system(check_cmd) == 0);
}
//-----------