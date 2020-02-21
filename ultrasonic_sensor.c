#include <3052.h>
#define RX_BUFFER_SIZE 256
#define TX_BUFFER_SIZE 256
////////////////距離測定センサのライブラリ開始///////////////
//とりあえずTrigはPA-0、EchoはPA-1につなぐ。PortAでなくてもできると思う
//イニシャライズ関数
void init(void){   //ポートのイニシャライズ
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
void measure(long *c){ //p.114参照。GRAなどによるTCNTクリア禁止 //立ち上がりエッジでカウント、内部クロックの1/8でカウント
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
void culculate(long count,long *d){   
  *d = count;
  *d *= 0.32; //カウンタの値を時間(マイクロ秒)に変換。カウント1回で0.32μs。
  *d /= 2; //往復の時間を半分にして片道の時間にする
  *d *= 340; //1マイクロ秒あたりの距離(m)を算出。音速は340m/sとする
  *d *= 100; //mをcmに変換
  *d /= 1000000; //1秒あたりの距離を算出//countはlong型なので先に100万で割ると0になってしまう
}
long ultrasonic_sensor(){   
    long count; //タイマーのカウント数を格納する変数
    long distance; //距離を格納する変数
    measure(&count); //距離を測定
    culculate(count,&distance); //カウンタの値を距離に変換 SCI1_PRINTF("%ldcm\n", distance); //distanceに必要な値が格納された
    return distance;
}
/////////////////////距離測定センサのライブラリ終了///////////////////////////
void main (void){ 
  init (); //イニシャライズ関数
  static char rxb[RX_BUFFER_SIZE];
  static char txb[TX_BUFFER_SIZE];
  SCI1_INIT(br19200, txb, TX_BUFFER_SIZE, rxb, RX_BUFFER_SIZE);
  EI;
  ultrasonic_sensor();
};
