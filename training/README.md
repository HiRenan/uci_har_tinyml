# Treinamento

O projeto usa o dataset publico **UCI Human Activity Recognition Using Smartphones**.

Ha duas formas de reproduzir o treinamento:

## 1. Reusar os CSVs processados incluidos no projeto

Funciona sem baixar o ZIP oficial novamente:

```bash
pip install -r training/requirements.txt
python training/train_uci_har_accel_model.py --from-processed
```

Esse comando usa:

- `dataset/uci_har_accel_features_train.csv`
- `dataset/uci_har_accel_features_test.csv`

## 2. Baixar o dataset oficial da UCI e reconstruir tudo

Requer internet:

```bash
pip install -r training/requirements.txt
python training/train_uci_har_accel_model.py --download
```

O script baixa o dataset oficial da UCI, extrai os sinais de acelerometro `total_acc_x/y/z`, calcula 43 features por janela, treina uma rede MLP 43-32-6, quantiza os pesos para int8 e regrava `main/har_model_int8.h`.

## Resultados obtidos nesta versao

- Acuracia float no teste: 0.8578
- Acuracia com pesos int8 no teste: 0.8571
- Reducao dos pesos: 75.00%

Arquivos de saida:

- `model_report.json`
- `confusion_matrix_test.csv`
- `confusion_matrix_quantized_test.csv`
- `classification_report_test.txt`
- `retrain_from_processed_output.txt`
