**********************************
REVISAR FUNCAO VALIDA_LEITURAS
REVISAR ALIMMENTA�AO DO SENSOR
**********************************

Usa sensor de ultrassom.

Tem servidor que responde a GETs: 

-IP/liquid, retorna a altura da coluna de liquido no reservatorio;

-IP/air, retorna a altura da coluna de ar no reservat�rio;

Tem suporte a Wifi usando ESP01 e rede via cabo usando Ethernet Shield

� configur�vel atrav�s de porta serial usando putty

Est� sempre resetando sozinho (ainda nao implementado, melhor usar watchdog)