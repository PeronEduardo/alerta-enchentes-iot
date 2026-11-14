// =====================================================================
//  MODELO de credenciais.
//  1. Copie este arquivo e renomeie a cópia para  secrets.h
//  2. Cole o ACCESS TOKEN do dispositivo criado no ThingsBoard
//  O secrets.h está no .gitignore -> o token NUNCA vai para o GitHub.
// =====================================================================
#pragma once

#define WIFI_SSID   "Wokwi-GUEST"   // rede virtual do Wokwi (aberta)
#define WIFI_SENHA  ""

#define TB_SERVIDOR "mqtt.thingsboard.cloud"
#define TB_PORTA    8883            // 8883 = MQTT sobre TLS (1883 seria sem criptografia)
#define TB_TOKEN    "COLE_SEU_ACCESS_TOKEN_AQUI"
