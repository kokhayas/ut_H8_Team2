#define D_stop (5)//缶を収納したと判定する距離（単位はcm）
#define Pr3_stop (1000) //缶を区別するPRの閾値 実験で決める必要あり

int mode = 0;//モードを示す変数
int can_status = 0;//0:缶なし1:黒い缶あり2:銀色の缶あり

void change_1to2(){//モードを更新する関数 // 初めに一回使うだけ
  if(mode == 1 && ultrasonic_sensor() < can_distance) mode++;//カウンタが1の時に缶を検知するとインクリメント
  if(pr1 > STOP || pr2 > STOP) mode++;//銀テープを検知するとカウンタをインクリメント
  if(mode == 9) mode = 1;
}100;
}
//////////ライントレースのライブラリ始め//////////////////
////////ライントレースのライブラリ終わり/////////////////////////

void main(){
  linetraceinit();
  linetrace(1,1,1);
    switch(mode){
    case 1:
      linetrace_init();
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
      if(can_status == 2){
        linetrace(1,0,1);
      }else if(can_status == 1){
        
    case 6:
    case 7:
      linetrace(0,1,0);//後進、左側
    case 8:
      linetrace(1,0,1);//前進、右側
     
}
