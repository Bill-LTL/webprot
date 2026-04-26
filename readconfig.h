/*
實現讀取文件與自動填充配置至結構體功能（類動態變量實現）
*/
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

void read_config(char *first, const char *time);/*讀取配置文件函數
first用於確認是否進行過讀取操作，如已讀取過，則不打印初始化信息*/

void auto_fill(const char *time, char *key, char *val);//自動填充配置結構體函數



/*-------- 配置結構體 ----------*/
struct Config {
    char api[128];
    //char system[64];
    char cmd_get6[128];
    char zoneID[128];
    char recordID[128];
    //char DNS[128];
    char domain[128];
    char waittime[8];
    char ttl[8];
    char proxy[8];
    char ping6[128];

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
    //{"DNS", offsetof(struct Config, DNS)},
    {"domain", offsetof(struct Config, domain)},
    {"waittime", offsetof(struct Config, waittime)},
    {"ttl", offsetof(struct Config, ttl)},
    {"proxy", offsetof(struct Config, proxy)},
    {"ping6", offsetof(struct Config, ping6)},


};
//-------------------------------------


void read_config(char *first, const char *time) {
//讀取配置文件
    char key[64];
    char value[64];
    char line[256];
    FILE *fp = fopen(".config", "r");
    if (fp == NULL) {
    printf("%s [錯誤] 無法開啟 .config 文件！\n", time);
    return; 
    }
    while (fgets(line, sizeof(line), fp)) {
        if (sscanf(line, " %[^ \t=] %*[ \t=] %[^\r\n]", key, value) == 2) {
                if(*first==0){
                    printf("%s 已解析: key=[%s], value=[%s]\n", time, key, value);
                }
            auto_fill(time, key, value);
        }
    }
    fclose(fp);
    *first = 1;
}    

//實現填充功能，將key對應到結構體成員並賦值
//匹配key到變量,賦值value,,,,獲取IPv6命令（無則默認）
void auto_fill(const char *time, char *key, char *val) {
    
    for (int i = 0; i < sizeof(map) / sizeof(map[0]); i++) {
        if (strcmp(key, map[i].name) == 0) {            
            char *target_address = (char *)&cfg + map[i].offset;
            strcpy(target_address, val);   
        }
    }
    return;
}