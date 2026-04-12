#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>


struct Config {
    char api[64];
    char system[64];
} cfg;

struct {
    char *name;
    size_t offset;
} map[] = {
    {"api",  offsetof(struct Config, api)},
    {"system", offsetof(struct Config, system)}
};
//匹配key到變量並賦值value
void auto_fill(char *key, char *val) {
    
    for (int i = 0; i < sizeof(map) / sizeof(map[0]); i++) {
        if (strcmp(key, map[i].name) == 0) {            
            char *target_address = (char *)&cfg + map[i].offset;
            strcpy(target_address, val);
            return;
        }
    }
}

int main() {
    //讀取配置文件
    char key[64];
    char value[64];
    char line[256];
    FILE *fp = fopen("config.txt", "r");
    while (fgets(line, sizeof(line), fp)) {
        if(sscanf(line, " %[^ \t=] = %s", key, value) == 2){
            printf("成功解析: key=[%s], value=[%s]\n", key, value);
            auto_fill(key, value);
        }
        
    }
    fclose(fp);

    printf("api: %s\n", cfg.api);
    printf("system: %s\n", cfg.system);
    return 0;

}