#include <3052.h>
#define RX_BUFFER_SIZE 256
#define TX_BUFFER_SIZE 256
#define Kp          (0.06)    //ライントレースの定数// 比例ゲイン
#define Kd          (0)
#define Duty_base   (40)        //基本制御のDuty比(%)
#define Duty_width  (15)        //Duty比の幅
#define Target_ref  (500)       //目標PR値 pr1,pr2の平均値
#define Average (50) //平均をとる回数
#define color_silver_pr1_pr2 (980)  // pr1>color_silver_pr1_pr2|| pr2>color_silver_pr1_pr2 の時に銀テープ検知
#define color_threshold_pr3 (800)  // pr3>threshold_pr3の時に銀の缶、pr3<threshold_pr3の時に黒の缶だと判断する
#define distance_threshold_pr3 (300)
​
// #define distance_d1 (6)   //d1<distance_d1の時に缶を検知する
​
#define CYCLE 4
#define CONTROLCODE	0xA0
#define WRITE 0
#define READ 1
​
//関数プロトタイプ宣言
void io_init(void);                             //IOポート
void scl_h(void);                               //SDL・SDA制御
void scl_l(void);
void sda_h(void);
void sda_l(void);
void sda_write(void);                           //SDA write or read
void sda_read(void);
void wait_us(unsigned int count1);                  //wait関数
void wait_ms(unsigned int count2);
void i2c_start(unsigned int address, unsigned char rw);				//i2c処理
void i2c_write_byte(unsigned char dat);
void i2c_write_byte_32(unsigned char *dat);
void i2c_read_byte(unsigned char *dat);
void i2c_read_byte_32(void);
void acknowledge_before_read(void);
void acknowledge_from_slave(void);
void acknowledge_from_master(void);
void i2c_stop(void);
​
unsigned int write_address = 0x0000;
unsigned int read_address = 0x0000;
unsigned char r_text[32];
​
void eeprom_write(unsigned char *data){
	unsigned int upper_address = (write_address >> 8) & 0xff; 
	unsigned int lower_address = write_address & 0xff;
	wait_ms(100);
	i2c_start(CONTROLCODE, WRITE);				//7bitアドレス
	acknowledge_from_slave();
	i2c_write_byte(upper_address);
	acknowledge_from_slave();
	i2c_write_byte(lower_address);
	acknowledge_from_slave();
	i2c_write_byte_32(data);	//&dataをdataに変更
	i2c_stop();
	wait_ms(5);
	write_address += 0x0020;
}
​
void eeprom_read(void){
	unsigned int upper_address = (read_address >> 8) & 0xff;
	unsigned int lower_address = read_address & 0xff;
	i2c_start(CONTROLCODE, WRITE);
	acknowledge_from_slave();
	i2c_write_byte(upper_address);
	acknowledge_from_slave();
	i2c_write_byte(lower_address);
	acknowledge_from_slave();
	i2c_start(CONTROLCODE, READ);
	acknowledge_before_read();
	i2c_read_byte_32();
	i2c_stop();
	wait_us(50);
	SCI1_PRINTF("%s\n", r_text);
	wait_ms(10);
	read_address += 0x0020;
}
​
​
//initiallize関数(Port4の1,3を出力に設定)
void io_init(void){
    P4.DDR = 0x0a;
}
​
//SCL,SDAのH・L制御
void scl_h(void){
    P4.DR.BIT.B1 = 1;
}
​
void scl_l(void){
    P4.DR.BIT.B1 = 0;
}
​
void sda_h(void){
    P4.DR.BIT.B3 = 1;
}
​
void sda_l(void){
    P4.DR.BIT.B3 = 0;
}
​
//SDAのread・write制御
void sda_write(void){
    P4.DDR = 0x0a;
}
​
void sda_read(void){
    P4.DDR = 0x02;
}
​
//wait関数(1カウントあたり約1us)
void wait_us(unsigned int count1){
    while(count1){
        count1--;
    }
}
​
//wait関数(1カウントあたり約1ms)
 void wait_ms(unsigned int count2){
	while(count2){
		wait_us(1000);
		count2--;
	}
}
​
//i2cのstart関数_SCLがHIGHのときにSDAの立ち下がりでスタートコンディション
void i2c_start(unsigned int address, unsigned char rw){
	sda_h();
	scl_h();
	wait_us(CYCLE);
	sda_l();									//start condition
	wait_us(CYCLE);
	scl_l();
	i2c_write_byte(((rw & 1) | address));		//スレーブアドレスとＲ／Ｗ
}
​
//1byte送信関数
void i2c_write_byte(unsigned char dat){		//0:write version, 1:read version
​
	unsigned char	i;
​
		wait_us(CYCLE);
​
		for (i = 0; i <= 7; i++) {			//上位から1bitずつ送信
			if(dat & 0x80) {			
				sda_h();
			}
			else {
				sda_l();
			}
			dat <<= 1;					//左に1bitシフト
			wait_us(CYCLE);
			scl_h();
			wait_us(CYCLE);
			scl_l();
		}
}
​
void i2c_write_byte_32(unsigned char *dat){
	unsigned char i;
	for (i=0; i<=31;i++){
		i2c_write_byte(*dat++);
		wait_ms(5);
		acknowledge_from_slave();
	}
}
​
//1byte読み出し関数
void i2c_read_byte(unsigned char *dat){
	sda_read();
	unsigned char	i;
	for(i = 0; i <= 7; i++) {
		wait_us(CYCLE);
		scl_h();
		*dat <<= 1;
		if(P4.DR.BYTE & 0x08){		
			*dat |= 0x01;
		}
		wait_us(CYCLE);
		scl_l();			
	}
	sda_write();
	sda_l();
}
​
void i2c_read_byte_32(void){
	unsigned char i;
	for (i=0; i<=30; i++){
		i2c_read_byte(&r_text[i]);
		acknowledge_from_master();
	}
	i2c_read_byte(&r_text[31]);
}
​
//acknowledge関数
void acknowledge_from_master(void){
	sda_write();
	wait_us(CYCLE);
	sda_l();
	wait_us(CYCLE);
	scl_h();
	wait_us(CYCLE);
	scl_l();
	wait_us(CYCLE);
	sda_read();
}
​
void acknowledge_from_slave(void){
	sda_read();
	wait_us(CYCLE);
	scl_h();
	wait_us(CYCLE);
	scl_l();
	sda_write();
}
​
void acknowledge_before_read(void){
	sda_read();
	wait_us(CYCLE);
	scl_h();
	wait_us(CYCLE);
	scl_l();
}
​
//i2cのstop関数_SCLがHIGHのときにSDAの立ち上がりでstop condition
void i2c_stop(void){
	sda_write();
    scl_l();
	sda_l();
	wait_us(CYCLE);
	scl_h();
	wait_us(CYCLE);
	sda_h();						//stop condition
	wait_us(CYCLE);
	scl_l();
	wait_us(CYCLE);
}
void wait(long t){
    while(t--);
}
void adinit(){
    AD.ADCSR.BIT.ADF = 0;     //PR
    AD.ADCSR.BIT.SCAN = 1;
    AD.ADCSR.BIT.CKS = 1;
    AD.ADCSR.BIT.CH = 2;//P7-0,P7-1,P7-2ピン利用 CN2 12pin & CN2 13pin //PR
}
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
}
void motor_start(int a0){ 
    PA.DR.BIT.B2=a0;
    PA.DR.BIT.B4=~a0;
    PA.DR.BIT.B6=a0;
    PB.DR.BIT.B0=~a0;
    ITU0.GRB = 5000;  
    ITU1.GRB = 1;
    ITU2.GRB = 5000;
    ITU3.GRB = 1;
    ITU.TSTR.BIT.STR0 = 1;
    ITU.TSTR.BIT.STR1 = 1;
    ITU.TSTR.BIT.STR2 = 1;
    ITU.TSTR.BIT.STR3 = 1;  // PWM信号出力開始
}
void motor_stop(void){
    PA.DR.BIT.B2=0;
    PA.DR.BIT.B4=0;
    PA.DR.BIT.B6=0;
    PB.DR.BIT.B0=0;
    ITU0.GRB = 1;
    ITU1.GRB = 1;
    ITU2.GRB = 1;
    ITU3.GRB = 1;
    ITU.TSTR.BIT.STR0 = 1;
    ITU.TSTR.BIT.STR1 = 1;
    ITU.TSTR.BIT.STR2 = 1;
    ITU.TSTR.BIT.STR3 = 1;  // PWM信号出力開始
}
void motor_backward_move(void){
    PA.DR.BIT.B2=0;
    PA.DR.BIT.B4=1;
    PA.DR.BIT.B6=0;
    PB.DR.BIT.B0=1;
    ITU0.GRB = 1;
    ITU1.GRB = 4000;
    ITU2.GRB = 1;
    ITU3.GRB = 4000;
    ITU.TSTR.BIT.STR0 = 1;
    ITU.TSTR.BIT.STR1 = 1;
    ITU.TSTR.BIT.STR2 = 1;
    ITU.TSTR.BIT.STR3 = 1;  // PWM信号出力開始
    }
void motor_brake(void){
    PA.DR.BIT.B2=1;
    PA.DR.BIT.B4=1;
    PA.DR.BIT.B6=1;
    PB.DR.BIT.B0=1;
    ITU0.GRB = 9999;
    ITU1.GRB = 9999;
    ITU2.GRB = 9999;
    ITU3.GRB = 9999;
    ITU.TSTR.BIT.STR0 = 1;
    ITU.TSTR.BIT.STR1 = 1;
    ITU.TSTR.BIT.STR2 = 1;
    ITU.TSTR.BIT.STR3 = 1;  // PWM信号出力開始
}
void motor_turn(void){
    PA.DR.BIT.B2=1;
    PA.DR.BIT.B4=0;
    PA.DR.BIT.B6=0;
    PB.DR.BIT.B0=1;
    ITU0.GRB = 6000;
    ITU1.GRB = 1;
    ITU2.GRB = 1;
    ITU3.GRB = 6000;
    ITU.TSTR.BIT.STR0 = 1;
    ITU.TSTR.BIT.STR1 = 1;
    ITU.TSTR.BIT.STR2 = 1;
    ITU.TSTR.BIT.STR3 = 1;  // PWM信号出力開始
}
int readPr1(){
    int i;
    long sum=0;
    for(i = 0; i < Average; i ++){
        AD.ADCSR.BIT.ADF=0;  // フラグクリア
        AD.ADCSR.BIT.ADST=1;          //変換スタート
        while(AD.ADCSR.BIT.ADF==0);     //変換が終わるまで待つ
        sum += AD.ADDRA >> 6;   //Port7,0
        AD.ADCSR.BIT.ADF=0;    //フラグクリア
    }
    return sum/Average;
}
int readPr2(){
    int i;
    long sum=0;
    for(i = 0; i < Average; i ++){
        AD.ADCSR.BIT.ADF=0;  // フラグクリア
        AD.ADCSR.BIT.ADST=1;          //変換スタート
        while(AD.ADCSR.BIT.ADF==0);     //変換が終わるまで待つ
        sum += AD.ADDRB >> 6;   //Port7,1
        AD.ADCSR.BIT.ADF=0;    //フラグクリア
    }
    return sum/Average;
}
​
int readPr3(){
    int i;
    long sum=0;
    for(i = 0; i < Average; i ++){
        AD.ADCSR.BIT.ADF=0;  // フラグクリア
        AD.ADCSR.BIT.ADST=1;          //変換スタート
        while(AD.ADCSR.BIT.ADF==0);     //変換が終わるまで待つ
        sum += AD.ADDRC >> 6;   //Port7,2
        AD.ADCSR.BIT.ADF=0;    //フラグクリア
    }
    return sum/Average;
}
​
​
void Pcontrl(int a0,int a1,int a2,int pr1,int pr2,int pr1_old,int pr2_old){
    int dutycntrl1;
    int dutycntrl2; //モーター1に対して制御 //ifで分岐。銀テープが来たらモーターの回転を止める
    dutycntrl1=(Target_ref - pr1)*Kp-(pr1-pr1_old)*Kd;//duty制御量の算出 //理想的には0になるべき値
    if(dutycntrl1>Duty_width){  //Duty_wifth大きくしたほうが良いかも
        dutycntrl1=Duty_width;
    }
    else if(dutycntrl1<-Duty_width){
        dutycntrl1=-Duty_width;
    }
    //SCI1_PRINTF("dutycntrl1=%d\t",dutycntrl1);
    dutycntrl2=(Target_ref - pr2)*Kp-(pr2-pr2_old)*Kd;//duty制御量の算出 //理想的には0になるべき値
    if(dutycntrl2>Duty_width){
        dutycntrl2=Duty_width;
    }
    else if(dutycntrl2<-Duty_width){
        dutycntrl2=-Duty_width;
    }
    //SCI1_PRINTF("dutycntrl2=%d\t",dutycntrl2);
    if(a0==1){
        ITU0.GRB=100*(Duty_base - dutycntrl1*a1 + dutycntrl2*a2);  //    (a1-a2) +
        ITU2.GRB=100*(Duty_base - dutycntrl2*a2 + dutycntrl1*a1);  // (a0,a1,a2) a0=1,直進  a0=0,後進 a1=1,
        //SCI1_PRINTF("ITU0.GRB=%d\t",ITU0.GRB);
        //SCI1_PRINTF("ITU2.GRB=%d\t",ITU2.GRB);
    }   //前進します
    else {
        ITU1.GRB=100*(Duty_base - dutycntrl1*a1 + dutycntrl2*a2);
        ITU3.GRB=100*(Duty_base - dutycntrl2*a2 + dutycntrl1*a1);
        //SCI1_PRINTF("ITU1.GRB=%d\t",ITU1.GRB);
        //SCI1_PRINTF("ITU3.GRB=%d\t",ITU3.GRB);
    }  //後進します
    ITU.TSTR.BIT.STR0 = 1;
    ITU.TSTR.BIT.STR1 = 1;
    ITU.TSTR.BIT.STR2 = 1;
    ITU.TSTR.BIT.STR3 = 1;  // PWM信号出力開始
    pr1_old=pr1;
    pr2_old=pr2;
}
​
​
int detect_distance_pr3(int pr3){
  if( pr3>distance_threshold_pr3){
  return 1;}
  else {return 0;}
  }
  
int detect_color_pr3(int pr3){
  if(pr3>color_threshold_pr3){  //銀だったら
    return 1;
  }
  else {  //黒だったら
    return -1;
  } 
}
​
int detect_silver_pr1_pr2(int pr1,int pr2){
  if (pr1>color_silver_pr1_pr2|| pr2>color_silver_pr1_pr2){ 
    return 1;        
  }
  else{ 
    return 0;
  }
}
​
void eeprom_flash(void){
    unsigned char data_0[32] = "00000000000000000000000000000000";
    int i;
    for (i=0;i<100;i++){
        eeprom_write(data_0);
    }
}
    
  int pr1;
  int pr2; 
  int pr1_old;   
  int pr2_old; 
  int pr3;
  int color=1; //long d1; 
  int mode=1;
  
void main(void){
	static char rxb[RX_BUFFER_SIZE];	static char txb[TX_BUFFER_SIZE];	SCI1_INIT(br19200, txb, TX_BUFFER_SIZE, rxb, RX_BUFFER_SIZE);	EI;
	unsigned char data_1[32] = "start_eeprom";  
 	unsigned char data_2[32] = "mode1_start";
	unsigned char data_3[32] = "mode2_start"; 
    unsigned char data_4[32] = "mode3_start";
    unsigned char can[32] = "can found";
    unsigned char black[32] = "Can is black";
    unsigned char silver[32] = "Can is silver";
    unsigned char release[32] = "can released";
    unsigned char goal[32] = "Goal!";
    eeprom_flash();
    adinit();
    ioinit_MD();
    ituinit_ITU(); //始めに一回だけ
    eeprom_write(data_1);
    wait(1000000);
    while(1){
 
        switch (mode){
 
            case 1:  
                eeprom_write(data_2);
                motor_start(1);
                while(1){
                    pr1=readPr1(); pr2=readPr2(); pr3=readPr3();  SCI1_PRINTF("pr1=%d,pr2=%d,pr3=%d\n",pr1,pr2,pr3);
                    if(color==1){  //銀だったら
                        Pcontrl(1,0,1,pr1,pr2,pr1_old,pr2_old);
                    }
                    else{
                        Pcontrl(1,1,0,pr1,pr2,pr1_old,pr2_old);
                    }
                    if(detect_distance_pr3(pr3)==1){
                        eeprom_write(can);
                        motor_start(1);
                        wait(300000);
                        motor_stop();
                        wait(500000);
                        pr3=readPr3();
                        color=detect_color_pr3(pr3); 
                        if (color == 1){
                            eeprom_write(silver);
                        }
                        else{
                            eeprom_write(black);
                        }
                        motor_turn();    //180度回転
                        wait(1000000);
                        motor_stop();
                        wait(100000);
                        mode=2; 
                        break;
                    }
                    if(detect_silver_pr1_pr2(pr1,pr2)==1){
                        motor_stop();
                        wait(100000);
                        mode=3;
                        break;
                    }
                }
                break;  //case1:を抜ける
​
            case 2:
                eeprom_write(data_3);
	            motor_start(1);
                pr1=readPr1(); pr2=readPr2(); 
                SCI1_PRINTF("pr1=%d,pr2=%d,pr3=%d\n",pr1,pr2,pr3);  //銀だったら
                while(detect_distance_pr3(pr3)==1){      //缶を捨てるまで繰り返す
                    while(detect_silver_pr1_pr2(pr1, pr2) == 0){    //銀テープが来るまで前進
                        pr1=readPr1();//SCI1_PRINTF("pr1=%d\t",pr1);  
			            pr2=readPr2();//SCI1_PRINTF("pr2=%d\t",pr2);
                        if (color == 1){ //銀なら左のPR
                            Pcontrl(1,1,0,pr1,pr2,pr1_old,pr2_old);
                        }
                        else {  //黒なら右のPR
                            Pcontrl(1,0,1,pr1,pr2,pr1_old,pr2_old);
                        }
                    }
                    motor_stop();  //いったん止まる
                    wait(100000);
		            motor_backward_move();
                    wait(1500000);
                    motor_stop();
                    wait(100000);
                    pr1=readPr1();
                    pr2=readPr2();
                    pr3=readPr3();
                }
                eeprom_write(release);
                mode=1; 
	            motor_turn();    //180度回転
                wait(1000000);
                motor_stop();
                wait(100000);
	            break;  //case2:を抜ける
​
            case 3:
                eeprom_write(data_4);
                eeprom_write(goal);
                motor_stop(); 
                wait(100000);
                motor_turn(); 
                while(1);
                break;
        }
    }
}
  
  
