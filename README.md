# FIAP - Faculdade de Informática e Administração Paulista

<p align="center">
<a href= "https://www.fiap.com.br/"><img src="assets/logo-fiap.png" alt="FIAP - Faculdade de Informática e Admnistração Paulista" border="0" width=40% height=40%></a>
</p>

#  FarmTech na era da cloud computing - Sprint 5 - Ir Além

## Nome do Grupo

- Arthur Luiz Rosado Alves -> RM562061
- Renan de Oliveira Mendes -> RM563145

### Link do vídeo: https://www.youtube.com/watch?v=hZPx_abSSwE

## Súmario
[1. Definição do sensor e contexto](#c1)

[2. Implementação no ESP32)](#c2)

[3. Armazenamento ou Visualização](#c3)

<br>

# <a name="c1"></a>1. Definição do sensor e contexto

Nosso Projeto utiliza sensores de baixo custo, aplicados a Internet das coisas voltado ao agronegócio.

  - DHT22: Sensor Digital que mede temperatura e umidade do ar com precisão. Esse sensor é digital pelo fato de realizar os processamentos internos dos sinais e já enviar os valores prontos em forma de bits, garatindo assim confiabilidade, simplicidade na integração de microcontroladores como o esp32.
  - Sensor Capacitive Soil: Sensor analógico que mede a variação da tensão elétrica conforme o nível de umidade do solo. Sendo assim um sensor Analógico pois apenas passa os volts para o esp32 processar esse valor em escala de "solo seco" ou "solo umido" na qual definimos no arduino ide os valores com base em experimentos sendo assim "3170" atribuido para solo seco e "1485" para solo umido.

No contexto de negócio, nosso projeto se destaca no agronegócio.

- A leitura em tempo real da umidade do solo permite automatizar sistemas de irrigação, reduzindo desperdicios de água e energia elétrica.
- ao correlacionar esses dados de temperatura, umidade do ar e do solo. É possivel criar modelos de machine learning capazes de antecipar condições de estresse hídrico ou doenças, permitindo maior autonomia ao produtor nas tomadas de decisões.
- produtores que utilizarem a nossa solução vao ter vantagens competitivas no mercado, pois vao reduzir seu custo operacional, aumentando a produtividade e assim se destacando no mercado.

# <a name="c2"></a>2. Implementação no ESP32

