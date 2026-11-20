# 🌊 Sistema IoT de Alerta de Enchentes

Projeto Final da disciplina **Internet das Coisas (IoT)**: Engenharia da Computação, UNIVAP
Prof. Wagner dos Santos Clementino de Jesus. Entrega em **24/11/2026**.

**Grupo:** Diogo Gil de Oliveira · Eduardo Peron · Gustavo Albano Nunes

---

## 📌 Sobre o projeto

Estação de monitoramento do nível de um rio para alertar moradores de bairros ribeirinhos.
O sistema é **simulado** no [Wokwi](https://wokwi.com) (ESP32) e conectado a uma nuvem real, o
[ThingsBoard Cloud](https://thingsboard.cloud).

```
 ┌───────── WOKWI (estação no rio) ─────────┐        ┌────────── THINGSBOARD CLOUD ──────────┐
 │ HC-SR04 + DHT22 → ESP32                   │        │ Broker MQTT + armazenamento           │
 │  → calcula o nível → classifica o risco   │ ─────► │  → Dashboard em tempo real            │
 │  → LEDs + sirene (ação local, sem rede)   │ MQTT/  │  → Regra de alarme (EMERGÊNCIA)       │
 │                                           │ TLS    │  → Alarmes e notificações             │
 │  ◄── comando remoto "setSirene" (RPC) ────│ ◄───── │  → Switch "Sirene remota"             │
 └───────────────────────────────────────────┘        └───────────────────────────────────────┘
```

Ciclo da IoT: **Percepção** (sensores) → **Transmissão** (Wi-Fi + MQTT/TLS) →
**Análise** (ESP32 na borda + regra de alarme na nuvem) → **Ação** (LEDs, sirene local e remota, alarmes).

## 📁 Estrutura

```
alerta-enchentes-iot/
├── wokwi/              # Firmware do ESP32 (C++/Arduino) + circuito
│   ├── sketch.ino
│   ├── diagram.json
│   ├── libraries.txt
│   ├── ca_thingsboard.h      # certificado CA do servidor (público)
│   └── secrets_exemplo.h     # modelo de credenciais (copiar para secrets.h)
├── thingsboard/        # Export JSON do dashboard
└── docs/
    ├── arquitetura.md  # Requisitos, lógica e decisões de projeto
    ├── imagens/        # Prints para o relatório
    └── relatorio/      # Relatório no modelo INIC
```

## ▶️ Como executar a simulação

1. Crie um projeto **ESP32** em https://wokwi.com.
2. Copie `sketch.ino`, `diagram.json`, `libraries.txt` e `ca_thingsboard.h` para arquivos de mesmo nome.
3. Crie um arquivo `secrets.h` com o conteúdo de `secrets_exemplo.h` e cole o **Access Token** do dispositivo no ThingsBoard.
4. Clique em ▶️ e **clique no sensor HC-SR04** para mudar a distância e simular a subida do rio.
5. ⚠️ Mantenha a aba do Wokwi **visível**: o navegador congela abas em segundo plano e a simulação para.

## 🚦 Estados de risco

| Nível do rio | Estado | LED | Envio para a nuvem |
|---|---|---|---|
| < 200 cm | NORMAL | 🟢 | a cada 60 s |
| 200 a 280 cm | ATENÇÃO | 🟡 | a cada 15 s |
| 280 a 340 cm | ALERTA | 🔴 | a cada 5 s |
| ≥ 340 cm | EMERGÊNCIA | 🔴 + sirene + alarme crítico na nuvem | a cada 2 s |

## 🔒 Segurança

- MQTT sobre **TLS** (porta 8883) e **token** individual por dispositivo.
- O `secrets.h` (com o token) está no `.gitignore` e **nunca** vai para o GitHub.
- Verificação do certificado do servidor configurável em `VERIFICAR_CERTIFICADO`
  (1 = recomendado em hardware real; 0 = usado no Wokwi, onde a verificação é lenta demais
  e o servidor encerra a conexão, erro mbedTLS -0x0050).

## ✅ Andamento

- [x] Firmware: leitura, classificação, LEDs/sirene, MQTT/TLS, reconexão, RPC
- [x] Teste no Wokwi com o ThingsBoard
- [x] Certificado CA + chave de verificação TLS
- [x] Dashboard (gráfico, tanque, estado, temperatura, umidade, sirene remota)
- [x] Regra de alarme `EnchenteEmergencia` + tabela de alarmes
- [x] Relatório INIC (rascunho v2)
- [ ] Revisão do relatório pelo grupo
- [ ] Vídeo de demonstração
- [ ] Apresentação
