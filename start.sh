#!/bin/bash

if [ "$EUID" -ne 0 ]; then
    echo "請使用 sudo 執行此腳本！"
    exit 1
fi


RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
BLUE='\033[0;34m'
NC='\033[0m'

CONF_FILE=".config"
SERVICE_NAME="webprot.service"
EXEC_NAME="webprot.out"

echo -e "${BLUE}=== WebProtect 系統環境與配置檢查 ===${NC}"



check_packages() {
    echo -e "${GREEN}[1/4] 正在檢查系統依賴...${NC}"
    local deps=("fping" "curl" "ip" "awk" "grep")
    local missing=()

    for cmd in "${deps[@]}"; do
        if ! command -v "$cmd" >/dev/null 2>&1; then
            missing+=("$cmd")
        fi
    done

    if [ ${#missing[@]} -ne 0 ]; then
        echo -e "${RED}[XX] 缺少必要組件: ${missing[*]}${NC}"
        echo -e "${YELLOW}正在嘗試自動安裝...${NC}"
        sudo apt update && sudo apt install fping curl iproute2 -y
        if [ $? -ne 0 ]; then
            echo -e "${RED}[錯誤] 自動安裝失敗，請手動執行: sudo apt install ${missing[*]}${NC}"
            exit 1
        fi
    else
        echo -e "  [OK] 所有必要依賴包已就緒。"
    fi
}

# --- 配置合法性深度檢查 ---
check_config() {
    echo -e "${GREEN}[2/4] 正在校驗配置內容...${NC}"
    
    if [ ! -f "$CONF_FILE" ]; then
        echo -e "${RED}[XX] 錯誤: 找不到 $CONF_FILE 檔案！${NC}"
        exit 1
    fi

    
    local critical_keys=("api" "zoneID" "recordID" "domain")
    local error_found=0

    for key in "${critical_keys[@]}"; do
        
        local val=$(grep "^${key}" "$CONF_FILE" | cut -d'=' -f2- | cut -d'#' -f1 | xargs)
        
        if [[ -z "$val" ]]; then
            echo -e "${RED}[XX] 配置錯誤: '${key}' 缺失或為空值。${NC}"
            error_found=1
        elif [[ "$val" =~ ^[[:space:]]+$ ]]; then
            echo -e "${RED}[XX] 配置錯誤: '${key}' 僅包含空格，這是無效的。${NC}"
            error_found=1
        else
            echo -e "  [OK] ${key} 驗證通過。"
        fi
    done

    if [ $error_found -eq 1 ]; then
        echo -e "${RED}請修正 .config 檔案後再次運行此腳本。${NC}"
        exit 1
    fi
}

# --- 執行檔與權限檢查 ---
check_execution() {
    echo -e "${GREEN}[3/4] 正在檢查執行環境...${NC}"
    
    if [ ! -f "./$EXEC_NAME" ]; then
        echo -e "${RED}[XX] 錯誤: 在當前目錄找不到執行檔 $EXEC_NAME。${NC}"
        exit 1
    fi

    # 修復執行權限
    chmod +x "$EXEC_NAME"
    
    sudo chmod u+s $(command -v fping)
    echo -e "  [OK] 執行權限與 fping SUID 已修復。"
}

# --- Systemd 服務部署與啟動 ---
deploy_service() {
    echo -e "${GREEN}[4/4] 正在部署 Systemd 服務...${NC}"
    local CUR_DIR=$(pwd)
    
    sudo cat <<EOF > /etc/systemd/system/$SERVICE_NAME
[Unit]
Description=WebProtect Cloudflare Failover Service
After=network.target

[Service]
User=root
WorkingDirectory=${CUR_DIR}
ExecStart=${CUR_DIR}/$EXEC_NAME
Restart=always
RestartSec=5
StandardOutput=journal
StandardError=journal

[Install]
WantedBy=multi-user.target
EOF

    sudo systemctl daemon-reload
    sudo systemctl enable $SERVICE_NAME
    echo -e "${GREEN}[✓] 服務部署完成，正在啟動...${NC}"
    sudo systemctl restart $SERVICE_NAME
}

# --- 執行流程 ---
check_packages
check_config
check_execution
deploy_service

echo -e "${BLUE}====================================${NC}"
echo -e "${YELLOW}一切正常！正在進入實時日誌監控 (Ctrl+C 退出日誌):${NC}"
sleep 2
sudo journalctl -u $SERVICE_NAME -f