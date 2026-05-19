# Guia rapido para executar no VS Code

1. Abra a pasta raiz do projeto no VS Code: `projeto_final_uci_har`.
2. Abra o terminal ESP-IDF.
3. Rode:

```powershell
idf.py set-target esp32s3
idf.py build
```

4. Tire print do terminal com `Project build complete`.
5. Execute `Wokwi: Start Simulator`.
6. Tire print do circuito e do monitor serial.

Se aparecer erro relacionado ao caminho do projeto, feche o VS Code e abra diretamente a pasta raiz do projeto, nao a pasta acima dela.
