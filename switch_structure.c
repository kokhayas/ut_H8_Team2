#define can_distance 5 //缶を収納したと判定する距離（単位はcm）
#define can_pr //缶を区別するPRの閾値

int mode = 0;//モードを示す変数
int can_status = 0;//0:缶なし1:黒い缶あり2:銀色の缶あり

void change_mode(){//モードを更新する関数
  if(mode == 1 && ultrasonic_sensor() < can_distance) mode++;//カウンタが1の時に缶を検知するとインクリメント
  if(pr1 > STOP || pr2 > STOP) mode++;//銀テープを検知するとカウンタをインクリメント
  if(mode == 9) mode = 1;
}

long Read_canPR(){//缶を判定するPRの値を読み取る関数
  long sum = 0;
  for (i=0; i<10; i++){//10回の平均をとる
    AD.ADCSR.BIT.ADF = 0;// フラグクリア
    AD.ADCSR.BIT.ADST = 1;//変換スタート
    while(AD.ADCSR.BIT.ADF == 0);//変換が終わるまで待つ
    sum += AD.ADDRC >> 6;//6bit右にずらしてsumに格納
    AD.ADCSR.BIT.ADF = 0;//フラグクリア
    quater_msecwait(); 
  }
  return sum/10;//読み取った値を返す
}

void main(){
  
  change_mode();
  
  switch(mode){
    case 1:
      linetrace(1,1,1);//前進、両側
    case 2:
      motor_brake();//ブレーキ//motor_stopでもいいかも
      if(Read_canPR() > can_pr){//缶の色を判定
        can_status = 2;//銀
      }else{
        can_status = 1;//黒
      }  
      linetrace(0,0,1);//後進、右側
    case 3:
      linetrace(0,0,1);//後進、右側
    case 4:
      linetrace(1,1,0);//前進、左側
    case 5:
      
    case 6:
    case 7:
      linetrace(0,1,0);//後進、左側
    case 8:
      linetrace(1,0,1);//前進、右側
     
}
