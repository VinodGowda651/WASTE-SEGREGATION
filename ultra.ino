#include <Stepper.h>
#include <Servo.h>

// Define pins
const int metalSensorPin = 2;    // Metal detector sensor
const int rainSensorPin = 3;     // Rain sensor
const int irSensorPin = 4;       // IR sensor
const int ledPin = 13;           // LED output pin
const int servoPin = 7;          // Servo control pin

// Define stepper motor parameters
const int stepsPerRevolution = 2048;  // Change this based on your stepper motor
// Initialize stepper motor on pins 8, 9, 10, 11
Stepper myStepper(stepsPerRevolution, 8, 10, 9, 11);

// Create servo object
Servo myServo;

// Sensor states
int metalState = 0;
int rainState = 0;
int irState = 0;

// Debouncing and state management variables
unsigned long lastMetalDetectionTime = 0;
unsigned long lastRainDetectionTime = 0;
const unsigned long SENSOR_COOLDOWN_PERIOD = 5000;  // 5 seconds cooldown
bool isProcessingMovement = false;

// Debouncing parameters
const int REQUIRED_CONSECUTIVE_READINGS = 3;
const int READING_INTERVAL = 50;  // Time between readings in milliseconds

// Sensor reading counters
int consecutiveMetalReadings = 0;
int consecutiveRainReadings = 0;

// Function to check if a sensor is consistently triggered
bool isSensorTriggered(int sensorPin, int &consecutiveReadings) {
    consecutiveReadings = 0;
    for (int i = 0; i < REQUIRED_CONSECUTIVE_READINGS; i++) {
        if (digitalRead(sensorPin) == HIGH) {
            consecutiveReadings++;
        }
        delay(READING_INTERVAL);
    }
    return consecutiveReadings == REQUIRED_CONSECUTIVE_READINGS;
}

// Function to rotate stepper and return to home
void rotateAndReturn(int degrees) {
    int steps = (stepsPerRevolution / 360) * degrees;
    
    // Move to desired position
    Serial.print("Moving to ");
    Serial.print(degrees);
    Serial.println(" degrees");
    myStepper.step(steps);
    delay(1000);
    
    // Return to home position
    Serial.println("Returning to home position");
    myStepper.step(-steps);
    delay(1000);
}

// Function to control servo movement
void operateServo() {
    myServo.write(90);    // Open to 90 degrees
    delay(1000);
    myServo.write(0);     // Return to closed position
    delay(1000);
}

void setup() {
    // Set pin modes
    pinMode(metalSensorPin, INPUT);
    pinMode(rainSensorPin, INPUT);
    pinMode(irSensorPin, INPUT);
    pinMode(ledPin, OUTPUT);
    
    // Attach servo to pin
    myServo.attach(servoPin);
    myServo.write(0);
    
    // Set stepper speed (RPM)
    myStepper.setSpeed(10);
    
    // Initialize serial communication
    Serial.begin(9600);
    Serial.println("System initialized");
}

void handleMetalDetection() {
    unsigned long currentTime = millis();
    
    if (currentTime - lastMetalDetectionTime >= SENSOR_COOLDOWN_PERIOD) {
        if (isSensorTriggered(metalSensorPin, consecutiveMetalReadings)) {
            Serial.println("Metal detected consistently!");
            isProcessingMovement = true;
            digitalWrite(ledPin, HIGH);
            
            rotateAndReturn(120);
            operateServo();
            
            digitalWrite(ledPin, LOW);
            lastMetalDetectionTime = currentTime;
            isProcessingMovement = false;
            
            Serial.println("Metal detection complete - Ready for next detection");
        }
    } else {
        Serial.println("Metal sensor cooling down...");
    }
}

void handleRainDetection() {
    unsigned long currentTime = millis();
    
    if (currentTime - lastRainDetectionTime >= SENSOR_COOLDOWN_PERIOD) {
        if (isSensorTriggered(rainSensorPin, consecutiveRainReadings)) {
            Serial.println("Rain detected consistently!");
            isProcessingMovement = true;
            digitalWrite(ledPin, HIGH);
            
            rotateAndReturn(90);
            operateServo();
            
            digitalWrite(ledPin, LOW);
            lastRainDetectionTime = currentTime;
            isProcessingMovement = false;
            
            Serial.println("Rain detection complete - Ready for next detection");
        }
    } else {
        Serial.println("Rain sensor cooling down...");
    }
}

void loop() {
    // Read all sensors
    metalState = digitalRead(metalSensorPin);
    rainState = digitalRead(rainSensorPin);
    irState = digitalRead(irSensorPin);
    
    // Handle Metal Detection
    if (metalState == HIGH && !isProcessingMovement) {
        handleMetalDetection();
    }
    
    // Handle Rain Detection
    else if (rainState == HIGH && !isProcessingMovement) {
        handleRainDetection();
    }
    
    // Handle IR Detection
    else if (irState == HIGH && !isProcessingMovement) {
        Serial.println("IR detected!");
        digitalWrite(ledPin, HIGH);
        delay(1000);
        digitalWrite(ledPin, LOW);
    }
    
    delay(50);  // Small delay between sensor checks
}
