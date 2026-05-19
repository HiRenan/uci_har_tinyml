# Projeto Final - TinyML HAR (ESP32-S3 + UCI HAR)

Implementacao de Human Activity Recognition (HAR) com TinyML embarcada em ESP32-S3 simulado no Wokwi, treinada no dataset publico UCI HAR.

## Objetivo

Classificar atividade humana (caminhando, subindo/descendo escadas, sentado, em pe, deitado) a partir de leituras de acelerometro triaxial, rodando inferencia int8 quantizada localmente no microcontrolador.

## Stack

- **ML:** Python (numpy, pandas, scikit-learn), MLP 43-32-6, quantizacao int8.
- **Embarcado:** C, ESP-IDF, ESP32-S3, MPU6050 via I2C.
- **Simulacao:** Wokwi.

## Roadmap

- [ ] Preparar dataset UCI HAR (features extraidas)
- [ ] Treinar MLP e exportar modelo quantizado
- [ ] Setup ESP-IDF + skeleton do firmware
- [ ] Driver MPU6050 + feature extraction no embarcado
- [ ] Engine de inferencia int8 + loop principal
- [ ] Simulacao Wokwi
- [ ] Testes de host e documentacao tecnica
- [ ] Apresentacao final

## Estrutura prevista

```
.
├── dataset/        # CSVs UCI HAR (features)
├── training/       # Script Python de treino e quantizacao
├── main/           # Firmware ESP32-S3
├── tests/          # Testes de host
├── docs/           # Documentacao tecnica
├── diagram.json    # Circuito Wokwi
└── wokwi.toml      # Config Wokwi
```

## Grupo

Trabalho desenvolvido em grupo no escopo da disciplina.
