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
    printf("%s system: %s\n", get_now(), SYSTEM);
    
    
//優先自定義獲取ipv6命令
    if(strcmp(cfg.cmd_get6, "default") == 0){
        getipv6 = "ip -6 addr show enp6s0 | grep 'scope global' | grep '/128' | awk '{print $2}' | cut -d'/' -f1";
        printf("%s 使用預設獲取IPv6命令: %s\n", get_now(), GET_IPV6);  
            
    }else{
        getipv6 = cfg.cmd_get6;
        printf("%s 使用配置文件獲取IPv6命令: %s\n", get_now(), getipv6);  
    }
//--------------------系統檢查     

    
    char *ipv6 ;
    ipv6 = run_cmd(GET_IPV6);

    while(1){
        
        printf("%s %s監控中...\n", get_now(), ipv6);
        sleep(5);
    }

    return 0;

}