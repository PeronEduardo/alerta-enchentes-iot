# Packet Tracer: Rede do Bairro

Salvar aqui o arquivo `rede-bairro.pkt`.

## Topologia planejada

- **Casas 1, 2 e 3:** cada uma com
  - Home Gateway (Wi-Fi com **WPA2-PSK / AES**)
  - Sensor de nível de água + sirene (+ MCU-PT com script Python)
- **Rede da Defesa Civil:**
  - Servidor com o serviço **IoT** ativo (central de registro dos dispositivos)
  - PC/smartphone para acessar o painel de monitoramento
- **Roteadores** interligando as casas à central (representando a internet)

## Checklist

- [ ] Endereçamento IP definido (tabela abaixo)
- [ ] Home Gateways com WPA2-AES
- [ ] Dispositivos registrados no IoT Server
- [ ] Regras (Conditions) ou script Python no MCU-PT
- [ ] Prints para o relatório em `docs/imagens/`

| Rede | Faixa IP | Gateway |
|---|---|---|
| Casa 1 | | |
| Casa 2 | | |
| Casa 3 | | |
| Defesa Civil | | |
