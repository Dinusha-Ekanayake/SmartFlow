#include <HardwareSerial.h> 
#include <math.h>  // Needed for sqrt() 
#include <ESP32Servo.h> // Include the servo library 
#define RXD2 04  // Change based on your ESP32 wiring 
#define TXD2 02  // Change based on your ESP32 wiring 
#define SERVO_PIN 18 // Change to the GPIO pin where your servo is connected 
#define CONTROL_PIN 19 // GPIO pin to receive signal from ESP Home (change as needed) 
HardwareSerial mySerial(2);  // Using UART2 
Servo trackingServo;  // Create a servo object 
// Servo positions (in degrees) 
const int SERVO_LEFT = 180; 
const int SERVO_MIDDLE = 145; 
const int SERVO_RIGHT = 90; 
int currentServoPos = SERVO_MIDDLE; // Track current position 
// Enhanced multi-target tracking variables 
struct Target { 
bool active; 
unsigned long lastDetectedTime; 
int position; // LEFT, MIDDLE, RIGHT 
f
 loat distance; 
int16_t x; 
}; 
#define TARGET_TIMEOUT 1000 // Time in ms before considering a target lost 
#define POSITION_LEFT 0 
#define POSITION_MIDDLE 1 
#define POSITION_RIGHT 2 
Target targets[3]; // Array to store target information 
unsigned long lastRotationTime = 0; 
const unsigned long ROTATION_INTERVAL = 2000; // Time to wait before switching 
positions (ms) 
int currentRotationTarget = SERVO_MIDDLE; 
int activeTargetCount = 0; 
int targetPositions[3] = {SERVO_LEFT, SERVO_MIDDLE, SERVO_RIGHT}; 
// Function to move servo smoothly to target position 
void moveServoTo(int targetPosition) { 
delay(2000); //stay while for rotate other state 
// Check if control signal is HIGH (rotation enabled) 
if (digitalRead(CONTROL_PIN) == LOW) { 
Serial.println("Control signal LOW - Rotation STOPPED"); 
return; 
} 
// Only move if there's a change in position 
if (targetPosition != currentServoPos) { 
Serial.println("Control signal HIGH - Rotation ENABLED"); 
// Determine direction and step size for smooth motion 
int step = (targetPosition > currentServoPos) ? 1 : -1; 
// Move in small increments for smoother motion 
while (currentServoPos != targetPosition) { 
currentServoPos += step; 
trackingServo.write(currentServoPos); 
delay(20); // Small delay for smooth movement 
} 
} 
} 
// Function to process and display data for each target 
void processTarget(uint8_t *data, int targetNumber) { 
// Extract raw values for X, Y, Speed and distance resolution 
int16_t raw_x = data[0] | (data[1] << 8); 
int16_t raw_y = data[2] | (data[3] << 8); 
int16_t raw_speed = data[4] | (data[5] << 8); 
uint16_t distance_resolution = data[6] | (data[7] << 8);  // Currently unused 
// Process the X coordinate: 
// If highest bit is 0, value is negative; otherwise positive (remove the sign bit) 
int16_t target_x; 
if ((raw_x & 0x8000) == 0) { 
target_x = -raw_x; 
} else { 
target_x = raw_x & 0x7FFF; 
} 
// Process the Y coordinate similarly: 
int16_t target_y; 
if ((raw_y & 0x8000) == 0) { 
target_y = -raw_y; 
} else { 
target_y = raw_y & 0x7FFF; 
} 
// (Optional) Process speed if needed: 
int16_t target_speed; 
if ((raw_speed & 0x8000) == 0) { 
target_speed = -raw_speed; 
} else { 
target_speed = raw_speed & 0x7FFF; 
} 
// Calculate Euclidean distance in mm 
  float distance = sqrt(pow(target_x, 2) + pow(target_y, 2)); 
 
  // Determine if the target is valid (non-zero coordinates) 
  bool target_detected = !(target_x == 0 && target_y == 0); 
 
  if (target_detected) { 
    // Position Classification: use a tighter threshold when close, wider when far 
    String position; 
    int targetPos = POSITION_MIDDLE; // Default position 
    int servoPos = SERVO_MIDDLE; 
     
    if (distance <= 1000) { 
      // When close, threshold is -150 and +150 for left/right detection 
      if (target_x < -150) { 
        position = "LEFT"; 
        targetPos = POSITION_LEFT; 
        servoPos = SERVO_LEFT; 
      } 
      else if (target_x > 150) { 
        position = "RIGHT"; 
        targetPos = POSITION_RIGHT; 
        servoPos = SERVO_RIGHT; 
      } 
      else { 
        position = "MIDDLE"; 
        targetPos = POSITION_MIDDLE; 
        servoPos = SERVO_MIDDLE; 
      } 
    } else { 
      // When farther away, threshold is -500 and +500 
      if (target_x < -500) { 
        position = "LEFT"; 
        targetPos = POSITION_LEFT; 
        servoPos = SERVO_LEFT; 
      } 
      else if (target_x > 500) { 
        position = "RIGHT"; 
        targetPos = POSITION_RIGHT; 
        servoPos = SERVO_RIGHT; 
      } 
      else { 
        position = "MIDDLE"; 
        targetPos = POSITION_MIDDLE; 
        servoPos = SERVO_MIDDLE; 
      } 
    } 
     
    // Update target tracking information 
    targets[targetNumber-1].active = true; 
    targets[targetNumber-1].lastDetectedTime = millis(); 
    targets[targetNumber-1].position = targetPos; 
    targets[targetNumber-1].distance = distance; 
targets[targetNumber-1].x = target_x; 
// Distance classification 
String range = (distance <= 1000) ? "CLOSE" : "FAR"; 
// Print out target information 
Serial.println("--------------------------"); 
Serial.print("Target "); 
Serial.println(targetNumber); 
Serial.print("Position: "); 
Serial.println(position); 
Serial.print("Range: "); 
Serial.println(range); 
Serial.print("Distance: "); 
Serial.print(distance); 
Serial.println(" mm"); 
Serial.print("Raw X: "); 
Serial.println(target_x); 
Serial.print("Raw Y: "); 
Serial.println(target_y); 
Serial.print("Servo Position: "); 
Serial.println(currentServoPos); 
Serial.print("Control Pin State: "); 
Serial.println(digitalRead(CONTROL_PIN) == HIGH ? "HIGH (Rotate)" : "LOW (Stop)"); 
Serial.println("--------------------------"); 
} else { 
// Don't immediately mark as inactive - this will be handled in handleMultiTargetRotation 
// with timeout logic to avoid losing targets due to momentary detection issues 
Serial.print("No target detected for Target "); 
Serial.println(targetNumber); 
} 
} 
void setup() { 
Serial.begin(115200);       
                     //
 Serial Monitor baud rate for debugging 
// Add small delay to ensure ESP32 is ready 
delay(1000);                           
// Serial Monitor baud rate for debugging 
mySerial.begin(256000, SERIAL_8N1, RXD2, TXD2);  // HLK-LD2450 default baud rate 
// Initialize control pin as input with pulldown 
pinMode(CONTROL_PIN, INPUT_PULLDOWN); 
// Initialize servo 
ESP32PWM::allocateTimer(0);  // Allocate timer 0 
trackingServo.setPeriodHertz(50); // Standard 50Hz servo 
trackingServo.attach(SERVO_PIN, 500, 2400); // Adjust min/max pulse width if needed 
// Center the servo at startup 
trackingServo.write(SERVO_MIDDLE); 
currentServoPos = SERVO_MIDDLE; 
currentRotationTarget = SERVO_MIDDLE; 
// Initialize target tracking 
for (int i = 0; i < 3; i++) { 
targets[i].active = false; 
targets[i].lastDetectedTime = 0; 
targets[i].position = POSITION_MIDDLE; 
targets[i].distance = 0; 
targets[i].x = 0; 
} 
Serial.println("HLK-LD2450 Radar with ESP Home Control Initialized"); 
Serial.print("Control pin: "); 
Serial.println(CONTROL_PIN); 
Serial.println("HIGH = Rotate Enabled, LOW = Rotate Disabled"); 
} 
// Function to manage multi-target rotation with improved persistence 
void handleMultiTargetRotation() { 
// Check control signal first - if LOW, don't rotate 
if (digitalRead(CONTROL_PIN) == LOW) { 
return; 
} 
unsigned long currentTime = millis(); 
activeTargetCount = 0; 
  // Check for target timeout and count active targets 
  for (int i = 0; i < 3; i++) { 
    // Mark targets as inactive if not detected for TARGET_TIMEOUT ms 
    if (targets[i].active && (currentTime - targets[i].lastDetectedTime) > TARGET_TIMEOUT) { 
      targets[i].active = false; 
      Serial.print("Target "); 
      Serial.print(i + 1); 
      Serial.println(" lost (timeout)"); 
    } 
     
    if (targets[i].active) { 
      activeTargetCount++; 
    } 
  } 
   
  Serial.print("Active targets: "); 
  Serial.println(activeTargetCount); 
   
  // If no targets, optionally return to middle position 
  if (activeTargetCount == 0) { 
    if (currentServoPos != SERVO_MIDDLE) { 
      moveServoTo(SERVO_MIDDLE); 
      currentRotationTarget = SERVO_MIDDLE; 
    } 
    return; 
  } 
  // If only 1 target is active, track it directly 
  if (activeTargetCount == 1) { 
    for (int i = 0; i < 3; i++) { 
      if (targets[i].active) { 
        int targetServoPos = targetPositions[targets[i].position]; 
        moveServoTo(targetServoPos); 
        currentRotationTarget = targetServoPos; 
        break; 
      } 
    } 
  } 
  // If multiple targets, implement rotation logic 
  else if (activeTargetCount >= 2) { 
    // Only move after interval has passed 
    if (currentTime - lastRotationTime >= ROTATION_INTERVAL) { 
      // Find the next active target position to rotate to 
      int currentPosIndex = -1; 
       
      // Find which position we're currently at 
      for (int i = 0; i < 3; i++) { 
        if (currentRotationTarget == targetPositions[i]) { 
          currentPosIndex = i; 
          break; 
        } 
      } 
       
      // Find the next position with an active target 
      int nextPosition = -1; 
      for (int offset = 1; offset <= 3; offset++) { 
        int checkIndex = (currentPosIndex + offset) % 3; 
        bool positionHasTarget = false; 
         
        // Check if any target is in this position 
        for (int t = 0; t < 3; t++) { 
          if (targets[t].active && targets[t].position == checkIndex) { 
            positionHasTarget = true; 
            break; 
          } 
        } 
         
        if (positionHasTarget) { 
          nextPosition = targetPositions[checkIndex]; 
          break; 
        } 
      } 
       
      // If we found a position with a target, move there 
      if (nextPosition != -1) { 
        moveServoTo(nextPosition); 
        currentRotationTarget = nextPosition; 
        lastRotationTime = currentTime; 
         
Serial.print("Rotating between targets, now at position: "); 
Serial.println(currentRotationTarget); 
} 
} 
} 
} 
void loop() { 
// Wait until a full frame of 32 bytes is available 
if (mySerial.available() >= 32) { 
uint8_t header[4]; 
mySerial.readBytes(header, 4); 
// Check for the expected header pattern: AA FF 03 00 
if (header[0] == 0xAA && header[1] == 0xFF && header[2] == 0x03 && header[3] == 0x00) { 
uint8_t dataBuffer[28];  // Remaining 28 bytes of the frame 
mySerial.readBytes(dataBuffer, 28); 
// Process target 1: bytes 0 to 7 
processTarget(&dataBuffer[0], 1); 
// Process target 2: bytes 8 to 15 
processTarget(&dataBuffer[8], 2); 
// Process target 3: bytes 16 to 23 
processTarget(&dataBuffer[16], 3); 
// Now handle the multi-target rotation based on targets detected in this frame 
handleMultiTargetRotation(); 
// (Optional) The remaining 4 bytes can be used for tail or reserved data. 
} else { 
// If header doesn't match, read one byte to re-sync the frame 
mySerial.read(); 
} 
} 
delay(50);  // Small delay to stabilize readings 
} 