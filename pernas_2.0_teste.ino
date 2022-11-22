/*
 ************************************************************************************
   Instituto Federal de Santa Catarina

   Projeto do Sistema Automático de Limpeza de Painéis Solares - AGNES

   Alunos: Gabriel Brognoli, Josué dos Santos e Yasmim Tezza
   Professores: Laércio, Fernando e Márcio

   <Descrição rápida do projeto>

   Componentes utilizados neste projeto:
   2 driverw tb6600
   2 motores de passo nema 23
   2 motores DC 12V 60 rpm
   1 compressor de bomba de parabrisa
   4 módulos de sensores infravermelhos
   2 módulos ponte H L298N
   1 arduino mega 2560
   LEDs, buzzer e botões

   Criado em 27 de Junho de 2022
   por Gabriel Brognoli

   <link do github>

    este código ainda está em desenvolvimento
 ************************************************************************************
*/

//bibliotecas:
#include <AccelStepper.h>
#include <AFMotor.h>
#include <LiquidCrystal_I2C.h>
#include <CuteBuzzerSounds.h>
#include <Sounds.h>

//sensores do robô:
#define dist_1 27 //Distância do sensor FRONTAL - 1
#define dist_2 28 //Distância do sensor TRASEIRO - 2
#define dist_3 29 //Distância do sensor ESQUERDO - 3
#define dist_4 30 //Distância do sensor DIREITO - 4

//rolos de limpeza
#define motor1_p 44 //Pino da saída positiva do motor 1
#define motor1_n 45 //Pino da saída negativa do motor 1
#define motor2_p 46 //Pino da saída positiva do motor 2
#define motor2_n 47 //Pino da saída negativa do motor 2

//compressor
#define comp_p 9 //Pino da saída positiva do compressor
#define comp_n 10 //Pino da saída negativa do compressor

//botão
#define botao 32 //Botão para reiniciar o ciclo de limpeza do robô
int last_state = 1; //Variavel para ler o ultimo estado do botão
int botao_l; //Variavel que le o estado do botao

//variáveis dos pinos dos robôs
int stepper1Pul = 3; //Pino que controla o andamento do stepper1
int stepper1Dir = 4; //Pino que controla a direção do stepper1
int stepper2Pul = 5; //Pino que controla o andamento do stepper2
int stepper2Dir = 6; //Pino que controla a direção do stepper2

//variáveis da baliza para a direita sentido esquerda->direita
int modo = 0;
int giro1_dir = 0;
int giro2_dir = 0;
int anda_dir = 0;
int para_dir = 0;
int para1_dir = 0;
int subida_dir = 0;
int descida_dir = 0;
int descida1_dir = 0;
int permite_dir = 1;
int inicio = 0;

//variáveis da baliza para a esquerda sentido esquerda->direita
int giro1_esq = 0;
int giro2_esq = 0;
int anda_esq = 0;
int para_esq = 0;
int para1_esq = 0;
int subida_esq = 0;
int subida1_esq = 0;
int descida_esq = 0;
int descida1_esq = 0;
int permite_esq = 1;

// variaveis para controle prático do robo:
#define sob_p 1000000 //controle de subida
#define des_p -1000000 //controle de descida
#define par_p 0 //controle de parada
#define accel 500 //controle de velocidade
#define maxspeed 100 //controle de velocidade máxima
int giro_dir1 = 100;
int giro_dir2 = 100;
int giro_dir3 = 100;
int giro_dir4 = 100;
int giro_esq1 = 100;
int giro_esq2 = 100;
int giro_esq3 = 100;
int giro_esq4 = 100;
int mov1_dir  = 100;
int mov2_dir  = 100;
int mov1_esq  = 100;
int mov2_esq  = 100;

//tela LCD
#define endereco  0x27 // endereços comuns: 0x27, 0x3F
#define colunas   16
#define linhas    2
LiquidCrystal_I2C lcd(endereco, colunas, linhas); //define o endereço do display i2c e indica quantas colunas e linhas o display possui
//variaveis de verificação para não travar o robô enquanto uso o LSD:
int i = 0;
int i2 = 0;
int i3 = 0;
int i4 = 0;
int i5 = 0;
int i6 = 0;
int i7 = 0;
int i8 = 0;
int i9 = 0;
int i10 = 0;

// motores de passo:

// criamos um objeto para cada stepper

AccelStepper stepper1(1, stepper1Pul, stepper1Dir);
AccelStepper stepper2(1, stepper2Pul, stepper2Dir);

//movimentação do robô:
//função de subir:
void sobe()
{
  stepper1.moveTo(sob_p);
  stepper2.moveTo(sob_p);
  return;
}
//função de descer:
void desce()
{
  stepper1.moveTo(des_p);
  stepper2.moveTo(des_p);
  return;
}
//função de parar:
void para()
{
  stepper1.setCurrentPosition(par_p); //zera a posição do stepper 1
  stepper2.setCurrentPosition(par_p); //zera a posição do stepper 2
  stepper1.stop(); //para o stepper1
  stepper2.stop(); //para o stepper2
  return;
}
//função de ligar o compressor
void aguinhaliga()
{
  digitalWrite(comp_p, HIGH);
  digitalWrite(comp_n, LOW);
}
//função de desligar o compressor
void aguinhadesliga()
{
  digitalWrite(comp_p, LOW);
  digitalWrite(comp_n, LOW);
}
//função de baliza para a direita
int baliza_dir()
{
  if (permite_dir == 1)//Permissão para fazer a baliza direita
  {
    if (giro1_dir == 0)//Se não girou ainda pela 1 vez
    {
      stepper1.moveTo(-giro_dir1); //gira pra um lado - giro_dir1
      stepper2.moveTo(giro_dir2); //gira pro outro - giro_dir2
      giro1_dir = 1;// indica que já girou pela 1 vez
    } else if (giro1_dir == 1 && anda_dir == 0 && stepper1.currentPosition() == -100 && stepper2.currentPosition() == 100)//Se já girou e não moveu ainda
    {
      stepper1.setCurrentPosition(0);
      stepper2.setCurrentPosition(0);
      stepper1.moveTo(mov1_dir); // mov1_dir
      stepper2.moveTo(mov2_dir); // mov2_dir
      anda_dir = 1;// indica que já andou depois de girar pela 1 vez
    } else if (giro1_dir == 1 && anda_dir == 1 && giro2_dir == 0 && stepper1.currentPosition() == 100 && stepper2.currentPosition() == 100)//Se já girou e moveu
    {
      stepper1.setCurrentPosition(0);
      stepper2.setCurrentPosition(0);
      stepper1.moveTo(giro_dir3); // giro_dir3
      stepper2.moveTo(-giro_dir4); // giro_dir4
      giro2_dir = 1;// indica que já girou pela 2 vez depois de andar
    } else if (giro2_dir == 1 && subida_dir ==  0 && stepper1.currentPosition() == 100 && stepper2.currentPosition() == -100)
    {
      desce();//Desce depois de finalizar a baliza para a esquerda
      descida_dir = 1;
    }
    if (descida_dir == 1 && para_dir == 0 && digitalRead(dist_2) && !digitalRead(dist_1) && digitalRead(dist_3) && digitalRead(dist_4))
    {
      para();//para depois de verificar o final da placa na descida
      para_dir = 1;
    }
    if (para_dir == 1 && subida_dir == 0 && stepper1.currentPosition() == 0 && stepper2.currentPosition() == 0)
    {
      sobe();//Sobe depois de chegar na final da placa
      subida_dir = 1;
    }
    if (subida_dir == 1 && para1_dir == 0 && !digitalRead(dist_2) && digitalRead(dist_1) && digitalRead(dist_3) && digitalRead(dist_4))
    {
      para();//Para depois de verificar o final da placa na subida e reseta as variaveis
      para1_dir = 1;
    }
    if (para1_dir == 1 && descida1_dir == 0 && stepper1.currentPosition() == 0 && stepper2.currentPosition() == 0)
    {
      desce();//Sobe depois de chegar na final da placa
      descida1_dir = 1;
      aguinhaliga();
    }
    if (descida1_dir == 1 && digitalRead(dist_2) && !digitalRead(dist_1) && digitalRead(dist_3) && digitalRead(dist_4))
    {
      para();//Para depois de verificar o final da placa na descida e reseta as variaveis
      giro1_dir = 0;
      giro2_dir = 0;
      anda_dir = 0;
      subida_dir = 0;
      descida1_dir = 0;
      descida_dir = 0;
      para_dir = 0;
      para1_dir = 0;
      aguinhadesliga();
    }
  }
  return;
}
//função de baliza para a esquerda
int baliza_esq()
{
  if (permite_esq == 1)//Permissão para fazer a baliza esquerda
  {
    if (giro1_esq == 0)//Se não girou ainda pela 1 vez
    {
      stepper1.moveTo(giro_esq1); //gira pra um lado - giro_dir1
      stepper2.moveTo(-giro_esq2); //gira pro outro - giro_dir2
      giro1_esq = 1;// indica que já girou pela 1 vez
    } else if (giro1_esq == 1 && anda_esq == 0 && stepper1.currentPosition() == 100 && stepper2.currentPosition() == -100)//Se já girou e não moveu ainda
    {
      stepper1.setCurrentPosition(0);
      stepper2.setCurrentPosition(0);
      stepper1.moveTo(mov1_esq); // mov1_dir
      stepper2.moveTo(mov2_esq); // mov2_dir
      anda_esq = 1;// indica que já andou depois de girar pela 1 vez
    } else if (giro1_esq == 1 && anda_esq == 1 && giro2_esq == 0 && stepper1.currentPosition() == 100 && stepper2.currentPosition() == 100)//Se já girou e moveu
    {
      stepper1.setCurrentPosition(0);
      stepper2.setCurrentPosition(0);
      stepper1.moveTo(-giro_esq3); // giro_dir3
      stepper2.moveTo(giro_esq4); // giro_dir4
      giro2_esq = 1;// indica que já girou pela 2 vez depois de andar
    } else if (giro2_esq == 1 && subida_esq ==  0 && stepper1.currentPosition() == -100 && stepper2.currentPosition() == 100)
    {
      desce();//Desce depois de finalizar a baliza para a esquerda
      descida_esq = 1;
    }
    else if (descida_esq == 1 && para_esq == 0 && digitalRead(dist_2) && !digitalRead(dist_1) && digitalRead(dist_3) && digitalRead(dist_4))
    {
      para();//para depois de verificar o final da placa na descida
      para_esq = 1;
    }
    if (para_esq == 1 && subida_esq == 0 && stepper1.currentPosition() == 0 && stepper2.currentPosition() == 0)
    {
      sobe();//Sobe depois de chegar na final da placa
      subida_esq = 1;
    }
    if (subida_esq == 1 && para1_esq == 0 && !digitalRead(dist_2) && digitalRead(dist_1) && digitalRead(dist_3) && digitalRead(dist_4))
    {
      para();//Para depois de verificar o final da placa na subida e reseta as variaveis
      para1_esq = 1;
    }
    if (para1_esq == 1 && descida1_esq == 0 && stepper1.currentPosition() == 0 && stepper2.currentPosition() == 0)
    {
      desce();//Sobe depois de chegar na final da placa
      descida1_esq = 1;
      aguinhaliga();
    }
    else if (descida1_esq == 1 && digitalRead(dist_2) && !digitalRead(dist_1) && digitalRead(dist_3) && digitalRead(dist_4))
    {
      para();//Para depois de verificar o final da placa na descida e reseta as variaveis
      giro1_esq = 0;
      giro2_esq = 0;
      anda_esq = 0;
      subida_esq = 0;
      descida_esq = 0;
      para_esq = 0;
      para1_esq = 0;
      descida1_esq = 0;
      aguinhadesliga();
    }
  }
  return;
}
//função da limpeza para a direita
void limpadir()
{
  baliza_dir();
  return;
}
//função da limpeza para a esquerda
void limpaesq()
{
  baliza_esq();
  return;
}
//função de ligar as escovas
void ligaesc()
{
  digitalWrite(motor1_p, HIGH);
  digitalWrite(motor1_n, LOW);
  digitalWrite(motor2_p, HIGH);
  digitalWrite(motor2_n, LOW);
  return;
}
//função de desligar as escovas
void desligaesc()
{
  digitalWrite(motor1_p, LOW);
  digitalWrite(motor1_n, LOW);
  digitalWrite(motor2_p, LOW);
  digitalWrite(motor2_n, LOW);
  return;
}
//setup:
void setup()
{
  Serial.begin(9600);
  //definição dos pinos:
  //sensores:
  pinMode(dist_1, INPUT);
  pinMode(dist_2, INPUT);
  pinMode(dist_3, INPUT);
  pinMode(dist_4, INPUT);

  //motores:
  pinMode(motor1_p, OUTPUT);
  pinMode(motor1_n, OUTPUT);
  pinMode(motor2_p, OUTPUT);
  pinMode(motor2_n, OUTPUT);
  pinMode(stepper1Dir, OUTPUT);
  pinMode(stepper1Pul, OUTPUT);
  pinMode(stepper2Dir, OUTPUT);
  pinMode(stepper2Pul, OUTPUT);
  pinMode(comp_p, OUTPUT);
  pinMode(comp_n, OUTPUT);

  //botão:
  pinMode(botao, INPUT_PULLUP);

  //settando aceleração e velocidade maxima dos steppers:
  stepper1.setMaxSpeed(maxspeed);
  stepper1.setAcceleration(accel);
  stepper2.setMaxSpeed(maxspeed);
  stepper2.setAcceleration(accel);
  
  //zerando os pinos do driver tb6600:
  digitalWrite(stepper1Dir, LOW);
  digitalWrite(stepper1Pul, LOW);
  digitalWrite(stepper2Dir, LOW);
  digitalWrite(stepper2Pul, LOW);
  stepper2.setPinsInverted(1, 0, 1); //invertendo os pinos do stepper2 p/ as rodas girarem corretamente

  //setup do LSD:
  lcd.init(); // INICIA A COMUNICAÇÃO COM O DISPLAY
  lcd.backlight(); // LIGA A ILUMINAÇÃO DO DISPLAY
  lcd.clear(); // LIMPA O DISPLAY
  lcd.setCursor(0, 0); // POSICIONA O CURSOR NA PRIMEIRA COLUNA DA LINHA 1
  lcd.print("Estado atual: ");

}//end setup
void loop()
{
  //settando os sensores:
  int distsen1 = digitalRead(dist_1); //Distância do sensor FRONTAL
  int distsen2 = digitalRead(dist_2); //Distância do sensor TRASEIRO
  int distsen3 = digitalRead(dist_3); //Distância do sensor ESQUERDO
  int distsen4 = digitalRead(dist_4); //Distância do sensor DIREITO
  //Serial.print(distsen1);
  //Serial.print("; ");
  //Serial.print(distsen2);
  //Serial.print("; ");
  //Serial.print(distsen3);
  //Serial.print("; ");
  //Serial.print(distsen4);
  //Serial.println("; ");

  // condições para o robô se posicionar na placa:
  if (modo == 0)//Modo 0 -> Posicionamento no canto superior da placa
  {
    if (distsen1 && distsen2 && distsen3 && !distsen4) // condição 3.1 -> sobe(1110)
    {
      sobe(); //robô sobe a placa
      stepper1.run();
      stepper2.run();
      if (i == 0)
      {
        lcd.setCursor(0, 1);
        lcd.print("Subindo    - 3.1");
        i = 1; i2 = 0; i3 = 0; i4 = 0; i5 = 0; i6 = 0; i7 = 0; i8 = 0; i9 = 0; i10 = 0;
      }
    } else if (distsen1 && distsen2 && !distsen3 && distsen4) // condição 3.2 -> sobe(1101)
    {
      sobe(); //robô sobe a placa
      stepper1.run();
      stepper2.run();
      if (i2 == 0)
      {
        lcd.setCursor(0, 1);
        lcd.print("Subindo    - 3.2");
        i = 0; i2 = 1; i3 = 0; i4 = 0; i5 = 0; i6 = 0; i7 = 0; i8 = 0; i9 = 0; i10 = 0;
      }
    } else if (distsen1 && distsen2 && !distsen3 && !distsen4) //condição 3.3(1100)
    {
      sobe(); //robô sobe a placa
      stepper1.run();
      stepper2.run();
      if (i3 == 0)
      {
        lcd.setCursor(0, 1);
        lcd.print("Subindo    - 3.3");
        i = 0; i2 = 0; i3 = 1; i4 = 0; i5 = 0; i6 = 0; i7 = 0; i8 = 0; i9 = 0; i10 = 0;
      }
    } else if (!distsen1 && distsen2 && distsen3 && !distsen4) //condição 3.4(0110)
    {
      sobe(); //robô sobe a placa
      stepper1.run();
      stepper2.run();
      if (i4 == 0)
      {
        lcd.setCursor(0, 1);
        lcd.print("Limpeza Direita ");
        i = 0; i2 = 0; i3 = 0; i4 = 1; i5 = 0; i6 = 0; i7 = 0; i8 = 0; i9 = 0; i10 = 0;
      }
    } else if (!distsen1 && distsen2 && !distsen3 && distsen4) //condição 3.5(0101)
    {
      sobe(); //robô sobe a placa
      stepper1.run();
      stepper2.run();
      if (i5 == 0)
      {
        lcd.setCursor(0, 1);
        lcd.print("Limpeza Esquerda");
        i = 0; i2 = 0; i3 = 0; i4 = 0; i5 = 1; i6 = 0; i7 = 0; i8 = 0; i9 = 0; i10 = 0;
      }
    } else if (!distsen1 && distsen2 && !distsen3 && !distsen4) //condição 3.6(0100)
    {
      sobe(); //robô sobe a placa
      stepper1.run();
      stepper2.run();

      if (i6 == 0)
      {
        lcd.setCursor(0, 1);
        lcd.print("Subindo    - 3.6");
        i = 0; i2 = 0; i3 = 0; i4 = 0; i5 = 0; i6 = 1; i7 = 0; i8 = 0; i9 = 0; i10 = 0;
      }
    } else if (distsen1 && !distsen2 && distsen3 && !distsen4) //condição 2.1 -> desce(1010) - O robo está no canto superior direito da placa
    {
      modo = 2; // Inicia a limpeza para a esquerda
      stepper1.setCurrentPosition(0); //zera a posição do stepper 1
      stepper2.setCurrentPosition(0); //zera a posição do stepper 2
      if (i7 == 0)
      {
        lcd.setCursor(0, 1);
        lcd.print("Descendo   - 2.1");
        i = 0; i2 = 0; i3 = 0; i4 = 0; i5 = 0; i6 = 0; i7 = 1; i8 = 0; i9 = 0; i10 = 0;
      }
    } else if (distsen1 && !distsen2 && !distsen3 && distsen4) //condição 2.2 -> desce(1001) - O robo está no canto superior esquerdo da placa
    {
      modo = 1; // Inicia a limpeza para a direita
      stepper1.setCurrentPosition(0); //zera a posição do stepper 1
      stepper2.setCurrentPosition(0); //zera a posição do stepper 2
      if (i8 == 0)
      {
        lcd.setCursor(0, 1);
        lcd.print("Descendo   - 2.2");
        i = 0; i2 = 0; i3 = 0; i4 = 0; i5 = 0; i6 = 0; i7 = 0; i8 = 1; i9 = 0; i10 = 0;
      }
    } else
    {
      para(); //robô para de subir
      if (i10 == 0)
      {
        lcd.setCursor(0, 1);
        lcd.print("Parado          ");
        i = 0; i2 = 0; i3 = 0; i4 = 0; i5 = 0; i6 = 0; i7 = 0; i8 = 0; i9 = 0; i10 = 1;
      }
    }
  }
  if (modo == 1) //modo 1 -> Inicia a limpeza para a direita
  {
    permite_esq = 0;
    permite_dir = 1;
    ligaesc(); //liga as escovas de limpeza
    if (inicio == 0) //desce quando chega na posição ideal de inicio, mas só na primeira vez
    {
      desce(); //robô desce a placa
      aguinhaliga(); //liga o compressor de agua
      stepper1.run();
      stepper2.run();
      if (!distsen1)
      {
        para();  //robô para
        aguinhadesliga(); //desliga o compressor de agua
        inicio = 1;
      }
    } else if (inicio == 1)
    {
      limpadir(); //faz a curva para a direita em baixo
      stepper1.run();
      stepper2.run();
      if (distsen1 && !distsen2 && distsen3 && !distsen4)
      {
        modo = 3; //se chegar nessa parte da placa ele vai para o modo 3
      }
    }
  }
  if (modo == 2) //modo 2 -> Inicia a limpeza para a esquerda
  {
    permite_dir = 0;
    permite_esq = 1;
    ligaesc(); //liga as escovas de limpeza
    if (inicio == 0)
    {
      desce(); //robô desce a placa
      aguinhaliga(); //liga o compressor de agua
      stepper1.run();
      stepper2.run();
      if (!distsen1)
      {
        para(); //robô para
        aguinhadesliga(); //desliga o compressor de agua
        inicio = 1;
      }
    } else if (inicio == 1)
    {
      limpaesq(); //limpa a placa da direita para a esquerda
      stepper1.run();
      stepper2.run();
      if (distsen1 && !distsen2 && !distsen3 && distsen4)
      {
        modo = 3; //se chegar nessa parte da placa ele vai para o modo 3
      }
    }
  }
  if (modo == 3) //modo 3 -> Finaliza a limpeza, desliga tudo e reinicia quando apertar um botão
  {
    aguinhadesliga(); //desliga o compressor de agua
    desligaesc(); // desliga as escovas de limpeza
    botao_l = digitalRead(botao);
    inicio = 0;
    if(!botao_l) //se o botão for 1 depois de terminar tudo ele reinicia e volta para o modo 0
    {
      modo = 0;
    }
  }
}// end loop