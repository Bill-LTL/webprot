/*
實現讀取文件與自動填充配置至結構體功能（類動態變量實現）
*/


void read_config(const char *time);   //讀取配置文件函數
void auto_fill(const char *time, char *key, char *val);//自動填充配置結構體函數



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


void read_config(const char *time) {
//讀取配置文件
    char key[64];
    char value[64];
    char line[256];
    FILE *fp = fopen(".config", "r");
    while (fgets(line, sizeof(line), fp)) {
        if(sscanf(line, " %[^ \t=] = %s", key, value) == 2){
            printf("%s 已解析: key=[%s], value=[%s]\n", time, key, value);
            auto_fill(time, key, value);
        }
    }
    fclose(fp);
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