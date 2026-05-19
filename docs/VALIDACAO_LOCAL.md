# Validacao realizada localmente

Etapas validadas neste ambiente:

- Estrutura obrigatoria do projeto conferida.
- `diagram.json` validado como JSON e contendo ESP32-S3 + MPU6050.
- CSVs derivados do UCI HAR conferidos com 7.352 amostras de treino e 2.947 de teste.
- Build ESP-IDF executado com alvo `esp32s3`.
- `diagram.json`, `sdkconfig` e `build/flasher_args.json` conferidos como ESP32-S3.
- Imagens `bootloader.bin` e `esp32s3_uci_har_mpu6050.bin` conferidas com `esptool image-info`.
- CSVs, manifesto e dimensoes do modelo conferidos por script local.

Resultado esperado do teste C nativo, registrado em `tests/host_model_test_output.txt`:

```text
case=0 expected=WALKING predicted=WALKING logits=[5.37440,3.01608,-5.60289,-6.53319,-1.43891,-2.14628]
case=1 expected=WALKING_UPSTAIRS predicted=WALKING_UPSTAIRS logits=[0.76156,9.39099,-3.41776,-7.93759,-4.62658,-4.54933]
case=2 expected=WALKING_DOWNSTAIRS predicted=WALKING_DOWNSTAIRS logits=[-0.33576,2.04448,4.71670,-9.49166,-4.50508,-7.25560]
case=3 expected=SITTING predicted=SITTING logits=[-3.45013,-5.04204,-2.78779,6.62670,6.31220,-4.00782]
case=4 expected=STANDING predicted=STANDING logits=[-2.54711,-2.18638,-4.40230,4.02884,7.85594,-3.70742]
case=5 expected=LAYING predicted=LAYING logits=[-8.07552,-5.28716,-6.12273,-3.13522,-13.55131,9.58333]
OK: todos os casos de teste do modelo UCI HAR passaram.
```

Metricas principais:

- Acuracia float no teste: 85,78%.
- Acuracia quantizada no teste: 85,71%.
- Reducao dos pesos: 75,00%.

Limitacao atual: o teste C nativo nao foi reexecutado nesta maquina porque nao ha compilador C de host no PATH e o `bash.exe` aponta para um WSL sem `/bin/bash`. O build ESP-IDF do firmware foi executado e finalizou com `Project build complete`.
