# Validacao tecnica executada

Validacoes executadas antes da entrega do pacote:

## 1. Dataset e metricas

Arquivo verificado: `training/model_report.json`.

Resultados principais:

- Dataset: UCI Human Activity Recognition Using Smartphones.
- Amostras de treino: 7352.
- Amostras de teste: 2947.
- Janela: 128 amostras.
- Frequencia original: 50 Hz.
- Features por janela: 43.
- Modelo: MLP 43-32-6, ReLU, pesos quantizados em int8.
- Acuracia float no teste: 85,78%.
- Acuracia quantizada no teste: 85,71%.
- Reducao de memoria dos pesos: 75,00%.

## 2. CSVs de features

Arquivos verificados:

- `dataset/uci_har_accel_features_train.csv`: 7352 linhas, 45 colunas.
- `dataset/uci_har_accel_features_test.csv`: 2947 linhas, 45 colunas.

As 45 colunas sao: `label_id`, `label_name` e 43 features numericas extraidas de janelas de aceleracao.

## 3. Circuito Wokwi

Arquivo verificado: `diagram.json`.

Resultado: JSON valido.

Ligacoes principais:

- MPU6050 VCC -> ESP32-S3 3V3.
- MPU6050 GND -> ESP32-S3 GND.
- MPU6050 SDA -> GPIO8.
- MPU6050 SCL -> GPIO9.

## 4. Teste C nativo do modelo

Comando executado:

```bash
./tests/run_host_test.sh
```

Saida obtida:

```text
case=0 expected=WALKING predicted=WALKING logits=[5.37440,3.01608,-5.60289,-6.53319,-1.43891,-2.14628]
case=1 expected=WALKING_UPSTAIRS predicted=WALKING_UPSTAIRS logits=[0.76156,9.39099,-3.41776,-7.93759,-4.62658,-4.54933]
case=2 expected=WALKING_DOWNSTAIRS predicted=WALKING_DOWNSTAIRS logits=[-0.33576,2.04448,4.71670,-9.49166,-4.50508,-7.25560]
case=3 expected=SITTING predicted=SITTING logits=[-3.45013,-5.04204,-2.78779,6.62670,6.31220,-4.00782]
case=4 expected=STANDING predicted=STANDING logits=[-2.54711,-2.18638,-4.40230,4.02884,7.85594,-3.70742]
case=5 expected=LAYING predicted=LAYING logits=[-8.07552,-5.28716,-6.12273,-3.13522,-13.55131,9.58333]
OK: todos os casos de teste do modelo UCI HAR passaram.
```

## 5. Build ESP-IDF

A compilacao ESP-IDF completa foi executada com o alvo `esp32s3`:

```powershell
idf.py set-target esp32s3
idf.py build
```

Resultado obtido: `Project build complete`.

Tambem foram verificadas as imagens geradas:

- `build/bootloader/bootloader.bin`: tipo ESP32-S3, checksum valido.
- `build/esp32s3_uci_har_mpu6050.bin`: tipo ESP32-S3, checksum valido.

Evidencia visual disponivel em `WokWiStart.png`: simulador Wokwi em execucao com ESP32-S3, MPU6050, LED indicador aceso e monitor serial mostrando linhas `HAR TinyML` com `classe=LAYING`.
