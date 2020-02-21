//main------------------------------------
int Can_Detected; int Count;
int main(){
  ioinit(); //モータドライバ
  ituinit(); //モータドライバ
  adinit();  //PRのAD変換初期設定
  while(1){
    //start
    //1,2,3,4,5までのアルゴリズムの関数
void Take_Can_And_Turn_Algorithm(){
    Silver_Detect(ref1,ref2); //銀テープを検知したらモータを止めて、もう一度検査する。Silver_Detected=1;
    if(Silver_Detected==1 && Count<=1){
    Count+=1;
    Silver_Detected=0;  //count=2; まで行く
    }
    void Can_Detect(ref_front,distance){
    if(distance<=4){          //缶との距離が4cmの時
      ITU0.GRB = 0; //PAの2,4,6
      ITU1.GRB = 0;
      ITU2.GRB = 0;
      ITU3.GRB = 0; //PBの0   モータを止める
      wait();
      if(distance<4 && ref_front>100){
      Can_Detected=1;}   // いったん１になるとその後も継続して１のまま
    Can_Detect(ref_front,distance);   //缶測定用のref_frontの値と,測距離センサによるdistanceの値により缶が検知されるとモータが止まり、Can_Detected=1;となる
    if(Can_Detected==1 && Count==2){
    Count+=1;
    Silver_Detected=0;
    if(Silver_Detected==1 && Can_Detected==1 ){
    Count+=1;}
    switch(Count){   //カウンタの値に応じて動作を実行していく
    case 0:
             average();
             F_LR_Control(ref1,ref2);//PRの平均値を用いてMDに出力する Forward Left and Right
             wait();
             break;
    case 1:
             average();
             F_L_Control(ref1); //Forward left linetrace
             wait();
             break;
    case 2:  
             average();
             F_LR_Control(ref1,ref2);//PRの平均値を用いてMDに出力する Forward Left and Right
             wait();
             break;
    case 3:   
             average();
             B_LR_Control(ref1,ref2);   // Back Left & Right
             wait();
             break; 
    case 4:
             average();
             B_R_Control(ref2); // Backward left linetrace
             wait();
             break;
     if(Count==5)
     Count=10;
     break;
   }
