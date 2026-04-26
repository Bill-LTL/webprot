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
#include "init.h"
#include <unistd.h>
char failover_core(char *last_state, const char *time); 
char update_dns_api(const char *time, const char *new_ip);
void get_dns_v6_from_api(const char *time, char *output);//從 API 獲取當前 DNS 記錄的 IPv6 地址

struct ip6_info {
    char sever[255];
    char host_ip[255];
    char target_ip[255];
} ip6;

int main() {
    check_env();
    //---------------------初始化結構體---------------------
    char last_state = 0; // 0: 正常, 1: 離線
    char first = 0;
    read_config(&first, get_now());
    //優先自定義獲取ipv6命令
    if(strcmp(cfg.cmd_get6, "default") == 0 || strcmp(cfg.cmd_get6,"") == 0){
        getipv6 = "ip -6 addr show eth0 | grep 'scope global' | grep '/128' | awk '{print $2}' | cut -d'/' -f1";
        printf("%s 使用預設獲取IPv6命令: %s\n", get_now(), getipv6);  
            
    }else{
        getipv6 = cfg.cmd_get6;
        printf("%s 使用配置文件獲取IPv6命令: %s\n", get_now(), getipv6);  
    }



    char full_cmd[512];

    get_dns_v6_from_api(get_now(), ip6.sever);
    strncpy(ip6.host_ip, run_cmd(get_now(), getipv6), sizeof(ip6.host_ip) - 1);
    
    //ip6.host_ip[strcspn(ip6.host_ip, "\r\n")] = 0;
    //---------------------------------------------
    char status = 0;
    

    while(status != -1) {
        read_config(&first, get_now());
        failover_core(&last_state, get_now());
        printf("%s [監控] 監控循環結束，等待下一次檢查...wait for %s sec\n", get_now(), cfg.waittime);
        sleep(atoi(cfg.waittime)); 
    }
    

    printf("program exited with error code: %d\n", status);

    return 0;

}

//----------------------------------------------------------
char failover_core(char *last_state, const char *time) {
    
    // 存活檢查
    char cmd[256];
    get_dns_v6_from_api(get_now(), ip6.target_ip);
    snprintf(cmd, sizeof(cmd), "fping -6 -c 3 -t 1000 -q %s > /dev/null 2>&1",ip6.target_ip);
    
    // 執行命令並獲取狀態
    int status;
    
    int is_down = system(cmd); 

    if (is_down != 0) {  //0爲ping通，非0爲ping不通
        // --- 離線 ---
        if (*last_state == 0) {
            printf("%s [警報] 服務器 %s 離線！正在切換 DNS 至宿主機...\n", time, ip6.sever);
            
            // 呼叫 API 更新 DNS 為宿主機 IP
            update_dns_api(time, ip6.host_ip);            
            *last_state = 1; // 切換狀態
        }else{
            printf("%s 看來配置出問題了\n", time);
        }
    } else {
        // --- 在線 ---
        if (*last_state == 1) {
            // 接管狀態
            get_dns_v6_from_api(get_now(), ip6.target_ip);
            
            if(strcmp(ip6.target_ip, ip6.host_ip) == 0){
                printf("%s [接管] 宿機接管運作中，DNS 已指向宿主機。\n", time);
            }else{
                *last_state = 0;
                printf("%s [正常] 服務器已恢復。\n", time);
            }
            }else{
                
                if(strcmp(ip6.target_ip, ip6.host_ip) == 0){  //防止程序重啓後狀態錯誤，如果DNS已經指向宿主機，則不再切換回服務器IP
                    printf("%s [監測] DNS已指向宿主機。\n", time);
                *last_state = 1;
                }else{
                    printf("%s [監測] 正常。\n", time);
    }
    }
}
        return status;
    
}


char update_dns_api(const char *time, const char *new_ip) {
    char cmd[2048];
    
    

    printf("%s [DNS] 準備將記錄更新為: %s\n", time, new_ip);

    
    snprintf(cmd, sizeof(cmd),
        "curl -s -X PATCH \"https://api.cloudflare.com/client/v4/zones/%s/dns_records/%s\" "
        "-H \"Authorization: Bearer %s\" "
        "-H \"Content-Type: application/json\" "
        "--data '{\"type\":\"AAAA\",\"name\":\"%s\",\"content\":\"%s\",\"ttl\":%s,\"proxied\":%s}'",
        cfg.zoneID,   // 區域 ID
        cfg.recordID, // 記錄 ID
        cfg.api,      // API 令牌
        cfg.domain,   // 域名 
        new_ip,       // 新的 IPv6 地址
        cfg.ttl,      // TTL 值
        cfg.proxy     // Proxy 值
    );

    // 執行命令
    FILE *pipe = popen(cmd, "r");
    if (!pipe) {
        printf("%s [錯誤] 無法啟動 curl 執行 DNS 更新\n", time);
        return 0;
    }

    char response[1024];
    if (fgets(response, sizeof(response), pipe) != NULL) {
        // 判斷 Cloudflare 回傳成功標誌
        if (strstr(response, "\"success\":true")) {
            printf("%s [成功] DNS 已更新為 %s\n", time, new_ip);
        } else {
            printf("%s [失敗] API 報錯: %s\n", time, response);
        }
    }

    pclose(pipe);
    return 0;
}

//從 API 獲取當前 DNS 記錄的 IPv6 地址
void get_dns_v6_from_api(const char *time, char *output) {
    char cmd[1024];
    
    // 構造 Cloudflare GET 請求
    snprintf(cmd, sizeof(cmd),
        "curl -s -X GET \"https://api.cloudflare.com/client/v4/zones/%s/dns_records/%s\" "
        "-H \"Authorization: Bearer %s\" "
        "-H \"Content-Type: application/json\"",
        cfg.zoneID, cfg.recordID, cfg.api);

    
    char *response = run_cmd(time, cmd);

    if (response != NULL) {
        // 尋找 JSON 中的 "content":"..." 欄位
        char *p = strstr(response, "\"content\":\"");
        if (p) {
            p += 11; // 跳過 "content":"
            char *end = strchr(p, '\"');
            if (end) {
                size_t len = end - p;
                strncpy(output, p, len);
                output[len] = '\0';
                printf("%s [API 查詢成功] 雲端當前記錄為: %s\n", time, output);
                return;
            }
        }
    }
    printf("%s [API 查詢失敗] 無法解析 IP 內容\n", time);
}