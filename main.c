#include "somefunc.h"
#include <unistd.h>

int main() {
//--------------------系統檢查  基本讀取  
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
    printf("%s system: %s\n", get_now(), cfg.system);
//--------------------系統檢查     

    
    char *ipv6 ;
    ipv6 = run_cmd(GET_IPV6);

    while(1){
        sleep(5);
        printf("%s %s 監控中...\n", get_now(), ipv6);
        break;
    }

    return 0;

}