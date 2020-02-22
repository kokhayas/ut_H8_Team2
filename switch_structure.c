#include <3052.h>
#define RX_BUFFER_SIZE 256
#define TX_BUFFER_SIZE 256
//////////////ライントレースのライブラリ始まり////////////////
#define Kp          (4)    //ライントレースの定数// 比例ゲイン
#define Duty_base   (70)        //基本制御のDuty比(%)
#define Duty_width  (20)         //Duty比振れ幅(%)
#define Target_ref  (986)       //目標PR値 pr1,pr2の平均値
//-------------------------------------
// #define STOP (1006) //銀テープ検知の定義pr値

//--------------------------------------
void quater_msecwait(){
 int t =17;
 while(t--);
 }
void wait(void){
  long t=500000;
  while(t--);
}
int readPr1(){
  int i; long sum;
  for(i = 0; i < 100; i ++){
    AD.ADCSR.BIT.ADF=0;  // フラグクリア
    AD.ADCSR.BIT.ADST=1;          //変換スタート
    while(AD.ADCSR.BIT.ADF==0);     //変換が終わるまで待つ
    sum += AD.ADDRA >> 6;   //Port7,0
    AD.ADCSR.BIT.ADF=0;    //フラグクリア
    quater_msecwait(); 
  }
  return sum/100; 
}
  
int readPr2(){
  int i; long sum;
  for(i = 0; i < 100; i ++){
    AD.ADCSR.BIT.ADF=0;  // フラグクリア
    AD.ADCSR.BIT.ADST=1;          //変換スタート
    while(AD.ADCSR.BIT.ADF==0);     //変換が終わるまで待つ
    sum += AD.ADDRB >> 6;   //Port7,0
    AD.ADCSR.BIT.ADF=0;    //フラグクリア
    quater_msecwait(); 
  }
  return sum/100; 
}

int readPr3(){
  int i; long sum;
  for(i = 0; i < 100; i ++){
    AD.ADCSR.BIT.ADF=0;  // フラグクリア
    AD.ADCSR.BIT.ADST=1;          //変換スタート
    while(AD.ADCSR.BIT.ADF==0);     //変換が終わるまで待つ
    sum += AD.ADDRC >> 6;   //Port7,0
    AD.ADCSR.BIT.ADF=0;    //フラグクリア
    quater_msecwait(); 
  }
  return sum/100; 
}
//初期設定-------------------------------
void ioinit_MD(){
  PA.DDR = 0xff;//ポートA出力
  PB.DDR = 0xff;//ポートB出力
    }
void ituinit_ITU(){    //MD用PWM出力初期設定
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
void adinit_CH1(){
  AD.ADCSR.BIT.ADF = 0;     //PR     
  AD.ADCSR.BIT.SCAN = 1;
  AD.ADCSR.BIT.CKS = 1;
  AD.ADCSR.BIT.CH = 1;//P7-0,P7-1,P7-2ピン利用 CN2 12pin & CN2 13pin //PR
}
void linetraceinit(){
  ioinit_MD();
  ituinit_ITU();
  adinit_CH1();
}
//制御用関数------------------------------
void motor_start(int a0){
    PA.DR.BIT.B2=a0;  
    PA.DR.BIT.B4=~a0;
    PA.DR.BIT.B6=a0;
    PB.DR.BIT.B2=~a0;
    wait();
} 
void motor_stop(void){
    PA.DR.BIT.B2=0;  
    PA.DR.BIT.B4=0;
    PA.DR.BIT.B6=0;
    PB.DR.BIT.B2=0;
}
void Pcontrl(int a0,int a1,int a2,int pr1,int pr2){  
    int dutycntrl1;
    int dutycntrl2; //モーター1に対して制御 //ifで分岐。銀テープが来たらモーターの回転を止める
     dutycntrl1=(Target_ref - pr1)*Kp;//duty制御量の算出 //理想的には0になるべき値
      if(dutycntrl1>Duty_width){
        dutycntrl1=Duty_width;
      }
    else if(dutycntrl1<-Duty_width){
      dutycntrl1=-Duty_width;
    }
    dutycntrl2=(Target_ref - pr2)*Kp;//duty制御量の算出 //理想的には0になるべき値
    if(dutycntrl2>Duty_width){
      dutycntrl2=Duty_width;
    }
    else if(dutycntrl2<-Duty_width){
      dutycntrl2=-Duty_width;
    }
    motor_start(a0);
    if(a0==1){
      ITU0.GRB=100*(Duty_base + dutycntrl1*a1 - dutycntrl2*a2);  // ITU0.GRB=
      ITU2.GRB=100*(Duty_base + dutycntrl2*a2 - dutycntrl1*a1);  // 
    }   //前進します
    else if(a0==0){
      ITU1.GRB=100*(Duty_base + dutycntrl1*a1 - dutycntrl2*a2);
      ITU3.GRB=100*(Duty_base + dutycntrl2*a2 - dutycntrl1*a1);
    }  //後進します
    SCI1_PRINTF("ITU0.GRB=%d\n",ITU0.GRB);
    SCI1_PRINTF("ITU1.GRB=%d\n",ITU1.GRB);
    SCI1_PRINTF("ITU2.GRB=%d\n",ITU2.GRB);
    SCI1_PRINTF("ITU3.GRB=%d\n",ITU3.GRB);
    
    ITU.TSTR.BIT.STR0 = 1;
    ITU.TSTR.BIT.STR1 = 1;
    ITU.TSTR.BIT.STR2 = 1;
    ITU.TSTR.BIT.STR3 = 1;  // PWM信号出力開始
    SCI1_PRINTF("OK\n");
    }

////////////////ライントレースのライブラリ終わり////////////////////
////////////////距離測定センサのライブラリ開始///////////////
//とりあえずTrigはPA-0、EchoはPA-1につなぐ。PortAでなくてもできると思う
//イニシャライズ関数
void ultrasonicinit(void){   //ポートのイニシャライズ
  P8.DDR = 0x01; //P8-0(Trig)は出力、P8-2(Echo)は入力//ITUのイニシャライズ
  P8.DR.BYTE = 0x00; //最初はEcho,TrigともにLow(0)
}
//precisely  xマイクロ秒待機する関数
void micro_wait(int x){
  ITU4.TCR.BYTE = 0x20; //超音波を出す時間を制御する  1/1,GRA==tcntの時にtcntクリア//p.114参照。GRAコンペアマッチでTCNTクリア//立ち上がりエッジでカウント、内部クロックそのままでカウント
  ITU4.GRA = x * 25; //1カウントで0.04μsだから25カウントで1μs
  ITU.TSTR.BIT.STR4 = 1; //タイマースタート
  while (!(ITU4.TSR.BIT.IMFA)); //フラグが立つ、つまり設定した時間が経つまで待つ
  ITU.TSTR.BIT.STR4 = 0; //タイマーストップ
  ITU4.TCNT = 0; //タイマーリセット
  ITU4.TSR.BIT.IMFA = 0; //フラグクリア
}
//距離を測定する周期を決める関数
//micro_waitは2msくらいしか計れないので別の関数を使う
//正確な制御はいらないのでタイマーは使わない
void about_wait(int t){  
  while (t--){   
    int m = 100;
    while (m--);
  }
}
//測距センサを動かす関数
void getCount(long *c){ //p.114参照。GRAなどによるTCNTクリア禁止 //立ち上がりエッジでカウント、内部クロックの1/8でカウント
  P8.DR.BIT.B0 = 1; //TrigをHigh(1)にして超音波を出力
  micro_wait(20); //超音波を20μs出す  //ITUのイニシャライズ
  ITU4.TCR.BYTE = 0x03; //超音波が反射するのにかかる時間を計測するタイマ 
  P8.DR.BIT.B0 = 0; //TrigをLow(0)にして超音波を出すのをやめる
  while (P8.DR.BIT.B2 == 0); //Echoの立ち上がりを待つ   //超音波を出し終わると同時にEchoが勝手に始動してHigh(1)になるSCI1_PRINTF("Echo high \n");
  ITU.TSTR.BIT.STR4 = 1; //タイマースタート
  while (P8.DR.BIT.B2 == 1); //Echoの立ち下がり、つまり反射した超音波を検出するのを待つ //反射した超音波を検出するとEchoが勝手にLow(0)になる SCI1_PRINTF("Echo low \n");
  *c = ITU4.TCNT; //タイマーの値をcountに格納
  ITU.TSTR.BIT.STR4 = 0; //タイマーストップ
  ITU4.TCNT = 0; //タイマーリセット  SCI1_PRINTF("%ld\n", count);
}
//カウンタの値を距離に変換
void convertCD(long count,long *d){   
  *d = count;
  *d *= 0.32; //カウンタの値を時間(マイクロ秒)に変換。カウント1回で0.32μs。
  *d /= 2; //往復の時間を半分にして片道の時間にする
  *d *= 340; //1マイクロ秒あたりの距離(m)を算出。音速は340m/sとする
  *d *= 100; //mをcmに変換
  *d /= 1000000; //1秒あたりの距離を算出//countはlong型なので先に100万で割ると0になってしまう
}
long readD1(){   
    long  count; //タイマーのカウント数を格納する変数
    long  distance; //距離を格納する変数
    getCount(&count); //距離を測定
    convertCD(count,&distance); //カウンタの値を距離に変換 SCI1_PRINTF("%ldcm\n", distance); //distanceに必要な値が格納された
    return distance;
}
/////////////////////距離測定センサのライブラリ終了///////////////////////////
void detectSilver(int pr1,int pr2,int sflag){
    if (pr1>1006 || pr2>1006){
      sflag=1;}
    else{
      sflag=0;
   }
}
void detectDistance(long d1,int dflag){
    if (d1 < 5){
      dflag=1;}
    else {
      dflag=0;
   }
} 
void decidePr3(int pr3,int color){
  if(pr3>1000){
    color=1;
  }
  else if(pr3<=1000){
    color=-1;
  }
}
//main------------------------------------
 //初期設定、変数の定義、変数に値を代入、関数に変数を代入、
//adinit_CH1();
//  int pr1;int pr2;  
 // pr1=readPr1(); pr2=readPr2();//0~1023;
  
 // ultrasonicinit();
 // long  d1; 
  //d1=readD1(); //cm
 
  //int sflag=0; 
  //detectSilver(pr1,pr2,&sflag);
 // if (sflag=1){};
    
  //int dflag=0;  
  //detectDistance(d1,&dflag);
  //if(dflag==1){
  //  int pr3; pr3=readPr3();
  //  void decidePr3(pr3,color);
 // };
//  if (color=1){}
 // else if(color=-1){}
  //ioinit_MD();
 // ituinit_ITU();
 // Pcontrl(1,1,1,pr1,pr2);
 // wait();
 // return 0;
//};
int mode= 0;//モードを示す変数  グローバル変数
int color = 0;//0:缶なし0:黒い缶あり-1:銀色の缶あり1:　グローバル変数
int main(){
    //switch検知やEEPROMの初期設定をする　初期設定が終わったらmode=1;にする
 　　mode=1
    switch(mode){
    case 1: 
      color=0; //初期化
      int dflag=0;
      while(dflag==0){
        adinit_CH1();
        int pr1;int pr2;  
        pr1=readPr1(); pr2=readPr2();//0~1023;
        ultrasonicinit();
        long  d1; 
        d1=readD1(); //cm //int sflag=0; detectSilver(pr1,pr2,&sflag)// if (sflag=1){};
        detectDistance(d1,dflag);   //缶を検知したらdflagを立てる
        if(dflag==1){        // dflag==1;のとき缶の色を検知しcolorに代入
          int pr3; pr3=readPr3();
          decidePr3(pr3,color);
        }
       ioinit_MD();
       ituinit_ITU();
       Pcontrl(1,1,1,pr1,pr2);   //直進、両方のPrを用いたライントレース(a0,a1,a2,pr1,pr2)
       wait();    //この時間だけライントレースをする
      }
      mode=2; //缶を見つけたらwhile文を抜けてmode2へ移行
    break;
     case 2:
       int sflag=0;
      while(sflag==1){
        adinit_CH1();
        int pr1;int pr2;  
        pr1=readPr1(); pr2=readPr2();//0~1023;
        detectSilver(pr1,pr2,sflag); //もし銀テープをどちらかのPrで検知したらsflag=1;にする
        ioinit_MD();
        ituinit_ITU();
        Pcontrl(0,1,1,pr1,pr2);   //後進、Pr2を用いたライントレース(a0,a1,a2,pr1,pr2)
        wait();    //この時間だけライントレースをする
      }
      mode=3; //銀テープを見つけたらwhile文を抜けてmode3へ移行
     break;
    case 3:
      int sflag=0;
      while(sflag==1){
        adinit_CH1();
        int pr1;int pr2;  
        pr1=readPr1(); pr2=readPr2();//0~1023;
        detectSilver(pr1,pr2,sflag); //もし銀テープをどちらかのPrで検知したらsflag=1;にする
        ioinit_MD();
        ituinit_ITU();
        Pcontrl(0,0,1,pr1,pr2);   //後進、Pr2を用いたライントレース(a0,a1,a2,pr1,pr2)
        wait();    //この時間だけライントレースをする
      }
      mode=4; //銀テープを見つけたらwhile文を抜けてmode3へ移行
      break;4
    case 4:
       int sflag=0;
       while(sflag==1){
         adinit_CH1();
         int pr1;int pr2;  
         pr1=readPr1(); pr2=readPr2();//0~1023;
         detectSilver(pr1,pr2,sflag); //もし銀テープをどちらかのPrで検知したらsflag=1;にする
         ioinit_MD();
         ituinit_ITU();
         Pcontrl(1,1,0,pr1,pr2);   //直進、Pr1を用いたライントレース(a0,a1,a2,pr1,pr2)
         wait();    //この時間だけライントレースをする
      }
      mode=5; //銀テープを見つけたらwhile文を抜けてmode4へ移行
      break;
    case 5:
      int sflag=0;
       while(sflag==0){
         adinit_CH1();
         int pr1;int pr2;  
         pr1=readPr1(); pr2=readPr2();//0~1023;
         detectSilver(pr1,pr2,sflag); //もし銀テープをどちらかのPrで検知したらsflag=1;にする
         ioinit_MD();
         ituinit_ITU();
         if(color==1){ //銀色の缶を持っている場合
         Pcontrl(1,1,0,pr1,pr2);   //直進、Pr1を用いたライントレース(a0,a1,a2,pr1,pr2)
         wait();    //この時間だけライントレースをする
         }
         else if(color==-1){
         Pcontrl(1,0,1,pr1,pr2);   //直進、pr2を用いたライントレース
         wait();
         }
       }
       mode = 6;   //缶捨て場に到着したらmode5へ移行
      break;
      
    case 6:
  int dflag=1;   //缶を落とすまで前後に少し移動する予定
      while(dflag==1){
        adinit_CH1();
        int pr1;int pr2;  
        pr1=readPr1(); pr2=readPr2();//0~1023;
        ultrasonicinit();
        long  d1; 
        d1=readD1(); //cm //int sflag=0; detectSilver(pr1,pr2,&sflag)// if (sflag=1){};
        detectDistance(d1,dflag);   //缶を捨てたらdflag=0;
        ioinit_MD();
        ituinit_ITU();
        Pcontrl(1,1,1,pr1,pr2);   //直進、両方のPrを用いたライントレース(a0,a1,a2,pr1,pr2)
        wait();    //この時間だけライントレースをする
       }
       mode=7;
      break;
    case 7:
        int sflag=0;
        while(sflag==0){
          adinit_CH1();
          int pr1;int pr2;  
          pr1=readPr1(); pr2=readPr2();//0~1023;
          detectSilver(pr1,pr2,sflag); //もし銀テープをどちらかのPrで検知したらsflag=1;にする
          ioinit_MD();
          ituinit_ITU();
          if(color==1){ //銀色の缶を持っていた場合
          Pcontrl(0,1,0,pr1,pr2);   //後進、Pr1を用いたライントレース(a0,a1,a2,pr1,pr2)
         wait();    //この時間だけライントレースをする
         }
         else if(color==-1){
         Pcontrl(0,0,1,pr1,pr2);   //後進、pr2を用いたライントレース
         wait();
         }
       }
       mode = 8;//へ移行
      break;
    case 8:
      int sflag=0;
        while(sflag==0){
          adinit_CH1();
          int pr1;int pr2;  
          pr1=readPr1(); pr2=readPr2();//0~1023;
          detectSilver(pr1,pr2,sflag); //もし銀テープをどちらかのPrで検知したらsflag=1;にする
          ioinit_MD();
          ituinit_ITU();
          Pcontrl(0,1,0,pr1,pr2);   //後進、Pr1を用いたライントレース(a0,a1,a2,pr1,pr2)
          wait();    //この時間だけライントレースをする
         }
       }
       mode = 9;//へ移行
 break;
     
    case 9:
          int sflag=0;
          while(sflag==0){
          adinit_CH1();
          int pr1;int pr2;  
          pr1=readPr1(); pr2=readPr2();//0~1023;
          detectSilver(pr1,pr2,sflag); //もし銀テープをどちらかのPrで検知したらsflag=1;にする
          ioinit_MD();
          ituinit_ITU();
          Pcontrl(1,0,1,pr1,pr2);   //後進、Pr1を用いたライントレース(a0,a1,a2,pr1,pr2)
          wait();    //この時間だけライントレースをする
         }
       }
       mode = 1;//へ移行
break;
 

     
}
