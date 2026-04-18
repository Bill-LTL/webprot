#include "somefunc.h"
#include <unistd.h>

int main() {
    read_config();    
    //優先自定義獲取ipv6命令
    if(strcmp(cfg.cmd_get6, "default") == 0){
        getipv6 = "ip -6 addr show enp6s0 | grep 'scope global' | grep '/128' | awk '{print $2}' | cut -d'/' -f1";
        printf("%s 使用預設獲取IPv6命令: %s\n", get_now(), getipv6);  
            
    }else{
        getipv6 = cfg.cmd_get6;
        printf("%s 使用配置文件獲取IPv6命令: %s\n", get_now(), getipv6);  
    }
   
    char *ipv6 ;
    ipv6 = run_cmd(getipv6);

    while(1){
        read_config();
        printf("%s %s監控中...\n", get_now(), ipv6);
        sleep(5);
    }

    return 0;

}