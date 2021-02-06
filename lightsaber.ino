#include <Adafruit_NeoPixel.h>
#include <Wire.h>
#include <MPU6050.h>

#define PIN           2
#define NUMPIXELS     48
#define color         3

#define VAL_SUB 1.0f
#define VAL_GAIN 64.0f
#define VAL_DAMP 0.9f

#define VAL_LED_STEP 64
#define VAL_GLOW_THRESHOLD NUMPIXELS * VAL_LED_STEP

Vector normAccel, prevAccel , dif, sum;

Adafruit_NeoPixel pixels  = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
MPU6050 mpu;

byte LED_COLOR_R[NUMPIXELS];
byte LED_COLOR_G[NUMPIXELS];
byte LED_COLOR_B[NUMPIXELS];

void setup() {
  pixels.begin();
  Serial.begin(115200);

  Serial.println("Initialize MPU6050");

  while (!mpu.begin(MPU6050_SCALE_2000DPS, MPU6050_RANGE_2G))
  {
    Serial.println("Could not find a valid MPU6050 sensor, check wiring!");
    delay(500);
  }

  // If you want, you can set accelerometer offsets
  // mpu.setAccelOffsetX();
  // mpu.setAccelOffsetY();
  // mpu.setAccelOffsetZ();

  checkSettings();
}

void loop() {

  ReadSensor();

  MapSensorToHSV();
  
  print_values();

 
  delay(10);
}



void MapSensorToRGB()
{
  MapLeds(sum.XAxis, LED_COLOR_R);
  MapLeds(sum.YAxis, LED_COLOR_G);
  MapLeds(sum.ZAxis, LED_COLOR_B);

  for (int i = 0; i < NUMPIXELS; i++) {
    pixels.setPixelColor(i, LED_COLOR_R[i], LED_COLOR_G[i], LED_COLOR_B[i]);
  }

  pixels.show();
}

void MapSensorToHSV()
{
  float alfa,h ;
  uint16_t  hue = 0;
  byte value = 0;
  byte saturation;
  if (  sum.YAxis == 0)
  {
    alfa = 0;
  }
  else
  {
    alfa = sum.XAxis;
    alfa = atan(alfa / sum.YAxis) *2;
  }


  hue = ((alfa)/PI) * 65535;
  saturation = 255;
  value = (sum.XAxis + sum.YAxis) / 50;

  Serial.print(hue);
  Serial.print(" s: ");
  Serial.print(saturation);
  Serial.print(" v: ");
  Serial.print(value);
  uint32_t rgbcolor = pixels.ColorHSV(hue, saturation, value);
  //pixels.fill(rgbcolor);
  SetLeds(sum.XAxis + sum.YAxis, rgbcolor);
  pixels.show();
}


void ReadSensor()
{
  normAccel = mpu.readNormalizeAccel();

  dif.XAxis = normAccel.XAxis - prevAccel.XAxis;
  dif.YAxis = normAccel.YAxis - prevAccel.YAxis;
  dif.ZAxis = normAccel.ZAxis - prevAccel.ZAxis;

  sum.XAxis += abs(dif.XAxis) * VAL_GAIN;
  sum.YAxis += abs(dif.YAxis) * VAL_GAIN;
  sum.ZAxis += abs(dif.ZAxis) * VAL_GAIN;
  if (sum.XAxis > 0)
  {
    sum.XAxis -= VAL_SUB;
    sum.XAxis *= VAL_DAMP;
  }
  if (sum.YAxis > 0)
    sum.YAxis -= VAL_SUB;
  sum.YAxis *= VAL_DAMP;
  if (sum.ZAxis > 0)
    sum.ZAxis -= VAL_SUB;
  sum.ZAxis *= VAL_DAMP;


  prevAccel = normAccel;
}

void SetLeds(long v, uint32_t clr) {
  long val = v;
  if ( v > 255 * NUMPIXELS)
    val = 255 * NUMPIXELS;

    int nbr_of_led = v / VAL_LED_STEP;
    int nbr_mid_led = NUMPIXELS / 2;
    
    for (int i = 0; i < NUMPIXELS ; i++)
    {
      if ( i > (nbr_mid_led - (nbr_of_led/2)) &&  i < (nbr_mid_led + (nbr_of_led/2)))
        pixels.setPixelColor(i, clr); 
      else
        pixels.setPixelColor(i, 0); 
    }

}

void MapLeds(int v, byte * led) {
  int val = v;
  if ( v > 255 * NUMPIXELS)
    val = 255 * NUMPIXELS;

  if (v < VAL_GLOW_THRESHOLD) {
    int nbr_of_led = v / VAL_LED_STEP;
    for (int i = 0; i < NUMPIXELS ; i++)
    {
      if ( i < nbr_of_led )
        led[i] = VAL_LED_STEP;
      else
        led[i] = 0;
    }
  }
  else {
    for (int i = 0; i < NUMPIXELS; i++)
    {
      led[i] = v / NUMPIXELS;
    }
  }
}


void print_values()
{
  Serial.print(" X: ");
  Serial.print(normAccel.XAxis);
  Serial.print(" Y: ");
  Serial.print(normAccel.YAxis);
  Serial.print(" Z: ");
  Serial.print(normAccel.YAxis);

  Serial.print("\tX: ");
  Serial.print(sum.XAxis);
  Serial.print(" Y: ");
  Serial.print(sum.YAxis);
  Serial.print(" Z: ");
  Serial.print(sum.ZAxis);


  Serial.print("\tled: ");
  Serial.println(LED_COLOR_R[16]);
}


void checkSettings()
{
  Serial.println();

  Serial.print(" * Sleep Mode:            ");
  Serial.println(mpu.getSleepEnabled() ? "Enabled" : "Disabled");

  Serial.print(" * Clock Source:          ");
  switch (mpu.getClockSource())
  {
    case MPU6050_CLOCK_KEEP_RESET:     Serial.println("Stops the clock and keeps the timing generator in reset"); break;
    case MPU6050_CLOCK_EXTERNAL_19MHZ: Serial.println("PLL with external 19.2MHz reference"); break;
    case MPU6050_CLOCK_EXTERNAL_32KHZ: Serial.println("PLL with external 32.768kHz reference"); break;
    case MPU6050_CLOCK_PLL_ZGYRO:      Serial.println("PLL with Z axis gyroscope reference"); break;
    case MPU6050_CLOCK_PLL_YGYRO:      Serial.println("PLL with Y axis gyroscope reference"); break;
    case MPU6050_CLOCK_PLL_XGYRO:      Serial.println("PLL with X axis gyroscope reference"); break;
    case MPU6050_CLOCK_INTERNAL_8MHZ:  Serial.println("Internal 8MHz oscillator"); break;
  }

  Serial.print(" * Accelerometer:         ");
  switch (mpu.getRange())
  {
    case MPU6050_RANGE_16G:            Serial.println("+/- 16 g"); break;
    case MPU6050_RANGE_8G:             Serial.println("+/- 8 g"); break;
    case MPU6050_RANGE_4G:             Serial.println("+/- 4 g"); break;
    case MPU6050_RANGE_2G:             Serial.println("+/- 2 g"); break;
  }

  Serial.print(" * Accelerometer offsets: ");
  Serial.print(mpu.getAccelOffsetX());
  Serial.print(" / ");
  Serial.print(mpu.getAccelOffsetY());
  Serial.print(" / ");
  Serial.println(mpu.getAccelOffsetZ());
  Serial.println();
}
