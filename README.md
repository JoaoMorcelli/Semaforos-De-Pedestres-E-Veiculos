# Semaforos-De-Pedestres-E-Veiculos
Repositório referente às aulas 07 e 08 de microcontroladores 2. O programa deve controlar o cruzamento entre duas vias (ruas), incluindo semáforos de veículos e de pedestres. Foi desenvolvido pelos alunos João Favato e João Carvalho.

## Requisitos de sistema 

Para a operação do sistema, são necessários um notebook, duas placas FRDM-KL25Z, e jumpers.

## Arquitetura de sistema

O projeto utiliza a arquitetura do platform.io com zephyr.

## Implementação do Semáforo de Pedestres

Foi implementado o semáforo de pedestres com mutex, fazendo com que as duas condições de ligamento do led nunca se sobreponham. Como foi solicitado, os leds são controlados por duas threads independentes, com o led verde ficando aceso por 4 segundos e o vermelho também. Abaixo está o teste realizado para comprovar o funcionamento desse semáforo:

### Teste Unitário
Pré-condição: A placa foi ligada e reiniciada pelo botão de “reset”, para garantir que
não houvesse nenhum lixo de memória.

Etapas:
1. Deixe as duas Threads com a mesma prioridade;
2. Defina 'SLEEP_TIME_VERDE' como 4000;
3. Defina 'SLEEP_TIME_VERMELHO' como 4000.

Pós-condição: Foi possível atestar a coloração certa dos leds bem como um intervalo de tempo adequado para os dois estados. Isso pode ser verificado no vídeo a seguir (o vídeo foi acelerado mas os intervalos de tempo estavam coerentes).

<img width="700" height="400" alt="TesteSemaforoPedestres" src="evidencias\TesteSemaforoPedestres.gif" />

## Implementação do Semáforo de Veículos

Conforme solicitado pelo enunciado, foram implementadas três threads, garantindo que os leds não se sobreponham. O comportamento do código é o seguinte: Verde acende por 3 segundos, amarelo acende por 1 segundo, vermelho acende por 4 segundos. Para validar o funcionamento dessa implementação, foi realizado o teste a seguir

### Teste Unitário 2
Pré-condição: A placa foi ligada e reiniciada pelo botão de “reset”, para garantir que
não houvesse nenhum lixo de memória.

Etapas:
1. Deixe as três Threads com a mesma prioridade;
2. Defina 'SLEEP_TIME_VERDE' como 3000;
3. Defina 'SLEEP_TIME_VERMELHO' como 4000;
4. Defina 'SLEEP_TIME_AMARELO' 1000.

Pós-condição: Verificou-se que os três leds apresentam a coloração correta, assim como o intervalo de tempo dos três estados está de acordo com o determinado. O vídeo a seguir evidencia esses aspectos.

<img width="700" height="400" alt="TesteSemaforoVeiculos" src="evidencias\TesteSemaforoVeiculos.gif" />


## Integração dos dois Semáforos

Para sincronizar os dois semáforos utilizando os leds das duas placas, foram necessárias diversas alterações no código. A primeira delas foi a utilização de semáforos na estrutura código em detrimento do mutex, pois dessa maneira foi possível criar uma lógica de troca de estados mais robusta. Em relação ao sincronismo das duas placas, usamos uma lógica em que existe uma placa 'mestre', que roda o código principal e liga os leds do semáforo de veículos, e uma placa 'escrava' que possui apenas um código para receber os sinais da placa mestre e ligar os leds do semáforo de pedestres. 

A fim de evidenciar o sincronismo das placas e o funcionamento dos dois semáforos simultaneamente, realizamos o teste a seguir.

### Teste de Integração
Pré-condição: A placa foi ligada e reiniciada pelo botão de “reset”, para garantir que
não houvesse nenhum lixo de memória.

Etapas:
1. Ligar o GND das duas placas através de um jumper;
2. Ligar os pinos PTD2 das duas placas;
3. Deixe as duas Threads com a mesma prioridade;
4. Defina 'SLEEP_TIME_VERDE' como 3000;
5. Defina 'SLEEP_TIME_VERMELHO' como 4000;
6. Defina 'SLEEP_TIME_AMARELO' como 1000.

Pós-condição: Foi possível atestar o sincronismo das duas placas, assim como os intervalos de tempo determinados. Isso pode ser observado no vídeo a seguir.

<img width="400" height="700" alt="TesteSincronizacaoPlacas" src="evidencias\TesteSincronizacaoPlacas.gif" />


## Modo Noturno

Para a implementação do modo noturno, foi necessário criar um sistema de escolha de modos (Normal ou Noturno), tanto no programa da placa mestre quanto no programa da placa escravo. Este sistema foi feito através de #if e #elif comparando a variável ModoAtual com os valores pré-definidos das variáveis ModoNormal e ModoNoturno.

### Teste de Validação
Pré-condição: Garantir que ambas as placas estejam com o programa atuando no mesmo modo de operação, caso contrário, não haverá erro de compilação, mas a lógica estaria errada.

Etapas:
1. Ligar o GND das duas placas através de um jumper;
2. Ligar os pinos PTD2 das duas placas;
3. Escolher o modo de operação do programa da placa mestre e dar Upload;
4. Escolhe o mesmo modo de operação para a placa escrava e dar upload;

Pós-condição: Foi possível observar a placa mestre piscar continuamente (de segundo em segundo) na cor amarela e a placa escrava piscar continuamente na cor vermelha.

https://github.com/user-attachments/assets/c9868c91-e950-483e-9b55-07698cbf437a

## Modo Botão (com interrupção)

Para a versão com botão e uso da interrupção, foi adicionado mais um modo ao sistema já existente no código. No denominado ModoBotao para a placa mestre, o semáforo de veículos começa como verde e o de pedestres como vermelho, permanecendo dessa maneira até que o botão é pressionado. 
Quando o botão é pressionado, o farol de veículos fica amarelo por um segundo, depois disso ele se torna vermelho e o de pedestres verde para permitir a travessia. Após o intervalo determinado na interrupção, os dois leds voltam para o estado de início e ficam assim até que haja nova interrupção. A placa escravo não precisou de um novo modo, já que continua se usando a mesma convenção do modo normal.

### Teste de Validação 2
Pré-condição: Garantir que ambas as placas estejam com o programa atuando no mesmo modo de operação, assim como garantir que a placa foi ligada e reiniciada pelo botão de “reset”, para garantir que
não houvesse nenhum lixo de memória.

Etapas:
1. Ligar o GND das duas placas através de um jumper;
2. Ligar os pinos PTD2 das duas placas;
3. Ligar o botão no pino PTA16 e no GND da placa mestre;
4. Defina 'TEMPO_DE_TRAVESSIA' como 5000

Pós condição: Foi verificado a adequação de sincronização das placas, da coloração dos leds e a interrupção gerada pelo botão se mostrou muito responsiva.

<img width="700" height="400" alt="TesteSemaforoVeiculos" src="evidencias\TesteModoBotao.gif" />

## Modo Normal para duas vias

Para o controle de duas vias, em outras palavras, um cruzamento entre ruas, utilizamos quatro semáforos para controlar o tráfego, dois de pedestres e dois de veículos. Como cada microcontrolador tem somente um LED interno, tivemos que utilizar cinco LEDs externos para simular outros dois semáforos (3 LEDs para o de veículos e 2 para o de pedestres). No código do programa, configuramos cinco portas do microcontrolador mestre para servirem como saídas para os LEDs.

### Teste de validação 3
Pré-condição: Garantir que a placa esteja sem lixo de memória, através do acionamento do botão "reset" do microcontrolador, além de confirmar a conexão correta entre as saídas do FRDM KL-25Z com os LEDS, caso contrário, os LEDs acenderiam em ordens erradas.

Etapas:
1. Ligar o GND das duas placas e dos LEDs através de jumpers;
2. Ligar os pinos PTD2 das duas placas;
3. Realizar as conexões corretas entre as saídas do microcontrolador (PTD4, PTA12, PTA13, PTC8, PTC9) e seus respectivos LEDs;
4. Escolher o modo de operação "Normal" e dar upload na placa mestre;
5. Escolher o mesmo modo de operação e dar upload na placa escrava;

Pós condição: Foi possível verificar o funcionamento correto dos semáforos deste cruzamento, na seguinte sequência;
- Um semáforo de veículos abre, juntamente com o semáforo de pedestres paralelo a ele;
- O mesmo semáforo de veículos passa pelo estado amarelo e, quando chega no vermelho, o outro semáforo de veículos abre, assim como o semáforo de pedestre paralelo a ele;
- Este segundo semáforo de veículo passa pelo estado amarelo e, quando chega no vermelho, o ciclo se reinicia.  

<img width="400" height="700" alt="TesteSincronizacaoPlacas" src="evidencias\ModoNormal2Vias.gif" />

## Modo Noturno para duas vias

Com o modo noturno para duas vias, basta manter as lógicas anteriores e fazer os leds externos piscarem da maneira pedida (o de pedestre vermelho e o de veículos amarelo).

### Teste de Validação 4
Pré-condição: Garantir que ambas as placas estejam com o programa atuando no mesmo modo de operação, caso contrário, não haverá erro de compilação, mas a lógica estaria errada.

Etapas:
1. Ligar o GND das duas placas e dos leds através de jumpers;
2. Ligar os pinos PTD2 das duas placas;
3. Ligar os pinos PTD4, PTA12, PTA13, PTC8, PTC9 aos seus respectivos leds;
4. Escolher o modo de operação noturno da placa mestre e dar Upload;
5. Escolhe o mesmo modo de operação para a placa escrava e dar upload;

Pós-condição: Foi possível observar a placa mestre piscar continuamente (de segundo em segundo) na cor amarela e a placa escrava piscar continuamente na cor vermelha, assim como os leds externos piscam adequadamente.

<img width="400" height="700" alt="TesteSincronizacaoPlacas" src="evidencias\ModoNoturno2Vias.gif" />

## Modo Botão para duas vias

No modo botão para duas vias, deixamos a lógica intacta para uma via estar sempre aberta e a outra sempre fechada, com os semáforos de pedestre acompanhando isso. Dessa maneira, quando o botão de travessia por partes for pressionado, será aberto o semáforo de pedestres e o da outra via por determinado tempo. 

### Teste de Validação 5
Pré-condição: Garantir que ambas as placas estejam com o programa atuando no mesmo modo de operação, caso contrário, não haverá erro de compilação, mas a lógica estaria errada.

Etapas:
1. Ligar o GND das duas placas e dos leds através de jumpers;
2. Ligar os pinos PTD2 das duas placas;
3. Ligar os pinos PTD4, PTA12, PTA13, PTC8, PTC9 aos seus respectivos leds;
4. Escolher o modo de operação botão da placa mestre e dar Upload;
5. Escolher o modo de operação normal da placa escrava e dar upload;

Pós-condição: Foi possível observar o funcionamento correto do botão e todos os semáforos responderam de forma adequada.

<img width="400" height="700" alt="TesteSincronizacaoPlacas" src="evidencias\ModoBotao2Vias.gif" />