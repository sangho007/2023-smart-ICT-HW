#include <Arduino.h>
#include <Car_Library.h>
#include <Servo.h>    // Servo 라이브러리 헥사 선언

// 라즈베리파이와 통신
String inputString = "";
bool stringComplete = false;

/**********************************************
 * 액츄에이터 테스트, 수위센서, 서보모터, 3색 LED, 스피커
***********************************************/

// 액츄에이터 모터
int motorL1  = 42; // 모터 드라이버 IN1 green
int motorL2  = 43; // 모터 드라이버 IN2 yellow

// 수위센서
int water_sensor_out = A0; // 외부 수위센서 A0핀 설정
int water_height_out = 0; // loop에서 사용할 변수 설정
int water_sensor_inner = A1; // 내부 수위센서 A0핀 설정
int water_height_inner = 0; // loop에서 사용할 변수 설정

// 워터펌프 (선 1개만 써도 되는데 그냥 사골 드라이버 쓰자. 결국 loop에서 motor_forward 돌리는데 IN1쪽에 초록이든 노란색이든 하나만 꽃으면 된다.)
int pump1  = 50; // 모터 드라이버 IN1 green 
int pump2  = 51; // 모터 드라이버 IN2 yellow

// 서보모터 SG90
Servo servo_out; // servo_out 이름의 서보 인스턴스 생성
Servo servo_inner; // servo_inner 이름의 서보 인스턴스 생성
int servoPin_Out = 22; // 앞쪽 문 signal pin
int servoPin_Inner = 23; // 뒤쪽 문 signal pin
int angle_Out = 0; //초기 각도값 설정  
int angle_Inner = 0; //초기 각도값 설정  

// 3색 LED 
#define LED_Red 36 
#define LED_Green 37 

// --------------------------------------------스피커-----------------------------------------------
// 스피커 모듈
#define SPEAKER_PIN 2

//쉼표
int rest =0;

// 계이름을 순서대로 배열로 선언
int melody[]= {262, 294, 330, 349, 392, 440, 494, 523};

// 연주 유지시간을 배열로 선언
//int noteDurations[] = {1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000};
int noteDurations[] = {500, 500, 500, 500, 500, 500, 500, 500};

unsigned long previousMillis = 0;
unsigned long noteMillis = 0;
int currentNote = -1;
boolean playing = false;
// --------------------------------------------스피커-----------------------------------------------

// 각종 변수
//int Actuator_flag = 0; // 액츄에이터 동작 모드 flag.
int Servo_flag = 0; // 문 열림 닫힌 동작 모드 flag. 

/*-----------------------------------------------------------------------
                      SETUP
------------------------------------------------------------------------*/
void setup() {
  // Interrupt resister setting.
//  TCCR2A = 2;            // Set CTC mode.  Same as TCCR2A = _BV(WGM21);
//  TCCR2B = 6;            // Prescaler to divide by 32 (CS21 and CS20 only) 4-> by 64
//  TCNT2 = 0;             // Clear the counter
//  OCR2A = 249;           // Set for 1 msec rate
//  TIMSK2 = 2;            // Set OCIE2A to begin counting with Compare A
//  TCCR1B = TCCR1B & 0b11111000 | 1;  // set 31KHz PWM to prevent motor noise
  
  // 통신
  Serial.begin(9600);   // 시리얼 통신 시작, 통신 속도 설정
  
  // GPIO Pin 설정 - 액츄에이터 
  pinMode(motorL1, OUTPUT);  
  pinMode(motorL2, OUTPUT); 
  motor_forward(motorL1, motorL2, 250); // 초기화하면 액츄에이터 시작 위치는 사이니지 바닥이므로 액츄에이터 다 나와있는 상태를 표시하므로 . 
  delay(5000);
  motor_hold(motorL1, motorL2);

  // GPIO Pin 설정 - 수위 센서
  pinMode(water_sensor_out, INPUT); // 외부에 부착하는 수위 센서
  pinMode(water_sensor_inner, INPUT); // 내부에 부착하는 수위 센서

  // GPIO Pin 설정 - 워터 펌프
  pinMode(pump1, OUTPUT); // 외부에 부착하는 수위 센서
  pinMode(pump2, OUTPUT); // 내부에 부착하는 수위 센서

  // GPIO Pin 설정 - 서보 모터
  servo_out.attach(servoPin_Out);
  servo_inner.attach(servoPin_Inner);
  servo_out.write(angle_Out);
  servo_inner.write(angle_Inner);
//  pinMode (servoPin_Out, OUTPUT);  // 모터 제어핀을 출력으로 설정
//  pinMode (servoPin_Inner, OUTPUT);  // 모터 제어핀을 출력으로 설정

  // GPIO Pin 설정 - 3색 LED 빨강과, 초록만 사용 예정. 
  pinMode(LED_Red, OUTPUT);
  pinMode(LED_Green, OUTPUT);

  // GPIO Pin 설정 - 스피커
  pinMode(SPEAKER_PIN,OUTPUT); //  핀 2번을 출력 설정
  
  // flag 초기화
  Servo_flag = 0; // 문 열림 닫힌 동작 모드 flag. 
}

/*-----------------------------------------------------------------------
                      INTERUUPT
-------------------------------------------------------------------------*/
volatile int counter = 0; // 인터럽트 카운터 추가
const int read_interval = 100; // 읽기 간격 설정 (read_interval = n인 경우 n밀리초 마다 읽음)

//ISR(TIMER2_COMPA_vect)
//{
//  counter++; // 인터럽트 카운터 증가
//
//  // 주어진 간격마다 가변 저항 값을 읽음
//  if (counter >= read_interval) {
//    water_height_out = analogRead(water_sensor_out) / 4; // 수위센서값을 읽어 변수에 저장 (나중에 디지털 적으로 사용할 수 있으므로 4로 나눠둠.)
//    water_height_inner = analogRead(water_sensor_inner) / 4; // 수위센서값을 읽어 변수에 저장 (나중에 디지털 적으로 사용할 수 있으므로 4로 나눠둠.)
//    // 수위 센서 측정해본 결과 : 조금만 담가도 140 정도 찍힘. 중간쯤 담그면 165 정도. 끝까지 담그면 170정도. 싼 센서니까 이해하고 넘어가자. 
//     
//    counter = 0; // 카운터 초기화
//    
//    // Serial 모니터로 출력
//    Serial.println((String) "외부 수위 센서 높이 = " + water_height_out);// 저장된 센서값을 시리얼 모니터에 출력
//    Serial.println((String) "내부 수위 센서 높이 = " + water_height_inner);// 저장된 센서값을 시리얼 모니터에 출력
//  }
//}

/*--------------------------------------------------
                       LOOP
---------------------------------------------------*/
void loop() {
  water_height_out = analogRead(water_sensor_out) / 4; // 수위센서값을 읽어 변수에 저장 (나중에 디지털 적으로 사용할 수 있으므로 4로 나눠둠.)
  delay(1);
  water_height_inner = analogRead(water_sensor_inner) / 4; // 수위센서값을 읽어 변수에 저장 (나중에 디지털 적으로 사용할 수 있으므로 4로 나눠둠.)
  delay(1);
//  Serial.println((String) "외부 수위 센서 높이 = " + water_height_out);// 저장된 센서값을 시리얼 모니터에 출력
//  Serial.println((String) "내부 수위 센서 높이 = " + water_height_inner);// 저장된 센서값을 시리얼 모니터에 출력

  pump_start(water_height_inner, water_height_out); // 내,외부 수위센서의 값에 따라 워터펌프 동작.
  delay(1);
  Actuator_control(water_height_inner); // 내부 수위센서의 값에 따른 액츄에이터 동작.
  delay(1);
  
  // 라즈베리파이에서 넘어오는 시리얼 통신 값. 
  if (stringComplete) {
    handleCommand(inputString); // 수신 명령에 따른 동작 실행. 
    inputString = "";
    stringComplete = false;
  }
  
}

/*----------------------------------------------------------------------------------------------------
                      FUNCTION
-----------------------------------------------------------------------------------------------------*/
// 액츄에이터 동작 함수
void Actuator_control(int water_height_inner){
  if(water_height_inner > 140){
//      Serial.println("Actuator Motor backward ----------- UP-UP-UP-UP-UP");
      motor_backward(motorL1, motorL2, 250);  // Actuator up
//      delay(3000);
//      motor_hold(motorL1, motorL2);
    }
  else if(water_height_inner < 10){
//      Serial.println("Actuator Motor forward -------------- Down-Down-Down-Down");
      motor_forward(motorL1, motorL2, 250); // Actuator down
//      delay(500);
//      motor_hold(motorL1, motorL2);
    }
}

// 서보 모터 동작 함수
void servo_control(int angle_Out, int angle_Inner){
    servo_out.write(angle_Out); // 바깥문 
    servo_inner.write(angle_Inner); // 안쪽문
}

// 워터 펌프 동작 함수
void pump_start(int water_height_inner, int water_height_out){
    // 내부에 물이 찼으면서, 외부 수위센서의 값이 작을 때 워터 펌프가 작동. 
    if(water_height_inner > 140 && water_height_out < 100){
          // Serial.println("Water pump start------------------------");
          motor_forward(pump1, pump2, 255);  // Car_Library 내장 함수 노란색 점퍼선 만 50번 핀에 꽂자. 
      }
    else{
        motor_hold(pump1, pump2);
    }
}

// 3색 LED 색상 설정 함수. -> RED, GREEN 색상만 사용 예정.
void setRGB(int red_value, int green_value)
{
  analogWrite(LED_Red, red_value);
  analogWrite(LED_Green, green_value);
}

// 스피커 모듈 음계 재생 함수. 
// for문으로 돌리는 가장 간단한 함수. (단점 : for문으로 돌리기에 for문 동작 동안 다른 작업 수행 불가.)
//void playMelody() {
//  for (int i = 0; i < sizeof(melody) / sizeof(melody[0]); i++) {
//    int noteDuration = noteDurations[i];
//    tone(SPEAKER_PIN, melody[i], noteDuration);
//
//    int pauseBetweenNotes = noteDuration * 1.3;
//    delay(pauseBetweenNotes);
//    noTone(SPEAKER_PIN);
//  }
//}

// 맬로디를 재생하면서 다른 작업도 동시에 수행하도록 보완한 함수. 
void playMelody() {
  unsigned long currentMillis = millis();

  if (!playing && currentMillis - previousMillis >= 3000) {
    playing = true;
    currentNote = 0;
  }

  if (playing && currentMillis - noteMillis >= (noteDurations[currentNote] * 1.3)) {
    if (currentNote >= 0) {
      noTone(SPEAKER_PIN);
    }

    if (currentNote < sizeof(melody) / sizeof(melody[0])) {
      tone(SPEAKER_PIN, melody[currentNote], noteDurations[currentNote]);
      noteMillis = currentMillis;
      currentNote++;
    } 
    else {
      playing = false;
      previousMillis = currentMillis;
      currentNote = -1;
    }
  }
}

// 라즈베리파이와 시리얼 통신
void serialEvent() {
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    if (inChar == '\n') {
      stringComplete = true;
    } else {
      inputString += inChar;
    }
  }
}

void handleCommand(String command) {
  if (command == "open") {
    Serial.println(command); 
    servo_control(90, 90); // Servo open - Door open
  } 
  else if (command == "close") {
    Serial.println(command); 
    servo_control(0, 0); // // Servo close - Door close
  }
  else if (command == "g") {
    Serial.println(command); 
    setRGB(0,255); // green 이라는 명령이 넘어오면 red 점등.
  }
  else if (command == "r") {
    Serial.println(command); 
    setRGB(255,0); // red 이라는 명령이 넘어오면 red 점등.
  }
  else { 
    Serial.println("Unknown command");
  }
}
