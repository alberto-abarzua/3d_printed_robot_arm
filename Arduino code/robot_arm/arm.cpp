#include "Arduino.h"
#include "arm.h"


#define motorInterfaceType 1

const int ACC= 10000; // Accuracy of the angles received.
 Sensor::Sensor(int new_pin){
     pin = new_pin;
 }


Joint::Joint(double a_ratio,bool a_inverted,int a_homing_dir,int a_offset,int joint_num_steppers){
    jn_steppers = joint_num_steppers;

    motors =(AccelStepper **) malloc(sizeof (AccelStepper *)*joint_num_steppers);
    sensor = (Sensor *) malloc(sizeof(Sensor *));
    for (int i =0;i<jn_steppers;i++) motors[i]=NULL;
    ratio = a_ratio;
    inverted = a_inverted;
    homing_dir = a_homing_dir;
    offset = a_offset;
    angle =0;
    position =0;
}


void Joint::create_motor(int step_pin,int dir_pin){
    int idx = 0;
    while(motors[idx]!=NULL) idx++;
    motors[idx] = new AccelStepper(motorInterfaceType, step_pin, dir_pin);
    
 }

 void Joint::motor_setup(){
     double mult = (5.0/8.0)*6.0;
     for (int i=0;i<jn_steppers;i++){
         motors[i]->setPinsInverted(inverted,false,false);
         motors[i]->setMaxSpeed(mult*2.0*ratio*MICRO_STEPPING);
         motors[i]->setSpeed(mult*ratio*MICRO_STEPPING);
         motors[i]->setAcceleration(mult*30.0*ratio*MICRO_STEPPING);
         
     }

 }
void Joint::create_sensor(int pin){
    sensor = new Sensor(pin);
 }

void Joint::show(){
    Serial.print("Joint ");
    Serial.print(index);
    Serial.print("  nmotors: ");
    Serial.println(jn_steppers);
}

Arm::Arm(int n_joints){
    num_joins = n_joints;
    joints = (Joint **)malloc(sizeof(Joint *)*n_joints);
    sensors =(Sensor **) malloc(sizeof(Sensor *)*n_joints);
    l_positions = (long *) malloc(sizeof(long)*n_joints);
    for(int i =0;i<num_joins;i++)  l_positions[i]=0;
    steppers = new MultiStepper();
    idx=0;
    m_idx =0;
    num_motors =0;


}
void Arm::register_joint(Joint* joint){
    joints[idx] = joint;
    joints[idx]->index = idx;
    idx++;
}
void Arm::build_joints(){
    for (int i =0;i<num_joins;i++){
        num_motors+= joints[i]->jn_steppers; //First count the amount of motors
    }
    //We create an array to store them
    motors =(AccelStepper **) malloc(sizeof(AccelStepper *)*num_motors);
    int spot= 0;
    for (int i =0;i<num_joins;i++){

        joints[i]->motor_setup();
        for (int j =0;j<joints[i]->jn_steppers;j++){
            motors[spot] = joints[i]->motors[j];
            steppers->addStepper(*motors[spot]);
            spot ++;
        }
    }

}

void Arm::show(){
    Serial.print("Num motors ");
    Serial.print(num_motors);
    Serial.println("  -  ");
    for(int i =0;i<num_joins;i++){
        joints[i]->show();
    }
    Serial.println("");

}

void Arm::create_gripper(int pin){
    gripper = new Servo();
    gripper->attach(pin);
}

void Arm::set_gripper_angle(int val){
    gripper->write(val);
}

void Joint::add_angle(long val){
    angle+=val;
    position = (long) ((ratio*angle*100*MICRO_STEPPING)/(PI*ACC));
    for(int i =0;i<jn_steppers;i++){
        motors[i]->moveTo(position);
    }
}

void Arm::run(){
    for (int i =0;i<num_motors;i++){
        motors[i]->run();
    }
}
void Arm::add(long * args){
    for(int i =0;i<num_joins;i++){
        joints[i]->add_angle(args[i]);
        l_positions[i] = (joints[i]->position);

    }

}
void Arm::show_pos(){
    Serial.print("Arms Positions: ");
    Serial.print(" ");
    for (int i =0;i<num_joins;i++){
        Serial.print(l_positions[i]);
        Serial.print(" ");

    }
    Serial.print("Arms angles: ");
    Serial.print(" ");
    for (int i =0;i<num_joins;i++){
        Serial.print(joints[i]->angle);
        Serial.print(" ");

    }
    Serial.print("Motors Positions ");
    Serial.print(" ");
    for (int i =0;i<num_joins;i++){
        Serial.print(motors[i]->currentPosition());
        Serial.print(" ");

    }




}

bool Sensor::read(){
    return !digitalRead(pin);
}

void Joint::launch_home(){
    add_angle(3.1415*ACC*homing_dir);
    for (int i =0 ;i<jn_steppers;i++){
      motors[i]->moveTo(position);
    }
}

void Joint::home(MultiStepper * m){
    int times_activated = 0;
    int tolerance = 7;
    while(true){
        if (sensor->read()){
            times_activated++;
        }else{
            times_activated =0;
        }

        if (times_activated>=tolerance){
            for (int i =0 ;i<jn_steppers;i++){
                motors[i]->setCurrentPosition(offset*homing_dir*-1);
                position=0;
                angle=0;
                motors[i]->moveTo(position);
            }
                break;
        }

        for (int i =0 ;i<jn_steppers;i++){
                motors[i]->run();
            }
    }
}

void Arm::home(){
    for(int i =0;i< num_joins;i++){
        joints[i]->launch_home();
    }
    for(int i =0;i< num_joins;i++){
        joints[i]->home(steppers);
    }
    run();

}

void Arm::show_sensors(){
    Serial.print("Sensors: ");
    for (int i =0;i<num_joins;i++){
        Serial.print(" ");
        Serial.print("S");
        Serial.print(i+1);
        Serial.print(" ");
        Serial.print(joints[i]->sensor->read()?"ACTIVE" : "OFF");
    }
}