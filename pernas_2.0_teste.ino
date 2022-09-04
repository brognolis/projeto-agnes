/*
 ************************************************************************************
   Instuto Federal de Santa Catarina

   Projeto do Sistema Automático de Limpeza de Painéis Solares - AGNES

   Alunos: Gabriel Brognoli, Josué dos Santos e Yasmim Tezza
   Professores:

   <Descrição rápida do projeto>

   Componentes utilizados neste projeto:
   1 motor shield L293D
   2 motores de passo nema 23
   2 motores DC 5V 200 rpm
   1 compressor de bomba de parabrisa
   4 módulos de sensores infravermelhos
   2 módulos ponte H L298N
   1 arduino mega 2560
   LEDs, buzzer e resistores

   Criado em 27 de Junho de 2022
   por Gabriel Brognoli

   <link do github>

    este código ainda está em desenvolvimento - *lembrar de usar switch/case nos modos
 ************************************************************************************
*/

//bibliotecas:
#include <AccelStepper.h>
#include <AFMotor.h>
#include <LiquidCrystal_I2C.h>
#include <CuteBuzzerSounds.h>
#include <Sounds.h>

//sensores do robô:
#define dist_1 24 //Distância do sensor FRONTAL - 1
#define dist_2 26 //Distância do sensor TRASEIRO - 2
#define dist_3 28 //Distância do sensor ESQUERDO - 3
#define dist_4 30 //Distância do sensor DIREITO - 4

//rolos de limpeza
#define motor1_p 46
#define motor1_n 47
#define motor2_p 44
#define motor2_n 45

//compressor
#define comp_p 48
#define comp_n 49

//variáveis da baliza para a direita sentido esquerda->direita
int modo = 0;
int giro1_dir = 0;
int giro2_dir = 0;
int anda_dir = 0;
int para_dir = 0;
int subida_dir = 0;
int descida_dir = 0;
int permite_dir = 1;
int inicio = 0;

//variáveis da baliza para a esquerda sentido esquerda->direita
int giro1_esq = 0;
int giro2_esq = 0;
int anda_esq = 0;
int para_esq = 0;
int subida_esq = 0;
int descida_esq = 0;
int permite_esq = 1;

//variáveis da baliza para a direita sentido direita->esquerda
int giro1_dir1 = 0;
int giro2_dir1 = 0;
int anda_dir1 = 0;
int para_dir1 = 0;
int subida_dir1 = 0;
int descida_dir1 = 0;
int permite_dir1 = 1;

//variáveis da baliza para a esquerda sentido direita->esquerda
int giro1_esq1 = 0;
int giro2_esq1 = 0;
int anda_esq1 = 0;
int para_esq1 = 0;
int subida_esq1 = 0;
int descida_esq1 = 0;
int permite_esq1 = 1;

// variaveis para controle prático do robo:
#define sob_p 1000000 // controle de subida
#define des_p -1000000 // controle de descida
#define par_p 0 // controle de parada
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
// dois stepper motors um em cada entrada da shield
AF_Stepper motor1(200, 1);
AF_Stepper motor2(200, 2);
// funções de movimento do primeiro motor(pode ser trocado pra DOUBLE ou INTERLEAVE ou MICROSTEP):
void forwardstep1()
{
  motor1.onestep(FORWARD, SINGLE); // função de andar pra frente, modo single
}
void backwardstep1()
{
  motor1.onestep(BACKWARD, SINGLE); // função de andar pra trás, modo single
}
// funções de movimento do segundo motor(pode ser trocado pra DOUBLE ou INTERLEAVE ou MICROSTEP):
void forwardstep2()
{
  motor2.onestep(FORWARD, SINGLE); // função de andar pra frente, modo single
}
void backwardstep2()
{
  motor2.onestep(BACKWARD, SINGLE); // função de andar pra trás, modo single
}

// Motor shield tem duas portas pra motor, agora a gente coloca elas num objeto da lib accelstepper:
AccelStepper stepper1(forwardstep1, backwardstep1);
AccelStepper stepper2(forwardstep2, backwardstep2);

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
  stepper1.setCurrentPosition(par_p);
  stepper2.setCurrentPosition(par_p);
  stepper1.stop();
  stepper2.stop();
  return;
}
//função de baliza para a esquerda sentido direita->esquerda
int baliza_esq()
{
  if (permite_dir == 1)//Permissão para fazer a baliza esquerda
  {
    permite_esq = 0; //Não deixa a baliza ocorrer ao mesmo tempo que a outra
    if (giro1_dir == 0)//Se não girou ainda pela 1 vez
    {
      stepper1.moveTo(giro_esq1); //gira pra um lado - giro_esq1
      stepper2.moveTo(-giro_esq2); //gira pro outro - giro_esq2
      giro1_dir = 1;// indica que já girou pela 1 vez
    } else if (giro1_dir == 1 && anda_dir == 0 && stepper1.currentPosition() == 100 && stepper2.currentPosition() == -100)//Se já girou e não moveu ainda
    {
      stepper1.setCurrentPosition(0);
      stepper2.setCurrentPosition(0);
      stepper1.moveTo(-mov1_esq); // mov1_esq
      stepper2.moveTo(-mov2_esq); // mov2_esq
      anda_dir = 1;// indica que já andou depois de girar pela 1 vez
    } else if (giro1_dir == 1 && anda_dir == 1 && giro2_dir == 0 && stepper1.currentPosition() == -100 && stepper2.currentPosition() == -100)//Se já girou e moveu
    {
      stepper1.setCurrentPosition(0);
      stepper2.setCurrentPosition(0);
      stepper1.moveTo(-giro_esq3); //giro_esq3
      stepper2.moveTo(giro_esq4); //giro4_esq4
      giro2_dir = 1;// indica que já girou pela 2 vez depois de andar
    } else if (giro2_dir == 1 && subida_dir ==  0 && stepper1.currentPosition() == -100 && stepper2.currentPosition() == 100)
    {
      sobe();//Sobe depois de finalizar a baliza para a direita
      subida_dir = 1;
    }
    if (subida_dir == 1 && para_dir == 0 && !digitalRead(dist_2) && digitalRead(dist_1) && digitalRead(dist_3) && digitalRead(dist_4))
    {
      para();//para depois de verificar o final da placa na subida
      para_dir = 1;
    }
    if (para_dir == 1 && descida_dir == 0 && stepper1.currentPosition() == 0 && stepper2.currentPosition() == 0)
    {
      desce();//desce depois de chegar na final da placa
      descida_dir = 1;
    }
    if (descida_dir == 1 && digitalRead(dist_2) && !digitalRead(dist_1) && digitalRead(dist_3) && digitalRead(dist_4))
    {
      para();//Para depois de verificar o final da placa na descida e reseta as variaveis
      permite_dir = 0;
      permite_esq = 1;
      giro1_dir = 0;
      giro2_dir = 0;
      anda_dir = 0;
      subida_dir = 0;
      descida_dir = 0;
      para_dir = 0;
    }
  }
  return permite_dir;
}
//função de baliza para a direita sentido direita->esquerda
int baliza_dir()
{
  if (permite_esq == 1)//Permissão para fazer a baliza direita
  {
    permite_dir = 0; //Não deixa a baliza ocorrer ao mesmo tempo que a outra
    if (giro1_esq == 0)//Se não girou ainda pela 1 vez
    {
      stepper1.moveTo(-giro_dir1); //gira pra um lado - giro_dir1
      stepper2.moveTo(giro_dir2); //gira pro outro - giro_dir2
      giro1_esq = 1;// indica que já girou pela 1 vez
    } else if (giro1_esq == 1 && anda_esq == 0 && stepper1.currentPosition() == -100 && stepper2.currentPosition() == 100)//Se já girou e não moveu ainda
    {
      stepper1.setCurrentPosition(0);
      stepper2.setCurrentPosition(0);
      stepper1.moveTo(mov1_dir); // mov1_dir
      stepper2.moveTo(mov2_dir); // mov2_dir
      anda_esq = 1;// indica que já andou depois de girar pela 1 vez
    } else if (giro1_esq == 1 && anda_esq == 1 && giro2_esq == 0 && stepper1.currentPosition() == 100 && stepper2.currentPosition() == 100)//Se já girou e moveu
    {
      stepper1.setCurrentPosition(0);
      stepper2.setCurrentPosition(0);
      stepper1.moveTo(giro_dir3); // giro_dir3
      stepper2.moveTo(-giro_dir4); // giro_dir4
      giro2_esq = 1;// indica que já girou pela 2 vez depois de andar
    } else if (giro2_esq == 1 && subida_esq ==  0 && stepper1.currentPosition() == 100 && stepper2.currentPosition() == -100)
    {
      desce();//Desce depois de finalizar a baliza para a esquerda
      descida_esq = 1;
    }
    if (descida_esq == 1 && para_esq == 0 && digitalRead(dist_2) && !digitalRead(dist_1) && digitalRead(dist_3) && digitalRead(dist_4))
    {
      para();//para depois de verificar o final da placa na descida
      para_esq = 1;
    }
    if (para_esq == 1 && subida_esq == 0 && stepper1.currentPosition() == 0 && stepper2.currentPosition() == 0)
    {
      sobe();//Sobe depois de chegar na final da placa
      subida_esq = 1;
    }
    if (subida_esq == 1 && !digitalRead(dist_2) && digitalRead(dist_1) && digitalRead(dist_3) && digitalRead(dist_4))
    {
      para();//Para depois de verificar o final da placa na descida e reseta as variaveis
      permite_esq = 0;
      permite_dir = 1;
      giro1_esq = 0;
      giro2_esq = 0;
      anda_esq = 0;
      subida_esq = 0;
      descida_esq = 0;
      para_esq = 0;
    }
  }
  return;
}
//função de baliza para a esquerda sentido esquerda->direita:
int baliza_esq2()
{
  if (permite_dir1 == 1)//Permissão para fazer a baliza esquerda sentido esquerda->direita:
  {
    permite_esq1 = 0; //Não deixa a baliza ocorrer ao mesmo tempo que a outra
    if (giro1_dir1 == 0)//Se não girou ainda pela 1 vez
    {
      stepper1.moveTo(-giro_esq1); //gira pra um lado - giro_esq1
      stepper2.moveTo(giro_esq2); //gira pro outro - giro_esq2
      giro1_dir1 = 1;// indica que já girou pela 1 vez
    } else if (giro1_dir1 == 1 && anda_dir1 == 0 && stepper1.currentPosition() == -100 && stepper2.currentPosition() == 100)//Se já girou e não moveu ainda
    {
      stepper1.setCurrentPosition(0);
      stepper2.setCurrentPosition(0);
      stepper1.moveTo(-mov1_esq); // mov1_esq
      stepper2.moveTo(-mov2_esq); // mov2_esq
      anda_dir1 = 1;// indica que já andou depois de girar pela 1 vez
    } else if (giro1_dir1 == 1 && anda_dir1 == 1 && giro2_dir1 == 0 && stepper1.currentPosition() == -100 && stepper2.currentPosition() == -100)//Se já girou e moveu
    {
      stepper1.setCurrentPosition(0);
      stepper2.setCurrentPosition(0);
      stepper1.moveTo(giro_esq3); //giro_esq3
      stepper2.moveTo(-giro_esq4); //giro4_esq4
      giro2_dir1 = 1;// indica que já girou pela 2 vez depois de andar
    } else if (giro2_dir1 == 1 && subida_dir1 ==  0 && stepper1.currentPosition() == 100 && stepper2.currentPosition() == -100)
    {
      sobe();//Sobe depois de finalizar a baliza para a direita
      subida_dir1 = 1;
    }
    if (subida_dir1 == 1 && para_dir1 == 0 && !digitalRead(dist_2) && digitalRead(dist_1) && digitalRead(dist_3) && digitalRead(dist_4))
    {
      para();//para depois de verificar o final da placa na subida
      para_dir1 = 1;
    }
    if (para_dir1 == 1 && descida_dir1 == 0 && stepper1.currentPosition() == 0 && stepper2.currentPosition() == 0)
    {
      desce();//desce depois de chegar na final da placa
      descida_dir1 = 1;
    }
    if (descida_dir1 == 1 && digitalRead(dist_2) && !digitalRead(dist_1) && digitalRead(dist_3) && digitalRead(dist_4))
    {
      para();//Para depois de verificar o final da placa na descida e reseta as variaveis
      permite_dir1 = 0;
      permite_esq1 = 1;
      giro1_dir1 = 0;
      giro2_dir1 = 0;
      anda_dir1 = 0;
      subida_dir1 = 0;
      descida_dir1 = 0;
      para_dir1 = 0;
    }
  }
  return permite_dir1;
}
//função de baliza para a direita sentido esquerda->direita:
int baliza_dir2()
{
  if (permite_esq1 == 1)//Permissão para fazer a baliza direita
  {
    permite_dir1 = 0; //Não deixa a baliza ocorrer ao mesmo tempo que a outra
    if (giro1_esq1 == 0)//Se não girou ainda pela 1 vez
    {
      stepper1.moveTo(giro_dir1); //gira pra um lado - giro_dir1
      stepper2.moveTo(-giro_dir2); //gira pro outro - giro_dir2
      giro1_esq1 = 1;// indica que já girou pela 1 vez
    } else if (giro1_esq1 == 1 && anda_esq1 == 0 && stepper1.currentPosition() == 100 && stepper2.currentPosition() == -100)//Se já girou e não moveu ainda
    {
      stepper1.setCurrentPosition(0);
      stepper2.setCurrentPosition(0);
      stepper1.moveTo(mov1_dir); // mov1_dir
      stepper2.moveTo(mov2_dir); // mov2_dir
      anda_esq1 = 1;// indica que já andou depois de girar pela 1 vez
    } else if (giro1_esq1 == 1 && anda_esq1 == 1 && giro2_esq1 == 0 && stepper1.currentPosition() == 100 && stepper2.currentPosition() == 100)//Se já girou e moveu
    {
      stepper1.setCurrentPosition(0);
      stepper2.setCurrentPosition(0);
      stepper1.moveTo(-giro_dir3); // giro_dir3
      stepper2.moveTo(giro_dir4); // giro_dir4
      giro2_esq1 = 1;// indica que já girou pela 2 vez depois de andar
    } else if (giro2_esq1 == 1 && subida_esq1 ==  0 && stepper1.currentPosition() == -100 && stepper2.currentPosition() == 100)
    {
      desce();//Desce depois de finalizar a baliza para a esquerda
      descida_esq1 = 1;
    }
    if (descida_esq1 == 1 && para_esq1 == 0 && digitalRead(dist_2) && !digitalRead(dist_1) && digitalRead(dist_3) && digitalRead(dist_4))
    {
      para();//para depois de verificar o final da placa na descida
      para_esq1 = 1;
    }
    if (para_esq1 == 1 && subida_esq1 == 0 && stepper1.currentPosition() == 0 && stepper2.currentPosition() == 0)
    {
      sobe();//Sobe depois de chegar na final da placa
      subida_esq1 = 1;
    }
    if (subida_esq1 == 1 && !digitalRead(dist_2) && digitalRead(dist_1) && digitalRead(dist_3) && digitalRead(dist_4))
    {
      para();//Para depois de verificar o final da placa na descida e reseta as variaveis
      permite_esq1 = 0;
      permite_dir1 = 1;
      giro1_esq1 = 0;
      giro2_esq1 = 0;
      anda_esq1 = 0;
      subida_esq1 = 0;
      descida_esq1 = 0;
      para_esq1 = 0;
    }
  }
  return;
}
//função da limpeza para a direita
void limpadir()
{
  baliza_dir();
  baliza_esq();
  return;
}
//função da limpeza para a esquerda
void limpaesq()
{
  baliza_dir2();
  baliza_esq2();
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
  pinMode(comp_p, OUTPUT);
  pinMode(comp_n, OUTPUT);

  //settando aceleração e velocidade maxima dos steppers:
  stepper1.setMaxSpeed(100);
  stepper1.setAcceleration(100.0);
  stepper2.setMaxSpeed(100);
  stepper2.setAcceleration(100.0);

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
      sobe();
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
      sobe();
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
      sobe();
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
      sobe();
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
      sobe();
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
      sobe();
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
      stepper1.setCurrentPosition(0);
      stepper2.setCurrentPosition(0);
      if (i7 == 0)
      {
        lcd.setCursor(0, 1);
        lcd.print("Descendo   - 2.1");
        i = 0; i2 = 0; i3 = 0; i4 = 0; i5 = 0; i6 = 0; i7 = 1; i8 = 0; i9 = 0; i10 = 0;
      }
    } else if (distsen1 && !distsen2 && !distsen3 && distsen4) //condição 2.2 -> desce(1001) - O robo está no canto superior esquerdo da placa
    {
      modo = 1; // Inicia a limpeza para a direita
      stepper1.setCurrentPosition(0);
      stepper2.setCurrentPosition(0);
      if (i8 == 0)
      {
        lcd.setCursor(0, 1);
        lcd.print("Descendo   - 2.2");
        i = 0; i2 = 0; i3 = 0; i4 = 0; i5 = 0; i6 = 0; i7 = 0; i8 = 1; i9 = 0; i10 = 0;
      }
    } /*else if (distsen1 && !distsen2 && !distsen3 && !distsen4) //condição 2.3 -> desce(1000)
    {
      desce();
      stepper1.run();
      stepper2.run();

      if (i9 == 0)
      {
        lcd.setCursor(0, 1);
        lcd.print("Descendo   - 2.3");
        i = 0; i2 = 0; i3 = 0; i4 = 0; i5 = 0; i6 = 0; i7 = 0; i8 = 0; i9 = 1; i10 = 0;
      }
    } */else
    {
      para();
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
    if (inicio == 0)
    {
      desce();
      stepper1.run();
      stepper2.run();
      if (!distsen1)
      {
        para();
        inicio = 1;
      }
    } else if (inicio == 1)
    {
      ligaesc();
      limpadir();
      stepper1.run();
      stepper2.run();
      if (distsen1 && !distsen2 && distsen3 && !distsen4)
      {
        modo = 3;
      }
    }
  }
  if (modo == 2) //modo 2 -> Inicia a limpeza para a esquerda
  {
    if (inicio == 0)
    {
      desce();
      stepper1.run();
      stepper2.run();
      if (!distsen1)
      {
        para();
        inicio = 1;
      }
    } else if (inicio == 1)
    {
      ligaesc();
      limpaesq();
      stepper1.run();
      stepper2.run();
      if (distsen1 && !distsen2 && !distsen3 && distsen4)
      {
        modo = 3;
      }
    }
  }
  if (modo == 3) // modo de teste só, por enquanto
  {
    desligaesc();
    inicio = 0;
  }
}// end loop
