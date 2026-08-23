#include <Wire.h>
#include <Adafruit_ADS1X15.h>
#include <arduinoFFT.h>
#include <LiquidCrystal_I2C.h>
Adafruit_ADS1115 ads;
LiquidCrystal_I2C lcd(0x27, 16, 2); 
TwoWire eegWire(1);

const int SDA_PIN = 25;
const int SCL_PIN = 26;

const int sda = 21;
const int scl = 22;
const int canal_eeg = 0;

const uint16_t samples = 256;
const uint32_t timp = 4000;

float vReal[samples];
float vImag[samples];

uint16_t indexs = 0;
uint32_t samplen = 0;
uint32_t sampletime1 = 0;
uint32_t sapletimeu = 0;

int16_t blockminim = 32767;
int16_t blockmaxim = 0;
float samplerate = 250.0;

const float yes_hz = 10.0;
const float no_hz = 15.0;
const float banda_wi = 1.5;
const float banda_we = 1.5;
const float harmon_we = 0.0;

const float nivel_detectie = 1.35;
const float ratie_min = 1.40;
const float power_min = 0.035;

const int pasi_calibrare = 10;
float baseline10 = 0;
float baseline15 = 0;
int calibration_index = 0;
bool calibrated = false;

int last_c = 0;
int same_c_c = 0;
bool armed = true;


float bandPower(float freqmin, float freqmax) {
  float power = 0;

  for (uint16_t i = 1; i < samples / 2; i++) {
    float frecventa = i * samplerate / samples;

    if (frecventa >= freqmin && frecventa <= freqmax) {
      power += vReal[i] * vReal[i];
    }
  }

  return power;
}


void resetare() {
  baseline10 = 0;
  baseline15 = 0;
  calibration_index = 0;
  calibrated = false;
  last_c = 0;
  same_c_c = 0;
  armed = true;

  Serial.println("resetat");
}


void calibrare(float power10, float power15) {
  baseline10 += power10;
  baseline15 += power15;
  calibration_index++;

  Serial.print("calibrare ");
  Serial.print(calibration_index);
  Serial.print("/");
  Serial.println(pasi_calibrare);
  lcd.setCursor(0, 0);
lcd.print("                ");

lcd.setCursor(0, 1);
lcd.print("                ");
  lcd.setCursor(0,0);
  lcd.print("calibrat");

  if (calibration_index >= pasi_calibrare) {
    baseline10 /= pasi_calibrare;
    baseline15 /= pasi_calibrare;

    if (baseline10 < 0.000000001) {
      baseline10 = 0.000000001;
    }

    if (baseline15 < 0.000000001) {
      baseline15 = 0.000000001;
    }

    calibrated = true;
    Serial.println("ready");
  }
}


void sendDetection(int frequency, float confidence) {
  String raspuns = "";

  if (frequency == 10) {
    raspuns = "DA";
  } else if (frequency == 15) {
    raspuns = "NU";
  } else {
    return;
  }

  Serial.print("SSVEP:");
  Serial.print(frequency);
  Serial.print(",CONF:");
  Serial.println(confidence, 2);

  Serial.print("RASPUNS: ");
  Serial.println(raspuns);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("RASPUNS: ");
  lcd.print(raspuns);

  lcd.setCursor(0, 1);
  lcd.print("Siguranta:");
  lcd.print(confidence * 100, 0);
  lcd.print("%");
  delay(2000);
}


void detectssvep(float power10, float power15) {
  float score10 = power10 / baseline10;
  float score15 = power15 / baseline15;

  float winner = max(score10, score15);
  float loser = min(score10, score15);
  float ratie = winner / max(loser, 0.000000001f);
  float winnerPower = score10 > score15 ? power10 : power15;
  int candidate = score10 > score15 ? 10 : 15;
  float confidence =
      winner / max(score10 + score15, 0.000000001f);

  Serial.print("10:");
  
  Serial.print(score10, 2);
  Serial.print(" 15:");
  
 
  Serial.print(score15, 2);
  Serial.print(" r:");
  Serial.println(ratie, 2);
lcd.setCursor(0, 0);
lcd.print("                ");

lcd.setCursor(0, 1);
lcd.print("                ");
delay(5);
  lcd.setCursor(0, 0);
lcd.print("10:");

lcd.setCursor(3, 0);
lcd.print(score10, 2);

lcd.setCursor(8, 0);
lcd.print("15:");

lcd.setCursor(11, 0);
lcd.print(score15, 2);

lcd.setCursor(0, 1);
lcd.print("r:");

lcd.setCursor(2, 1);
lcd.print(ratie, 2);

  if (!armed) {
    if (winner < 1.30 || ratie < 1.12) {
      armed = true;
      last_c = 0;
      same_c_c = 0;
      Serial.println("ready");
    }

    return;
  }

  bool detected =
      winner >= nivel_detectie &&
      ratie >= ratie_min &&
      winnerPower >= power_min;

  if (!detected) {
    last_c = 0;
    same_c_c = 0;
    return;
  }

  if (candidate == last_c) {
    same_c_c++;
  } else {
    last_c = candidate;
    same_c_c = 1;
  }

  if (same_c_c >= 2) {
    sendDetection(candidate, confidence);
    armed = false;
    last_c = 0;
    same_c_c = 0;
  }
}


void procesarefft() {
  uint32_t durata =
      sapletimeu - sampletime1;

  if (durata == 0) {
    return;
  }

  samplerate =
      (samples - 1) * 1000000.0 / durata;

  ArduinoFFT<float> fft(
      vReal,
      vImag,
      samples,
      samplerate);

  fft.dcRemoval();
  fft.windowing(
      FFTWindow::Hamming,
      FFTDirection::Forward);
  fft.compute(FFTDirection::Forward);
  fft.complexToMagnitude();

  float totalPower =
      bandPower(6.0, 35.0);

  if (totalPower < 0.000000001) {
    totalPower = 0.000000001;
  }

  float power10 =
      bandPower(
          yes_hz - banda_wi,
          yes_hz + banda_we) +
      harmon_we *
      bandPower(
          2 * yes_hz - banda_wi,
          2 * yes_hz + banda_we);

  float power15 =
      bandPower(
          no_hz - banda_wi,
          no_hz + banda_we) +
      harmon_we *
      bandPower(
          2 * no_hz - banda_wi,
          2 * no_hz + banda_we);

  power10 /= totalPower;
  power15 /= totalPower;

  if (blockminim <= 5 || blockmaxim >= 32760) {
    Serial.println("semnal saturat");
  }

  if (!calibrated) {
    calibrare(power10, power15);
  } else {
    detectssvep(power10, power15);
  }
}


void citireeeg() {
  int16_t valoare =
      ads.readADC_SingleEnded(canal_eeg);

  uint32_t acum = micros();

  if (indexs == 0) {
    sampletime1 = acum;
    blockminim = 32767;
    blockmaxim = 0;
  }

  vReal[indexs] = valoare;
  vImag[indexs] = 0;

  if (valoare < blockminim) {
    blockminim = valoare;
  }

  if (valoare > blockmaxim) {
    blockmaxim = valoare;
  }

  sapletimeu = acum;
  indexs++;

  if (indexs >= samples) {
    procesarefft();
    indexs = 0;
  }
}


void setup() {
  Serial.begin(115200);
  delay(1000);

  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(100000);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("NeuroVoice");

  eegWire.begin(sda, scl, 400000);

  if (!ads.begin(0x48, &eegWire)) {
    Serial.println("ADS1115 nu raspunde");

    lcd.setCursor(0, 1);
    lcd.print("ADS lipsa");

    while (true) {
      delay(1000);
    }
  }

  ads.setGain(GAIN_ONE);
  ads.setDataRate(RATE_ADS1115_860SPS);

  lcd.setCursor(0, 1);
  lcd.print("Calibrare...");

  Serial.println("NeuroVoice");
  Serial.println("calibrare");

  samplen = micros();
}

void loop() {
  if (Serial.available()) {
    char comanda = Serial.read();

    if (comanda == 'c' || comanda == 'C') {
      resetare();
    }
  }

  uint32_t acum = micros();

  if ((int32_t)(acum - samplen) >= 0) {
    samplen += timp;

    if ((int32_t)(acum - samplen) > (int32_t)timp) {
      samplen = acum + timp;
    }

    citireeeg();
  }
}
