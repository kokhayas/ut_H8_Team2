#define can_distance 5 //缶を収納したと判定する距離（単位はcm）
#define can_pr //缶を区別するPRの閾値

int mode =0;//モードを示す変数

void change_mode(){//モードを更新する関数
  if(mode == 1 && ultrasonic_sensor() < can_distance) mode++;カウンタが1の時に缶を検知するとインクリメント
  if(pr1 > STOP || pr2 > STOP) mode++;//銀テープを検知するとカウンタをインクリメント
  if(count == 9) mode = 1;
}

int Read_canPR(){

}

void main(){
  
  change_mode();
  
  switch(mode){
    case 1:
      linetrace(1,1,1);//前進、両側
    case 2:
      motor_brake();//ブレーキ//motor_stopでもいいかも
      if(Read_canPR() > can_pr) 
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
