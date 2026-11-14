# Arquitetura e Requisitos

## Ciclo da IoT aplicado (Percepção → Transmissão → Análise → Ação)

| Etapa | No projeto |
|---|---|
| Percepção | HC-SR04 mede a distância até a água; DHT22 mede temperatura e umidade |
| Análise | ESP32 converte distância → nível e classifica o risco (Edge Computing) |
| Ação | LEDs e sirene local; sirene acionada remotamente pelo dashboard |
| Transmissão | MQTT sobre TLS (porta 8883) até o ThingsBoard Cloud |

## Processamento dos dados

1. **Distância:** `d = (t × 0,0343) / 2` (t em µs, d em cm; divide por 2 porque o som vai e volta)
2. **Nível:** `nível = H − d`, com H = 400 cm (altura do sensor em relação ao fundo do rio)
   - Exemplo: d = 150 cm → nível = 400 − 150 = **250 cm** → estado **ATENÇÃO**
3. **Classificação:** NORMAL < 200 ≤ ATENÇÃO < 280 ≤ ALERTA < 340 ≤ EMERGÊNCIA

## Requisitos Funcionais

| RF | Descrição | Implementação |
|---|---|---|
| RF01 | Coleta de dados | HC-SR04 + DHT22 (Wokwi); sensores de água (Packet Tracer) |
| RF02 | Processamento | Cálculo de nível, classificação de risco, JSON |
| RF03 | Conectividade Wi-Fi | ESP32 (Wokwi); Home Gateways (Packet Tracer) |
| RF04 | Comunicação com a nuvem | ThingsBoard Cloud via MQTT |
| RF05 | Visualização | Dashboard ThingsBoard + painel do IoT Server (Packet Tracer) |
| RF06 | Atuação remota | RPC `setSirene` aciona a sirene do ESP32 |

## Requisitos Não Funcionais

| RNF | Descrição | Implementação |
|---|---|---|
| RNF01 | Escalabilidade | Novo dispositivo no ThingsBoard / novo gateway no Packet Tracer sem alterar o restante |
| RNF02 | Confiabilidade | Reconexão automática ao Wi-Fi e ao MQTT; ação local funciona sem internet |
| RNF03 | Segurança | TLS na porta 8883; token por dispositivo; WPA2-AES no Packet Tracer; token fora do GitHub |
| RNF04 | Usabilidade | Estados por cor; um único painel |
| RNF05 | Eficiência energética | Intervalo de envio adaptativo ao risco |

## Protocolo de comunicação (ThingsBoard)

| Direção | Tópico MQTT | Exemplo de mensagem |
|---|---|---|
| ESP32 → nuvem | `v1/devices/me/telemetry` | `{"nivel_cm":250.0,"estado":"ATENCAO",...}` |
| nuvem → ESP32 | `v1/devices/me/rpc/request/{id}` | `{"method":"setSirene","params":true}` |
| ESP32 → nuvem | `v1/devices/me/rpc/response/{id}` | `true` |
