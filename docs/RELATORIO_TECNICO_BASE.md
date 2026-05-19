# Relatorio tecnico base - Projeto Final TinyML HAR

## 1. Objetivo

Desenvolver uma aplicacao de IA embarcada para classificar atividades humanas com ESP32-S3, sensor inercial MPU6050 e modelo compacto treinado com dataset publico.

## 2. Dataset publico

Foi utilizado o dataset **UCI Human Activity Recognition Using Smartphones**. O dataset contem registros de sensores inerciais de smartphone para seis atividades: WALKING, WALKING_UPSTAIRS, WALKING_DOWNSTAIRS, SITTING, STANDING e LAYING.

O projeto usa os sinais publicos de aceleracao total triaxial (`total_acc_x`, `total_acc_y`, `total_acc_z`). Cada amostra do modelo corresponde a uma janela de 128 leituras, amostradas a 50 Hz no dataset original.

Os arquivos `dataset/uci_har_accel_features_train.csv` e `dataset/uci_har_accel_features_test.csv` foram derivados do dataset publico, portanto nao sao dados sinteticos.

## 3. Pre-processamento

Cada janela de 128 amostras foi convertida em 43 atributos estatisticos. Foram calculadas medidas como media, desvio padrao, minimo, maximo, RMS, valor absoluto medio, amplitude, diferenca media entre amostras, desvio das diferencas, taxa de cruzamento por zero e correlacoes entre eixos.

Essas features foram escolhidas porque sao simples de calcular em C no ESP32-S3 e mantem correspondencia direta com as leituras do acelerometro do MPU6050.

## 4. Modelo

O modelo e uma rede neural MLP com arquitetura 43-32-6:

- 43 entradas;
- 32 neuronios na camada oculta com ReLU;
- 6 saidas, uma para cada atividade.

O treinamento ocorre no computador. No ESP32-S3 ocorre apenas a inferencia.

## 5. Compressao

Os pesos das camadas foram quantizados de float32 para int8. As medias, escalas de normalizacao e biases foram mantidos em float32 para preservar estabilidade numerica. A reducao dos pesos foi de 75,00%.

## 6. Resultados

- Amostras de treino: 7.352.
- Amostras de teste: 2.947.
- Acuracia float no teste: 85,78%.
- Acuracia com pesos int8 no teste: 85,71%.
- Reducao dos pesos: 75,00%.

## 7. Deploy embarcado

O firmware para ESP32-S3 executa a seguinte pipeline:

1. Inicializa o barramento I2C.
2. Inicializa o MPU6050 no endereco 0x68.
3. Le 128 amostras de aceleracao em tres eixos.
4. Extrai as mesmas 43 features usadas no treinamento.
5. Normaliza as features.
6. Executa a MLP com pesos int8.
7. Imprime a classe prevista no monitor serial.

## 8. Testes realizados

Foram validadas as seguintes partes neste ambiente:

- estrutura obrigatoria do projeto;
- validade do `diagram.json`;
- treinamento a partir dos CSVs derivados do UCI HAR;
- exportacao do modelo para C;
- teste nativo em C usando o mesmo header `main/har_model_int8.h` usado pelo firmware.

A compilacao ESP-IDF final foi realizada com alvo `esp32s3` e gerou os artefatos usados pelo Wokwi.

## 9. Observacoes e limitacoes

Como o Wokwi inicializa o MPU6050 parado, o resultado no simulador tende a ser uma atividade estatica. Ainda assim, a pipeline completa de leitura do sensor, pre-processamento e inferencia local e executada no ESP32-S3 simulado.

Uma melhoria futura seria coletar uma base real com o proprio MPU6050 no corpo, usando o mesmo formato de janela do UCI HAR, e realizar um retreinamento especifico para o hardware final.
