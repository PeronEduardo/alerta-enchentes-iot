# Arquitetura e Requisitos

## Ciclo da IoT aplicado (Percepção → Transmissão → Análise → Ação)

| Etapa | No projeto |
|---|---|
| Percepção | HC-SR04 mede a distância até a água; DHT22 mede temperatura e umidade |
| Transmissão | Wi-Fi + MQTT sobre TLS (porta 8883) até o ThingsBoard Cloud |
| Análise | ESP32 converte distância → nível e classifica o risco (Edge Computing); regra de alarme `EnchenteEmergencia` na nuvem |
| Ação | LEDs e sirene local; sirene acionada remotamente pelo dashboard; alarmes e notificações |

## Processamento dos dados

1. **Distância:** `d = (t × 0,0343) / 2` (t em µs, d em cm; divide por 2 porque o som vai e volta)
2. **Nível:** `nível = H − d`, com H = 400 cm (altura do sensor em relação ao fundo do rio)
   - Exemplo: d = 150 cm → nível = 400 − 150 = **250 cm** → estado **ATENÇÃO**
3. **Classificação:** NORMAL < 200 ≤ ATENÇÃO < 280 ≤ ALERTA < 340 ≤ EMERGÊNCIA

## Regra de alarme (ThingsBoard)

| Item | Configuração |
|---|---|
| Tipo | `EnchenteEmergencia`, severidade **Critical** |
| Argumento | `estado` (último valor de telemetria) |
| Disparo | `estado` igual a `EMERGENCIA` |
| Encerramento | `estado` diferente de `EMERGENCIA` |

## Requisitos Funcionais

| RF | Descrição | Implementação |
|---|---|---|
| RF01 | Coleta de dados | HC-SR04 (nível) + DHT22 (temperatura e umidade) |
| RF02 | Processamento | Cálculo de nível, classificação de risco, JSON |
| RF03 | Conectividade Wi-Fi | ESP32 conectado à rede Wi-Fi |
| RF04 | Comunicação com a nuvem | ThingsBoard Cloud via MQTT sobre TLS |
| RF05 | Visualização | Dashboard ThingsBoard em tempo real, com tabela de alarmes |
| RF06 | Atuação remota | RPC `setSirene` aciona a sirene do ESP32 |

## Requisitos Não Funcionais

| RNF | Descrição | Implementação |
|---|---|---|
| RNF01 | Escalabilidade | Nova estação = novo dispositivo no ThingsBoard, sem alterar o restante |
| RNF02 | Confiabilidade | Reconexão automática ao Wi-Fi e ao MQTT; ação local funciona sem internet |
| RNF03 | Segurança | TLS na porta 8883; token por dispositivo; token fora do GitHub; verificação do certificado configurável (`VERIFICAR_CERTIFICADO`) |
| RNF04 | Usabilidade | Estados por cor e por nome; um único painel |
| RNF05 | Eficiência energética | Intervalo de envio adaptativo ao risco (60 s a 2 s) |

## Protocolo de comunicação (ThingsBoard)

| Direção | Tópico MQTT | Exemplo de mensagem |
|---|---|---|
| ESP32 → nuvem | `v1/devices/me/telemetry` | `{"nivel_cm":250.0,"estado":"ATENCAO",...}` |
| nuvem → ESP32 | `v1/devices/me/rpc/request/{id}` | `{"method":"setSirene","params":true}` |
| ESP32 → nuvem | `v1/devices/me/rpc/response/{id}` | `true` |
