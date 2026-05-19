# Projeto Final - TinyML HAR com ESP32-S3, MPU6050 e dataset publico UCI HAR

Este projeto remodela a proposta anterior com dataset sintetico para uma versao baseada em um dataset publico citado nas orientacoes da disciplina: **UCI Human Activity Recognition Using Smartphones (UCI HAR)**.

## Ideia

Classificar atividade humana a partir de uma janela de acelerometro. O modelo foi treinado com o dataset publico UCI HAR e embarcado em um ESP32-S3 simulado no Wokwi. O sensor usado no circuito e o **MPU6050**, conectado por I2C.

Classes do modelo:

1. WALKING
2. WALKING_UPSTAIRS
3. WALKING_DOWNSTAIRS
4. SITTING
5. STANDING
6. LAYING

## Por que este dataset?

O UCI HAR foi escolhido porque e um dos datasets publicos citados como exemplo no enunciado do projeto final. Ele foi construido com sensores inerciais de smartphone e possui acelerometro triaxial, giroscopio e rotulos de atividades humanas.

Nesta implementacao, o modelo usa o subconjunto de aceleracao total triaxial (`total_acc_x`, `total_acc_y`, `total_acc_z`) porque ele pode ser reproduzido no ESP32-S3 usando a parte de acelerometro do MPU6050.

Os CSVs incluidos em `dataset/` sao derivados diretamente do UCI HAR publico, nao sao dados sinteticos.

## Pipeline tecnico

1. Dataset publico UCI HAR.
2. Uso dos sinais `total_acc_x`, `total_acc_y`, `total_acc_z`.
3. Janelas de 128 amostras a 50 Hz.
4. Extracao de 43 features por janela.
5. Treinamento de MLP 43-32-6.
6. Quantizacao dos pesos para int8.
7. Exportacao do modelo para `main/har_model_int8.h`.
8. Leitura do MPU6050 no ESP32-S3.
9. Extracao das mesmas features no firmware.
10. Inferencia local e impressao da classe no monitor serial.

## Resultados do modelo

- Amostras de treino: **7.352**.
- Amostras de teste: **2.947**.
- Features por janela: **43**.
- Acuracia float no conjunto de teste: **85,78%**.
- Acuracia com pesos int8 no conjunto de teste: **85,71%**.
- Reducao de memoria dos pesos: **75,00%**.

## Circuito Wokwi

Conexoes principais:

| Componente | ESP32-S3 |
|---|---|
| MPU6050 VCC | 3V3 |
| MPU6050 GND | GND |
| MPU6050 SDA | GPIO8 |
| MPU6050 SCL | GPIO9 |
| LED indicador | GPIO4 + resistor 220 ohm para GND |

O LED e apenas um indicador visual de que a inferencia esta rodando. A classificacao completa aparece no monitor serial.

## Como compilar no VS Code com ESP-IDF

Abra a pasta raiz do projeto no VS Code:

```text
projeto_final_uci_har
```

No terminal ESP-IDF:

```powershell
idf.py set-target esp32s3
idf.py build
```

O print obrigatorio deve mostrar:

```text
Project build complete
```

## Como rodar no Wokwi

Depois do build:

```text
Ctrl + Shift + P
Wokwi: Start Simulator
```

Saida esperada no monitor serial:

```text
HAR TinyML | janela=0 | ax_mean=... ay_mean=... az_mean=... mag_std=... | logits=[...] | classe=LAYING
```

O MPU6050 do Wokwi inicia normalmente parado, entao a classe mais provavel no primeiro teste sera uma postura estatica, como `LAYING`, `SITTING` ou `STANDING`. Para demonstrar movimento, altere os atributos do MPU6050 no `diagram.json`, ou use o arquivo opcional `wokwi_mpu6050_demo.scenario.yaml` como roteiro de simulacao.

## Teste local do modelo em C

Em Linux/WSL/Git Bash:

```bash
./tests/run_host_test.sh
```

Resultado esperado:

```text
OK: todos os casos de teste do modelo UCI HAR passaram.
```

## Regerar o modelo pelo dataset publico

Para baixar o dataset oficial da UCI e treinar novamente:

```bash
pip install -r training/requirements.txt
python training/train_uci_har_accel_model.py --download
```

Para apenas reusar os CSVs derivados que ja estao no projeto:

```bash
python training/train_uci_har_accel_model.py --from-processed
```

Esses comandos recriam:

- `dataset/uci_har_accel_features_train.csv`;
- `dataset/uci_har_accel_features_test.csv`;
- `training/model_report.json`;
- `training/confusion_matrix_quantized_test.csv`;
- `training/classification_report_test.txt`;
- `main/har_model_int8.h`.

## Observacao tecnica importante

O UCI HAR foi coletado com smartphone na cintura. No Wokwi, o MPU6050 fornece leituras simuladas configuraveis. Por isso, a inferencia embarcada demonstra corretamente o pipeline tecnico de leitura, preprocessamento, features, modelo compactado e classificacao. Para obter equivalencia fisica perfeita, a melhoria futura seria coletar dados reais com um MPU6050 preso ao corpo e retreinar o modelo.
