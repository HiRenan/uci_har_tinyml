# Dataset usado no projeto

Dataset publico escolhido: **UCI Human Activity Recognition Using Smartphones**.

Arquivos derivados incluidos nesta pasta:

- `uci_har_accel_features_train.csv`
- `uci_har_accel_features_test.csv`

Esses CSVs foram gerados a partir dos sinais publicos `total_acc_x`, `total_acc_y` e `total_acc_z` do UCI HAR. Cada linha representa uma janela de 128 amostras. Para cada janela foram extraidas 43 features estatisticas simples que tambem podem ser calculadas no ESP32-S3 a partir do MPU6050.

Classes:

1. WALKING
2. WALKING_UPSTAIRS
3. WALKING_DOWNSTAIRS
4. SITTING
5. STANDING
6. LAYING

Fonte oficial: https://archive.ics.uci.edu/dataset/240/human+activity+recognition+using+smartphones

Observacao: os CSVs nesta pasta sao derivados do dataset publico, nao sao dados sinteticos.
