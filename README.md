# 🌊 Sistema IoT de Alerta de Enchentes

Projeto Final da disciplina **Internet das Coisas (IoT)**: Engenharia da Computação, UNIVAP
Prof. Wagner dos Santos Clementino de Jesus. Entrega em **24/11/2026**.

**Grupo:** Eduardo Peron · Diogo Gil de Oliveira · Gustavo Albano Nunes

---

## 📌 Sobre o projeto

Sistema de monitoramento do nível de um rio para alertar moradores de bairros ribeirinhos.
O projeto é **simulado** em duas camadas complementares:

| Camada | Ferramenta | O que demonstra |
|---|---|---|
| **Estação de monitoramento do rio** | [Wokwi](https://wokwi.com) (ESP32) + [ThingsBoard Cloud](https://thingsboard.cloud) | Coleta, processamento, MQTT/TLS, nuvem, dashboard e atuação remota |
| **Rede do bairro** | Cisco Packet Tracer | Home Gateways, gerenciamento de dispositivos, escalabilidade e segurança Wi-Fi (WPA2-AES) |

```
 ┌──────────── WOKWI (nuvem real) ────────────┐
 │ ESP32 + HC-SR04 + DHT22 + LEDs + sirene     │
 │  → calcula nível → classifica risco         │
 │  → MQTT/TLS (8883) → ThingsBoard Cloud      │
 │  ← comando remoto "setSirene"               │
 └─────────────────────────────────────────────┘
 ┌──────── PACKET TRACER (rede do bairro) ─────┐
 │ Casas 1..N: Home Gateway + sensor + sirene  │
 │ Central da Defesa Civil: IoT Server + PC    │
 └─────────────────────────────────────────────┘
```

## 📁 Estrutura

```
alerta-enchentes-iot/
├── wokwi/              # Firmware do ESP32 (C++/Arduino) + circuito
│   ├── sketch.ino
│   ├── diagram.json
│   ├── libraries.txt
│   └── secrets_exemplo.h   # modelo de credenciais (copiar para secrets.h)
├── packet-tracer/      # Arquivo .pkt da rede do bairro
├── thingsboard/        # Configuração do dispositivo e dashboard (export JSON)
└── docs/
    ├── arquitetura.md  # Requisitos, lógica e decisões de projeto
    ├── imagens/        # Prints para o relatório
    └── relatorio/      # Relatório no modelo INIC
```

## ▶️ Como executar a simulação no Wokwi

1. Crie um projeto **ESP32** em https://wokwi.com.
2. Copie o conteúdo de `sketch.ino`, `diagram.json` e `libraries.txt` para os arquivos de mesmo nome.
3. Crie um arquivo `secrets.h` (botão de novo arquivo) com o conteúdo de `secrets_exemplo.h` e cole o **Access Token** do ThingsBoard.
4. Clique em ▶️ e **clique no sensor HC-SR04** para mudar a distância e simular a subida do rio.

## 🚦 Estados de risco

| Nível do rio | Estado | LED | Envio para a nuvem |
|---|---|---|---|
| < 200 cm | NORMAL | 🟢 | a cada 60 s |
| 200 a 280 cm | ATENÇÃO | 🟡 | a cada 15 s |
| 280 a 340 cm | ALERTA | 🔴 | a cada 5 s |
| ≥ 340 cm | EMERGÊNCIA | 🔴 + sirene | a cada 2 s |

## ✅ Andamento

- [x] Estrutura do repositório
- [x] Firmware inicial: leitura, classificação, LEDs/sirene, MQTT/TLS, reconexão, RPC
- [ ] Testar o firmware no Wokwi com o token real
- [ ] Trocar `setInsecure()` pelo certificado CA do ThingsBoard
- [ ] Dashboard no ThingsBoard (gráfico, medidor, estado, botão da sirene)
- [ ] Regras de alarme no ThingsBoard
- [ ] Topologia no Packet Tracer (3 casas + Defesa Civil)
- [ ] Script Python no MCU-PT
- [ ] Relatório INIC (3 a 6 páginas)
- [ ] Vídeo de demonstração
