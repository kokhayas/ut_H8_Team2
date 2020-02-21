#define can_distance 5 //缶を収納したと判定する距離（単位はcm）

int count =0;//モードを示すカウンタ

void change_count(){//カウンタを更新する関数
  if(count == 1 && ultrasonic_sensor() < can_distance) count++;カウンタが1の時に缶を検知するとインクリメント
  if(pr1 > STOP || pr2 > STOP) count++;//銀テープを検知するとカウンタをインクリメント
  if(count == 9) count = 1;
}

void main(){
  
  change_count();
  
  switch(count){
    case 1:
      linetrace(1,1,1);//前進、両側
    case 2:
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
