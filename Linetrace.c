#include <3052.h>
#define RX_BUFFER_SIZE 256
#define TX_BUFFER_SIZE 256
//--------------------------------------
//
//比例制御のパラメータ
//
//--------------------------------------
//////////////ライントレースのライブラリ始まり////////////////
#define Kp          (4)    //比例ゲイン
#define Duty_base   (70)        //基本制御のDuty比(%)
#define Duty_width  (20)         //Duty比振れ幅(%)
#define Target_ref  (986)       //目標PR値
//-------------------------------------
#define STOP (1000)
//--------------------------------------
void quater_msecwait(){
 int t =17;
 while(t--);
 }
void Read_Pr(int *p,int *q){
  int i; long sum1; long sum2; 
  for ( i=0; i < 100; i++){
    AD.ADCSR.BIT.ADF=0;  // フラグクリア
    AD.ADCSR.BIT.ADST = 1;          //変換スタート
    while(AD.ADCSR.BIT.ADF ==0);     //変換が終わるまで待つ
    sum1 += AD.ADDRA >> 6;
    sum2 += AD.ADDRB >> 6;     //6bit右にずらしてsumに格納
    AD.ADCSR.BIT.ADF = 0;    //フラグクリア
    quater_msecwait();       //1msec wait  
  }
  *p=sum1/100;  //pr1=sum1/100;と同じ
  *q=sum2/100;  //pr2=sum2/100;と同じ
}
void wait(void){
  long t=500000;
  while(t--);
}
//初期設定-------------------------------
void ioinit(){
  PA.DDR = 0xff;//ポートA出力
  PB.DDR = 0xff;//ポートB出力
    }
void ituinit(){    //MD用PWM出力初期設定
  ITU0.TCR.BYTE = 0x23;
  ITU1.TCR.BYTE = 0x23;
  ITU2.TCR.BYTE = 0x23;
  ITU3.TCR.BYTE = 0x23;
  ITU.TMDR.BIT.PWM0 = 1;
  ITU.TMDR.BIT.PWM1 = 1;
  ITU.TMDR.BIT.PWM2 = 1;
  ITU.TMDR.BIT.PWM3 = 1;
  ITU0.GRA = 10000;
  ITU1.GRA = 10000;
  ITU2.GRA = 10000;
  ITU3.GRA = 10000;
  ITU0.GRB = 1;
  ITU1.GRB = 1;
  ITU2.GRB = 1;
  ITU3.GRB = 1;    
}
void adinit(){
  AD.ADCSR.BIT.ADF = 0;     //PR     
  AD.ADCSR.BIT.SCAN = 1;
  AD.ADCSR.BIT.CKS = 1;
  AD.ADCSR.BIT.CH = 1;//P7-0,P7-1ピン利用 CN2 12pin & CN2 13pin //PR
}
void linetraceinit(){
  ioinit();
  ituinit();
  adinit();
}
//制御用関数------------------------------
void motor_start(a0){
    PA.DR.BIT.B2=a0;  
    PA.DR.BIT.B4=~a0;
    PA.DR.BIT.B6=a0;
    PB.DR.BIT.B2=~a0;
} 
void motor_stop(void){
    PA.DR.BIT.B2=0;  
    PA.DR.BIT.B4=0;
    PA.DR.BIT.B6=0;
    PB.DR.BIT.B2=0;
}

void motor_brake(){//ブレーキをかける関数
    PA.DR.BIT.B2=1;  
    PA.DR.BIT.B4=1;
    PA.DR.BIT.B6=1;
    PB.DR.BIT.B2=1;
} 
void Pcontrl(int a0,int a1,int a2,int pr1,int pr2,int *stop){  
    int dutycntrl1;
    int dutycntrl2;
    //モーター1に対して制御
    //ifで分岐。銀テープが来たらモーターの回転を止める
    if(pr1>STOP && pr2>STOP){
      motor_stop();
      *stop=1;
      SCI1_PRINTF("STOP\n");
    }
    else
      dutycntrl1=(Target_ref - pr1)*Kp;//duty制御量の算出
      if(dutycntrl1>Duty_width){
        dutycntrl1=Duty_width;
      }
    else if(dutycntrl1<-Duty_width){
      dutycntrl1=-Duty_width;
    }
    dutycntrl2=(Target_ref - pr2)*Kp;//duty制御量の算出
    if(dutycntrl2>Duty_width){
      dutycntrl2=Duty_width;
    }
    else if(dutycntrl2<-Duty_width){
      dutycntrl2=-Duty_width;
    }
    motor_start();
    if(a0==1){
      ITU0.GRB=100*(Duty_base - dutycntrl1*a1 + dutycntrl2*a2);
      ITU2.GRB=100*(Duty_base - dutycntrl2*a2 + dutycntrl1*a1);
    }   //前進します
    else if(a0==0){
      ITU1.GRB=100*(Duty_base - dutycntrl1*a1);
      ITU3.GRB=100*(Duty_base - dutycntrl2*a2);
    }  //後進します
    SCI1_PRINTF("%dITU0.GRB\n",ITU0.GRB);
    SCI1_PRINTF("%dITU1.GRB\n",ITU1.GRB);
    SCI1_PRINTF("%dITU2.GRB\n",ITU2.GRB);
    SCI1_PRINTF("%dITU3.GRB\n",ITU3.GRB);
    ITU.TSTR.BIT.STR0 = 1;
    ITU.TSTR.BIT.STR1 = 1;
    ITU.TSTR.BIT.STR2 = 1;
    ITU.TSTR.BIT.STR3 = 1;  // PWM信号出力開始
    SCI1_PRINTF("OK\n");
    }
void linetrace(int a0,int a1,int a2){
  while(1){                           SCI1_PRINTF("LINE147\n");
    int pr1; int pr2;int Stop=0;
    Read_Pr(&pr1,&pr2);//
    Pcontrl(a0,a1,a2,pr1,pr2,&Stop);       SCI1_PRINTF("Line150\n"); //a0=1;前進、a0=0;後進、a1=1;pr1で制御、 a2=1;pr2で制御、0の時はprを無視して平均速度  
    wait();  
    if(Stop==1)
      break;
  };
  };
////////////////ライントレースのライブラリ終わり////////////////////
//main------------------------------------
int main(){
  static char rxb[RX_BUFFER_SIZE];
  static char txb[TX_BUFFER_SIZE];
  SCI1_INIT(br19200, txb, TX_BUFFER_SIZE, rxb, RX_BUFFER_SIZE);
  EI;                     
  linetraceinit();   SCI1_PRINTF("Linetracestart\n");// ioinit(); ituinit(); adinit();が含まれている
  linetrace(1,1,1);  SCI1_PRINTF("Lintraceend\n"); //linetrace(a0,a1,a2);   
  return 0;
}
