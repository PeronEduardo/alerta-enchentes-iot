# ThingsBoard Cloud

## Configuração do dispositivo

1. Criar conta em https://thingsboard.cloud
2. **Entities → Devices → +** → nome: `estacao-rio-01`
3. Copiar o **Access Token** (aba *Manage credentials*) para o `wokwi/secrets.h`
4. (Segurança) Baixar o certificado CA em *Check connectivity* para trocar o `setInsecure()` no código

## Dashboard planejado

| Widget | Chave de telemetria |
|---|---|
| Gráfico de linha (série temporal) | `nivel_cm` |
| Medidor (gauge) | `nivel_cm` |
| Cartão de texto / indicador | `estado` |
| Cartões | `temperatura`, `umidade` |
| **Switch (RPC)**: ligar sirene | método `setSirene` (get: `getSirene`) |

Depois de pronto: exportar o dashboard (JSON) e salvar nesta pasta.
