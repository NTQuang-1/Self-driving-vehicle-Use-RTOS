#include <Arduino_FreeRTOS.h>
#include <SoftwareSerial.h>
#include <Servo.h>
#include<task.h>
#include<queue.h>

//khai bao cac chan ket noi
#define trig 8
#define echo 7
#define in1 10
#define in2 13
#define in3 11
#define in4 12

int pos = 0;
char message[4]; // mảng có 4 phần tử
int distance;
int distance1;
int distance2;
int distance3;

//khoi tao servo
Servo myServo;

//khoi tao bluetooth
SoftwareSerial BTSerial(0, 1); //Tx noi voi chan 1, Dx noi voi chan 2
int gioihan = 45;//khoảng cách nhận biết vật 
unsigned long thoigian; 
float khoangcach, khoangcachtruocmat, khoangcachtrai, khoangcachphai;

TaskHandle_t xTask1;
TaskHandle_t xTask2;
TaskHandle_t xTask3;

QueueHandle_t f;
QueueHandle_t r;
QueueHandle_t l;

void dokhoangcach();
void dithang(int duongdi);
void disangtrai();
void disangphai();
void dilui();
void resetdongco();
void quaycbsangphai();
void quaycbsangtrai();
void Task1(void *pvP);//nhận dữ liệu từ người dùng
void Task2(void *pvP);//chế độ tự hành
void Task3(void *pvP);//đo khoảng cách từ 0-180 độ

void setup() 
{
  // put your setup code here, to run once:
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);
  pinMode(trig, OUTPUT);
  pinMode(echo, INPUT);
  Serial.begin(9600);
  BTSerial.begin(9600);
  delay(20);
  Serial.println("khoi tao cong ket noi");
  delay(20);
  Serial.setTimeout(50); //thoi gian cho toi da de nhan du lieu noi tiep
  //dat muc dien ap ban dau cho cac can 
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);
  myServo.attach(9); //khai bao chan cho servo
  myServo.write(90); //dat servo o vi tri 90deg
  delay(20);

  char message[4];
  f = xQueueCreate(1, sizeof(message));         //tao hang doi f
  r = xQueueCreate(1, sizeof(message));         //tao hang doi r
  l = xQueueCreate(1, sizeof(message));         //tao hang doi l

  xTaskCreate(Task1, "Task 1", 128, NULL, 2, &xTask1);
  xTaskCreate(Task2, "Task 2", 128, NULL, 1, &xTask2);
  xTaskCreate(Task3, "Task 3", 128, NULL, 1, &xTask3);
  Serial.println("da khoi tao 3 task");

  vTaskStartScheduler();
}

void Task1(void *pvP) 
{
  char num[4];
  int l,r,f;
  for (;;) 
  {
      Serial.println("Task1 is running");//xong thì xóa
      delay(20);
      while (Serial.available() == 0) {} //chờ dữ liệu available
      String Navigation = Serial.readString();
      Navigation.trim();
      Serial.println(Navigation);
      if (Navigation == "F") 
      {
        Serial.println("di thang");
          dithang();
      }
      else if (Navigation == "B") 
      {
        dilui();
        Serial.println("di lui");

      }
      else if (Navigation == "L") 
      {
        disangtrai();
        Serial.println("di sang trai");
      }
      else if (Navigation == "R") 
      {
        disangphai();
        Serial.println("di sang phai");
      }
      else if (Navigation == "S") 
      {
        dunglai();
        Serial.println("dung lai");
      }
      else if (Navigation == "A") 
      {
        // chế độ tự hành
        vTaskPrioritySet(xTask1, 1);
        vTaskPrioritySet(xTask2, 2);
        Serial.println("che do tu hanh");
      }
    }
}

void Task2(void *pvP) {
  vTaskPrioritySet(xTask3, 3);
  //xe di auto
  for (;;) {
    khoangcachtruocmat = distance;
    khoangcachtrai = distance1;
    khoangcachphai = distance2;
    dichuyen();
  }
}

void Task3(void *pvP) 
{
  //do khoang cach
  for (;;) 
  {
    Serial.println("Task3 is running"); // xong thì xóa

    for (pos = 0; pos <= 180; pos += 1) 
    {
      myServo.write(pos);
      delay(15);
      dokhoangcach();
      distance=khoangcach;
      if(distance<=25)
      {
        Serial.print(distance);
        Serial.println("==========phat hien co vat can phia truoc");
      }
      //lay gia tri khoang cach tai pos = 0 roi in ra man hinh
      if(pos == 0)
      {
        distance1=khoangcach;
        Serial.println(distance1);
        sprintf(message, "%d", distance1);
        if (distance1 > 0) 
        {
          xQueueOverwrite(l, message);
        }
      }

      //lay gia tri khoang cach tai pos = 90 roi in ra man hinh
      else if(pos == 90)
      {
        distance2=khoangcach;
        Serial.println(distance2);
        sprintf(message, "%d", distance2);
        if (distance2 > 0) 
        {
        xQueueOverwrite(f, message);
        }
      }
        
      //lai gia tri khoang cach tai pos = 180 roi in ra man hinh
        else if(pos == 180)
        {
        distance3=khoangcach;
        Serial.println(distance3);
        sprintf(message, "%d", distance3);
        if (distance3 > 0) 
        {
          xQueueOverwrite(r, message);
        }
      }
    }
    for (pos = 180; pos >= 0; pos -= 1) 
    {
      myServo.write(pos);
      delay(15);
      dokhoangcach();

      //hien thi khoang cach khi pos = 180
      if(pos == 180)
      {
        distance1=khoangcach;
        Serial.println(distance1);
        sprintf(message, "%d", distance1);
        if (distance1 > 0) 
        {
          xQueueOverwrite(l, message);
        }
      }

      //hien thi khoang cach khi pos =90
      else if(pos == 90)
      {
        distance2=khoangcach;
        Serial.println(distance2);
        sprintf(message, "%d", distance2);
        if (distance2 > 0) 
        {
          xQueueOverwrite(f, message);
        }          
      }
      
      //hien thi khoang cach khi pos = 0
      else if(pos == 0)
      {
        distance3=khoangcach;
        Serial.println(distance3);
        sprintf(message, "%d", distance3);
        if (distance3 > 0) 
        {
          xQueueOverwrite(r, message);
        }
      }
    }
  }
  vTaskPrioritySet(xTask3, 2);
}

void dichuyen()
{
  char num[4];
  int l;
  int r;
  int f;

 xQueuePeek(l, num, ( TickType_t )0);
  l = atoi(num);
   xQueuePeek(r, num, ( TickType_t )0);
  r = atoi(num);
  dithang();

  xQueuePeek(f, num, ( TickType_t )0);
  f = atoi(num);
  if (f > gioihan || f == 0)
  {
    dithang();
    Serial.println("");
    delay(10);
  }
  if (f <= gioihan)
  {
    dilui();
    Serial.println("xe di lui 300ms");
    delay(1000);
    //xe đứng yên
    dunglai();
    Serial.println("xe dung yen");
    if  (l > r)
    {
      if (l > gioihan || l == 0) 
      {
        //quay xe sang trái
        disangtrai();
        Serial.println("quay xe sang trai");
        delay (50);
      }
      else 
      {
        //quay xe sang phải
        disangphai();
        Serial.println("quay sang phai");
        delay(50);
      }
    }
    else
    {
      if  (r > gioihan || r == 0)
      {
        //quay xe sang phải
        disangphai();
        Serial.println("quay xe sang phai");
        delay (50);
      }
      else 
      {
        //quay xe sangtrai
        disangtrai();
        Serial.println("quay sangtrai");
        delay(30);
      }
    }
  }
}

void dithang()
{
  digitalWrite(in1, 1);
  digitalWrite(in2, 0);
  digitalWrite(in3, 1);
  digitalWrite(in4, 0);

}
void dunglai()
{
  digitalWrite(in1, 0);
  digitalWrite(in2, 0);
  digitalWrite(in3, 0);
  digitalWrite(in4, 0);
  }

void disangphai()
{
  digitalWrite(in1, 1);
  digitalWrite(in2, 0);
  digitalWrite(in3, 0);
  digitalWrite(in4, 1);

}
void disangtrai()
{
  digitalWrite(in1, 0);
  digitalWrite(in2, 1) ;
  digitalWrite(in3, 1);
  digitalWrite(in4, 0);

}

void dilui()
{
  digitalWrite(in1, 0);
  digitalWrite(in2, 1);
  digitalWrite(in3, 0);
  digitalWrite(in4, 1);

}

void dokhoangcach()
{
  digitalWrite(trig, LOW); 
  delayMicroseconds(2);
  digitalWrite(trig, HIGH);  
  delayMicroseconds(10); 
  digitalWrite(trig, LOW); 

  // Đo độ rộng xung HIGH ở chân echo.
  thoigian = pulseIn(echo, HIGH);

  khoangcach = thoigian / 2 / 29.412;
}

void quaycbsangtrai()
{
  myServo.write(180);              // tell servo to go to position in variable 'pos'
  delay(1000);
  dokhoangcach();
  myServo.write(90);              // tell servo to go to position in variable 'pos'
}

void quaycbsangphai()
{
  myServo.write(0);              // tell servo to go to position in variable 'pos'
  delay(1000);
  dokhoangcach();
  myServo.write(90);              // tell servo to go to position in variable 'pos'
}

void resetdongco()
{
  digitalWrite(in1,LOW);
  digitalWrite(in2,LOW);
  digitalWrite(in3,LOW);
  digitalWrite(in4,LOW);
}

void resetservo()
{
   myServo.write(90);
}
void loop() {}
