#include <IRLibAll.h>

//#define IR_INFO
#define JOYSTICK
//#define TANK

#define LEFT_UP_COMMAND     0x2FDD827
#define LEFT_DOWN_COMMAND   0x2FDF807
#define RIGHT_UP_COMMAND    0x2FD58A7
#define RIGHT_DOWN_COMMAND  0x2FD7887

#define LEFT_COMMAND        0x2FD7887
#define RIGHT_COMMAND       0x2FD58A7
#define FORWARD_COMMAND     0x2FDD827
#define BACKWARD_COMMAND    0x2FDF807
#define STOP_COMMAND        0x2FD847B
#define HORN_COMMAND        0x2FD08F7

#define IR_PIN      3
#define LED_LEFT    2
#define LED_RIGHT   6
#define MOTOR_PLUS  5
#define MOTOR_MINUS 4
#define HORN_PIN    9

#define MOTOR_LEFT  1
#define MOTOR_RIGHT 2

#define STEPS_COUNT 10

int speed_index = 0;
int angle_index = 0;

int left_index  = 0;
int right_index = 0;

IRrecv myReceiver(IR_PIN);
IRdecode myDecoder;   

void setup()
{
    pinMode(LED_LEFT,    OUTPUT);
    pinMode(LED_RIGHT,   OUTPUT);
    pinMode(MOTOR_MINUS, OUTPUT);
    pinMode(MOTOR_PLUS,  OUTPUT);
    pinMode(HORN_PIN,    OUTPUT);

    Serial.begin(9600);
    myReceiver.enableIRIn();

    avari(1);
    back_motor_move(0);
    motor_move(MOTOR_LEFT,  0);
    motor_move(MOTOR_RIGHT, 0);

    delay(1000);
    avari(0);
}

void avari(int state) {
    digitalWrite(LED_LEFT,  state);
    digitalWrite(LED_RIGHT, state);
}

void motor_move(int motor, int speed)
{
    // ??
    if (speed > 0) {
        speed = map(speed, 0, STEPS_COUNT, 30, 255);
        Serial.write(0xfe);
        Serial.write(motor);
        Serial.write(0xf0);
        Serial.write(speed);
    }
    else if (speed < 0){
        speed = map(-speed, 0, STEPS_COUNT, 30, 255);
        Serial.write(0xfe);
        Serial.write(motor);
        Serial.write(0xba);
        Serial.write(speed);
    } else {
        speed = 0;
        Serial.write(0xfe);
        Serial.write(motor);
        Serial.write(0xf0);
        Serial.write(speed);
    }
}

void back_motor_move(int speed) {
    if (speed > 0) {
        digitalWrite(MOTOR_MINUS, 1);
        digitalWrite(MOTOR_PLUS, 0);
    } else if (speed < 0) {
        digitalWrite(MOTOR_MINUS, 0);
        digitalWrite(MOTOR_PLUS, 1);
    } else {
        digitalWrite(MOTOR_PLUS, 0);
        digitalWrite(MOTOR_MINUS, 0);
    }
}

void drive()
{
    back_motor_move(speed_index);
    if (speed_index < 0) {
        if (angle_index < 0) {
            motor_move(MOTOR_LEFT, speed_index - angle_index);
            motor_move(MOTOR_RIGHT, speed_index);
        } else {
            motor_move(MOTOR_LEFT, speed_index);
            motor_move(MOTOR_RIGHT, speed_index + angle_index);
        }
    } else if (speed_index > 0) {
        if (angle_index < 0) {
            motor_move(MOTOR_LEFT, speed_index + angle_index);
            motor_move(MOTOR_RIGHT, speed_index);
        } else {
            motor_move(MOTOR_LEFT, speed_index);
            motor_move(MOTOR_RIGHT, speed_index - angle_index);
        }
    } else {
        motor_move(MOTOR_LEFT, 0);
        motor_move(MOTOR_RIGHT, 0);
    }
}

void drive_tank() {
    motor_move(MOTOR_RIGHT, right_index);
    motor_move(MOTOR_LEFT, left_index);

    if (left_index != 0 || right_index != 0) { 
        if (abs(left_index) < abs(right_index) && right_index < 0 || abs(left_index) > abs(right_index) && left_index < 0)
            back_motor_move(-1);
        else 
            back_motor_move(1);
    } else {
        back_motor_move(0);
    }
    
}

#ifdef IR_INFO
void loop()
{
    if (myReceiver.getResults())
    {
        avari(1);
        myDecoder.decode();
        myDecoder.dumpResults(true);      
        Serial.print(myDecoder.value, HEX);
        Serial.print("\n");
        myReceiver.enableIRIn();
    }
    delay(500);
    avari(0);
}
#endif

#ifdef JOYSTICK
void loop()
{
    if (myReceiver.getResults())
    {
        myDecoder.decode();
        switch (myDecoder.value) {
            case LEFT_COMMAND:
                avari(1);
                //digitalWrite(LED_LEFT, 1);
                if (angle_index > -STEPS_COUNT) {
                    angle_index--;
                    drive();
                }
                break;
            case RIGHT_COMMAND:
                //digitalWrite(LED_RIGHT, 1);
                avari(1);
                if (angle_index < STEPS_COUNT) {
                    angle_index++;
                    drive();
                }
                break;
            case FORWARD_COMMAND:
                avari(1);
                if (speed_index < STEPS_COUNT) {
                    speed_index++;
                    drive();
                }
                break;
            case BACKWARD_COMMAND:
                avari(1);
                if (speed_index > -STEPS_COUNT) {
                    speed_index--;
                    drive();
                }
                break;
            case STOP_COMMAND:
                avari(1);
                speed_index = 0;
                angle_index = 0;
                drive();
                break;
           case HORN_COMMAND:
                avari(1);
                analogWrite(HORN_PIN, 31);
                break;
        }
        myReceiver.enableIRIn();
    }
    delay(500);
    analogWrite(HORN_PIN, 0);
    avari(0);
}
#endif

#ifdef TANK
void loop()
{
    if (myReceiver.getResults())
    {
        myDecoder.decode();
        switch (myDecoder.value) {
            case LEFT_UP_COMMAND:
                avari(1);
                if (left_index < STEPS_COUNT) {
                    left_index++;
                    drive_tank();
                }
                break;
            case LEFT_DOWN_COMMAND:
                avari(1);
                if (left_index > -STEPS_COUNT) {
                    left_index--;
                    drive_tank();
                }
                break;
            case RIGHT_UP_COMMAND:
                avari(1);
                if (right_index < STEPS_COUNT) {
                    right_index++;
                    drive_tank();
                }
                break;
            case RIGHT_DOWN_COMMAND:
                avari(1);
                if (right_index > -STEPS_COUNT) {
                    right_index--;
                    drive_tank();
                }
                break;
            case STOP_COMMAND:
                avari(1);
                left_index = 0;
                right_index = 0;
                drive_tank();
                break;
           case HORN_COMMAND:
                avari(1);
                analogWrite(HORN_PIN, 31);
                break;
        }
        myReceiver.enableIRIn();
    }
    delay(500);
    avari(0);
    analogWrite(HORN_PIN, 0);
}
#endif 
