**********************************
REVISAR FUNCAO VALIDA_LEITURAS
REVISAR ALIMMENTAÇAO DO SENSOR
**********************************

Usa sensor de ultrassom.

Tem servidor que responde a GETs: 

-IP/liquid, retorna a altura da coluna de liquido no reservatorio;

-IP/air, retorna a altura da coluna de ar no reservatório;

Tem suporte a Wifi usando ESP01 e rede via cabo usando Ethernet Shield

É configurável através de porta serial usando putty

Está sempre resetando sozinho (ainda nao implementado, melhor usar watchdog)