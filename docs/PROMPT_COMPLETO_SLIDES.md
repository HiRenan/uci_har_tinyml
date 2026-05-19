# Prompt completo para gerar slides do projeto final com outra IA

Copie e cole o prompt abaixo em uma IA geradora de slides.

---

Crie uma apresentacao tecnica em portugues do Brasil, com visual moderno, limpo e profissional, para um projeto final da disciplina **IA Embarcada e Modelos Compactos**. A apresentacao deve ter entre **10 e 12 slides**, deve caber em **ate 10 minutos de fala** e deve ser adequada para avaliacao por professor/banca.

## Tema do projeto

**TinyML HAR: classificacao de atividade humana com ESP32-S3, MPU6050 e dataset publico UCI HAR**.

## Contexto da atividade

O enunciado do projeto final pede uma aplicacao de IA embarcada com as seguintes etapas:

- leitura/coleta de dados de sensores;
- treinamento de um modelo com dataset publico ou proprio;
- conversao/compressao do modelo para embarcar;
- desenvolvimento da pipeline de inferencia no dispositivo;
- demonstracao em hardware real ou simulado;
- codigo organizado em repositorio GitHub.

Este projeto usa um dos datasets publicos sugeridos pelo professor: **UCI Human Activity Recognition Using Smartphones (UCI HAR)**.

## Resumo tecnico do projeto

- Dataset: **UCI HAR**, publico, nao sintetico.
- Atividade: reconhecimento de atividade humana por sensores inerciais.
- Classes: `WALKING`, `WALKING_UPSTAIRS`, `WALKING_DOWNSTAIRS`, `SITTING`, `STANDING`, `LAYING`.
- Sensores no dataset original: acelerometro e giroscopio de smartphone.
- Subconjunto usado no modelo: aceleracao total triaxial `total_acc_x`, `total_acc_y`, `total_acc_z`.
- Hardware/simulacao: **ESP32-S3 no Wokwi**.
- Sensor no circuito: **MPU6050**, usado como acelerometro triaxial via I2C.
- Conexoes: SDA GPIO8, SCL GPIO9, VCC 3V3, GND; LED indicador no GPIO4 com resistor de 220 ohm.
- Framework: **ESP-IDF em C**.
- Janela de inferencia: **128 amostras** a **50 Hz**, equivalente a 2,56 s.
- Features por janela: **43 features estatisticas**.
- Modelo: **MLP 43-32-6**, com ReLU na camada oculta.
- Compressao: quantizacao dos pesos de `float32` para `int8`.
- Deploy: modelo exportado como header C em `main/har_model_int8.h`.
- Pipeline embarcada: MPU6050 -> janela de 128 amostras -> extracao de features -> normalizacao -> inferencia MLP int8 -> classe no monitor serial.

## Resultados do modelo

Inclua estes resultados nos slides:

- Amostras de treino: **7.352**.
- Amostras de teste: **2.947**.
- Features por janela: **43**.
- Acuracia float no conjunto de teste: **85,78%**.
- Acuracia com pesos quantizados int8 no conjunto de teste: **85,71%**.
- Reducao de memoria dos pesos: **75,00%**.
- Teste C nativo: passou para casos reais das seis classes.

## Estrutura obrigatoria dos slides

Crie os slides nesta ordem:

### Slide 1 - Titulo

Titulo: **TinyML HAR: Classificacao de Atividade Humana com ESP32-S3, MPU6050 e UCI HAR**.

Incluir:
- disciplina;
- integrantes;
- instituicao;
- frase curta: “Inferencia local em microcontrolador usando dataset publico e modelo compacto”.

### Slide 2 - Problema e motivacao

Explicar:
- o problema de reconhecer atividades humanas com sensores inerciais;
- aplicacoes: wearables, saude, seguranca, monitoramento de idosos, industria 4.0;
- por que fazer inferencia local: menor latencia, menor trafego de dados, funcionamento sem nuvem e maior privacidade.

### Slide 3 - Aderencia aos requisitos do projeto final

Criar uma tabela com duas colunas: requisito e como foi atendido.

Linhas:
- Dataset publico: UCI HAR.
- Sensor: MPU6050 no Wokwi.
- Treinamento: Python/scikit-learn.
- Compressao: quantizacao int8 dos pesos.
- Deploy: ESP32-S3 com ESP-IDF.
- Inferencia: leitura do sensor, features, normalizacao, MLP e saida serial.

### Slide 4 - Dataset publico UCI HAR

Mostrar:
- fonte: UCI Machine Learning Repository;
- 6 atividades;
- sensores inerciais;
- janelas de 128 amostras a 50 Hz;
- conjuntos de treino e teste;
- motivo da escolha: e um dos exemplos publicos sugeridos e combina com sensor inercial.

### Slide 5 - Relacao dataset x sensor embarcado

Explicar:
- UCI HAR foi coletado com smartphone, mas os sinais usados sao de aceleracao triaxial;
- o MPU6050 tambem possui acelerometro triaxial;
- por isso foi possivel criar uma pipeline equivalente em Wokwi;
- destacar a limitacao: a simulacao nao reproduz perfeitamente o posicionamento do smartphone na cintura.

### Slide 6 - Pre-processamento e features

Mostrar o fluxo:
- janela de 128 leituras;
- eixos X, Y, Z;
- calculo da magnitude;
- 43 features: media, desvio padrao, minimo, maximo, RMS, valor absoluto medio, amplitude, diferencas, taxa de cruzamento por zero e correlacoes.

Incluir um pequeno diagrama de bloco: `janela bruta -> features -> vetor de entrada`.

### Slide 7 - Modelo treinado

Mostrar:
- arquitetura MLP 43-32-6;
- 43 entradas;
- 32 neuronios ocultos com ReLU;
- 6 saidas;
- treinamento no computador;
- inferencia no ESP32-S3.

Incluir um desenho simples da rede neural.

### Slide 8 - Compressao do modelo

Explicar:
- pesos originalmente em float32;
- quantizacao para int8;
- objetivo: reduzir memoria e tornar o deploy mais leve;
- resultado: reducao de 75% nos pesos;
- impacto minimo na acuracia: 85,78% float contra 85,71% int8.

Incluir tabela:

| Versao | Acuracia | Observacao |
|---|---:|---|
| Float | 85,78% | modelo original |
| Int8 | 85,71% | modelo compactado |

### Slide 9 - Firmware embarcado

Mostrar a pipeline C/ESP-IDF:

1. inicializacao do I2C;
2. inicializacao do MPU6050;
3. coleta de 128 amostras a 50 Hz;
4. extracao de 43 features;
5. normalizacao;
6. inferencia MLP com pesos int8;
7. impressao da classe no monitor serial.

Citar arquivos principais:
- `main/main.c`;
- `main/har_feature_extraction.h`;
- `main/har_inference.h`;
- `main/har_model_int8.h`.

### Slide 10 - Circuito e demonstracao no Wokwi

Reservar espaco para prints:
- print do circuito Wokwi com ESP32-S3 e MPU6050;
- print do terminal com `Project build complete`;
- print do monitor serial com linhas `HAR TinyML` e classe prevista.

Incluir conexoes:
- MPU6050 SDA -> GPIO8;
- MPU6050 SCL -> GPIO9;
- VCC -> 3V3;
- GND -> GND;
- LED indicador -> GPIO4.

### Slide 11 - Validacao e resultados

Mostrar:
- dataset com 7.352 amostras de treino e 2.947 de teste;
- acuracia float 85,78%;
- acuracia int8 85,71%;
- teste C nativo passou com casos reais das seis classes;
- o header C usado no teste e o mesmo usado pelo firmware.

Incluir uma frase: “A validacao confirma que a conversao para C e a quantizacao preservaram o comportamento do modelo dentro do objetivo do projeto”.

### Slide 12 - Limitacoes, melhorias futuras e conclusao

Limitacoes:
- Wokwi simula o MPU6050, mas nao simula perfeitamente movimentos corporais reais;
- o dataset original usou smartphone na cintura;
- o modelo usa apenas aceleracao total, nao usa giroscopio nesta versao.

Melhorias futuras:
- incluir sinais de giroscopio;
- coletar dados reais com MPU6050 preso ao corpo;
- retreinar o modelo com dados do proprio hardware;
- testar TFLite Micro ou arquiteturas menores/mais robustas.

Conclusao:
- o projeto cumpre o fluxo completo de IA embarcada: dataset publico, treinamento, compressao, deploy e inferencia local no ESP32-S3.

## Estilo visual

- Usar cores azul, cinza, branco e detalhes em verde.
- Design limpo, tecnico e moderno.
- Evitar paragrafos longos.
- Usar bullets curtos.
- Usar diagramas de pipeline.
- Usar tabelas pequenas.
- Reservar espacos visuais para prints do VS Code/Wokwi.
- Incluir notas de fala curtas abaixo de cada slide.
- A apresentacao precisa ser compreensivel para quem conhece ESP32 e IA embarcada, mas nao conhece todos os detalhes do dataset.

## Observacao para a IA geradora

Nao invente resultados diferentes. Use exatamente os resultados informados acima. Nao diga que os dados foram coletados pelo aluno; diga que o treinamento usou dataset publico UCI HAR e que a leitura embarcada usa MPU6050 no Wokwi para demonstrar a pipeline de inferencia com sensor inercial.

---
