#include <FastLED.h>
#include <SoftwareSerial.h>

// ================= Config =================
#define LED_PIN     6
#define MIC_PIN     A0
#define NUM_LEDS    59
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB

CRGB leds[NUM_LEDS];

// Invertí la orientación física si tu tira está "al revés"
const bool REVERSE = false;

// Bluetooth
SoftwareSerial bluetooth(2, 3);  // RX=D2, TX=D3

int  brightness      = 180;
int  soundThreshold  = 100;

// Modos
enum { MODE_SOUND = 0, MODE_MANUAL = 1 };
int modeSel = MODE_SOUND;

// Direcciones 0=all, 1=left, 2=right, 3=center
int direction = 0;

// Arcoíris
bool rainbowMode = false;        // arcoíris encendido
bool rainbowRespectDir = false;  // arcoíris respeta dirección

// ======== Índices (59 LEDs impar) ========
inline int idx(int i){ return REVERSE ? (NUM_LEDS-1-i) : i; }
inline int midIndex(){ return NUM_LEDS/2; }               // 59 -> 29
inline int leftStart(){ return 0; }
inline int leftLen()  { return midIndex(); }              // 29 -> [0..28]
inline int rightStart(){ return midIndex()+1; }           // 30
inline int rightLen() { return NUM_LEDS - rightStart(); } // 29 -> [30..58]
inline int centerLen(){ return NUM_LEDS/2; }              // 29
inline int centerStart(){ return (NUM_LEDS-centerLen())/2; } // 15 -> [15..43]

// ======== Color base elegido por el usuario ========
CRGB    baseColor = CRGB::Blue;   // default azul para evitar “amarillo”
uint8_t baseHue   = 160;          // aprox. azul
bool    colorWasSetManually = false;

void setBaseColor(const CRGB& c){
  baseColor = c;
  CHSV hsv  = rgb2hsv_approximate(c);
  baseHue   = hsv.h;                 // guardamos hue del color elegido
  colorWasSetManually = true;
}

// ======== Estilo de sonido ========
// BRIGHT: mantiene color base, solo modula brillo
// HUE: oscila el tono alrededor del color base
enum { SOUND_STYLE_BRIGHT = 0, SOUND_STYLE_HUE = 1 };
int soundStyle = SOUND_STYLE_BRIGHT;

// ======== Utilitarios ========
void setSegment(int start,int len,const CRGB& c){
  if(start<0) start=0;
  if(start>=NUM_LEDS) return;
  if(start+len>NUM_LEDS) len = NUM_LEDS-start;
  for(int i=0;i<len;i++) leds[idx(start+i)] = c;
}
void clearAll(){ fill_solid(leds, NUM_LEDS, CRGB::Black); }

void applyDirection(const CRGB& color){
  clearAll();
  switch(direction){
    case 0: setSegment(0, NUM_LEDS, color);                              break; // ALL
    case 1: setSegment(leftStart(),  leftLen(),  color);                  break; // LEFT   [0..28]
    case 2: setSegment(rightStart(), rightLen(), color);                  break; // RIGHT  [30..58]
    case 3: setSegment(centerStart(), centerLen(), color);                break; // CENTER [15..43]
  }
  FastLED.show();
}

void printDirDebug(){
  Serial.println("---- DIRECCION / RANGOS ----");
  Serial.print("NUM_LEDS="); Serial.println(NUM_LEDS);
  Serial.print("REVERSE=");  Serial.println(REVERSE?"true":"false");
  Serial.print("mid=");      Serial.println(midIndex());
  Serial.print("LEFT:  [0.."); Serial.print(leftLen()-1); Serial.print("] len="); Serial.println(leftLen());
  Serial.print("CENTER:["); Serial.print(centerStart()); Serial.print(".."); Serial.print(centerStart()+centerLen()-1);
  Serial.print("] len="); Serial.println(centerLen());
  Serial.print("RIGHT: ["); Serial.print(rightStart()); Serial.print(".."); Serial.print(NUM_LEDS-1);
  Serial.print("] len="); Serial.println(rightLen());
  Serial.println("---------------------------");
}

void testBars(){
  clearAll();
  leds[idx(0)] = CRGB::Red;
  leds[idx(midIndex())] = CRGB::Green;
  leds[idx(NUM_LEDS-1)] = CRGB::Blue;
  FastLED.show();
}

// ======== Parser robusto ========
String normalizeCmd(String s){
  s.trim();
  if (s.startsWith("CMD:")) s.remove(0,4);
  s.replace('\r',' ');
  s.replace('\n',' ');
  s.replace(':',' ');
  s.replace('=',' ');
  s.replace('_',' ');
  while(s.indexOf("  ")!=-1) s.replace("  "," ");
  s.trim();
  s.toUpperCase();
  return s;
}
String readLineFrom(Stream &p){
  if(!p.available()) return "";
  String s = p.readStringUntil('\n');
  if(s.length()==0) s = p.readString();
  return normalizeCmd(s);
}

// ================= Procesador de comandos =================
void applyCmd(const String& cmdRaw){
  if(!cmdRaw.length()) return;
  String cmd = cmdRaw;
  Serial.print("CMD: "); Serial.println(cmd);

  // Atajos
  if(cmd=="SOUNDON")  cmd="SOUND ON";
  if(cmd=="SOUNDOFF") cmd="SOUND OFF";

  // Debug
  if(cmd=="TEST BARS"){ testBars(); return; }
  if(cmd=="DIR?"){ printDirDebug(); return; }
  if(cmd=="MODE?"){
    Serial.print("Mode: "); Serial.println(modeSel==MODE_SOUND?"SOUND":"MANUAL");
    Serial.print("SoundStyle: "); Serial.println(soundStyle==SOUND_STYLE_BRIGHT?"BRIGHT":"HUE");
    Serial.print("Direction: "); Serial.println(direction);
    Serial.print("Rainbow: "); Serial.println(rainbowMode?"ON":"OFF");
    return;
  }

  // ===== Colores básicos + NUEVOS (respetan dirección) =====
  // Inglés
  if(cmd.indexOf("COLOR RED")>=0     || cmd=="RED")     { rainbowMode=false; modeSel=MODE_MANUAL; setBaseColor(CRGB::Red);                 applyDirection(baseColor); return; }
  if(cmd.indexOf("COLOR GREEN")>=0   || cmd=="GREEN")   { rainbowMode=false; modeSel=MODE_MANUAL; setBaseColor(CRGB::Green);               applyDirection(baseColor); return; }
  if(cmd.indexOf("COLOR BLUE")>=0    || cmd=="BLUE")    { rainbowMode=false; modeSel=MODE_MANUAL; setBaseColor(CRGB::Blue);                applyDirection(baseColor); return; }
  if(cmd.indexOf("COLOR WHITE")>=0   || cmd=="WHITE")   { rainbowMode=false; modeSel=MODE_MANUAL; setBaseColor(CRGB::White);               applyDirection(baseColor); return; }
  if(cmd.indexOf("COLOR YELLOW")>=0  || cmd=="YELLOW")  { rainbowMode=false; modeSel=MODE_MANUAL; setBaseColor(CRGB(255,255,0));           applyDirection(baseColor); return; }
  if(cmd.indexOf("COLOR CYAN")>=0    || cmd=="CYAN")    { rainbowMode=false; modeSel=MODE_MANUAL; setBaseColor(CRGB(0,255,255));           applyDirection(baseColor); return; }
  if(cmd.indexOf("COLOR MAGENTA")>=0 || cmd=="MAGENTA") { rainbowMode=false; modeSel=MODE_MANUAL; setBaseColor(CRGB(255,0,255));           applyDirection(baseColor); return; }
  if(cmd.indexOf("COLOR PINK")>=0    || cmd=="PINK")    { rainbowMode=false; modeSel=MODE_MANUAL; setBaseColor(CRGB(255,105,180));         applyDirection(baseColor); return; }
  // Español (por si escribís a mano en Serial)
  if(cmd.indexOf("COLOR AMARILLO")>=0|| cmd=="AMARILLO"){ rainbowMode=false; modeSel=MODE_MANUAL; setBaseColor(CRGB(255,255,0));           applyDirection(baseColor); return; }
  if(cmd.indexOf("COLOR CIAN")>=0    || cmd=="CIAN")    { rainbowMode=false; modeSel=MODE_MANUAL; setBaseColor(CRGB(0,255,255));           applyDirection(baseColor); return; }
  if(cmd.indexOf("COLOR ROSA")>=0    || cmd=="ROSA")    { rainbowMode=false; modeSel=MODE_MANUAL; setBaseColor(CRGB(255,105,180));         applyDirection(baseColor); return; }

  // Arcoíris
  if(cmd=="RAINBOW DIR"){ rainbowRespectDir=true; rainbowMode=true; Serial.println("🌈 Rainbow con dirección"); return; }
  if(cmd=="RAINBOW"){ rainbowRespectDir=false; rainbowMode=true; Serial.println("🌈 Rainbow full strip"); return; }

  // Brillo
  if(cmd.startsWith("BRIGHT") || cmd.indexOf("BRIGHTNESS")>=0){
    int sep = cmd.lastIndexOf(' ');
    int val = (sep>0)? constrain(cmd.substring(sep+1).toInt(),0,255): brightness;
    brightness = val;
    FastLED.setBrightness(brightness);
    FastLED.show();
    Serial.print("Brillo = "); Serial.println(brightness);
    return;
  }

  // Sensibilidad
  if(cmd.startsWith("THRESH")){
    int sep = cmd.lastIndexOf(' ');
    int val = (sep>0)? constrain(cmd.substring(sep+1).toInt(),0,1023): soundThreshold;
    soundThreshold = val;
    Serial.print("Umbral sonido = "); Serial.println(soundThreshold);
    return;
  }

  // Dirección
  if(cmd.indexOf("ALL")>=0){ direction=0; Serial.println("Dirección: TODAS"); return; }
  if(cmd.indexOf("LEFT")>=0){ direction=1; Serial.println("Dirección: IZQUIERDA"); return; }
  if(cmd.indexOf("RIGHT")>=0){ direction=2; Serial.println("Dirección: DERECHA"); return; }
  if(cmd.indexOf("CENTER")>=0){ direction=3; Serial.println("Dirección: CENTRO"); return; }

  // Estilo de sonido
  if(cmd=="SOUND STYLE BRIGHT"){ soundStyle=SOUND_STYLE_BRIGHT; Serial.println("🔊 Sound style: BRIGHT (solo brillo)"); return; }
  if(cmd=="SOUND STYLE HUE"){ soundStyle=SOUND_STYLE_HUE; Serial.println("🔊 Sound style: HUE (oscila tono)"); return; }

  // Sound ON / OFF
  if(cmd.indexOf("SOUND ON")>=0 || cmd.indexOf("MIC ON")>=0){
    modeSel = MODE_SOUND; rainbowMode=false;
    Serial.println("🔊 Modo sonido ACTIVADO");
    return;
  }
  if(cmd.indexOf("SOUND OFF")>=0 || cmd.indexOf("MIC OFF")>=0){
    modeSel = MODE_MANUAL;
    clearAll(); FastLED.show();
    Serial.println("🔇 Modo manual");
    return;
  }

  // Help
  if(cmd=="HELP"){
    Serial.println("📘 COMANDOS:");
    Serial.println("COLOR:RED|GREEN|BLUE|WHITE|YELLOW|CYAN|MAGENTA|PINK");
    Serial.println("(ES) COLOR:AMARILLO|CIAN|MAGENTA|ROSA");
    Serial.println("BRIGHT:0..255");
    Serial.println("LEFT | RIGHT | CENTER | ALL | DIR?");
    Serial.println("SOUND ON | SOUND OFF | SOUND STYLE BRIGHT | SOUND STYLE HUE");
    Serial.println("RAINBOW | RAINBOW DIR");
    Serial.println("THRESH:0..1023");
    Serial.println("TEST BARS | MODE?");
    return;
  }

  Serial.println("❌ Comando no reconocido. Escribí HELP.");
}

// ================= Setup / Loop =================
void setup(){
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(brightness);

  Serial.begin(9600);
  bluetooth.begin(9600);

  Serial.println("✅ Listo. Escribí HELP.");
  printDirDebug();
  testBars();
}

void loop(){
  // Comandos
  String c = readLineFrom(Serial);     if(c.length()) applyCmd(c);
  String cb= readLineFrom(bluetooth);  if(cb.length()) applyCmd(cb);

  // Modo sonido
  if(modeSel==MODE_SOUND && !rainbowMode){
    int soundLevel = analogRead(MIC_PIN);
    int levelAbove = max(0, soundLevel - soundThreshold);

    // Brillo local según nivel de sonido (no pisa setBrightness)
    uint8_t val = (uint8_t)constrain(map(levelAbove, 0, 1023 - soundThreshold, 8, 255), 0, 255);

    CHSV hsv;
    if(soundStyle==SOUND_STYLE_BRIGHT){
      // Mantiene TU color: usa hue base y s base (si elegiste blanco, s≈0 -> blanco)
      CHSV base = rgb2hsv_approximate(baseColor);
      hsv = CHSV(base.h, base.s, val);
    }else{
      // Oscila el tono alrededor del tuyo
      uint8_t delta = (uint8_t)constrain(map(levelAbove, 0, 1023 - soundThreshold, 0, 96), 0, 96);
      hsv = CHSV(baseHue + delta, 255, val);
    }

    CRGB reactive; reactive.setHSV(hsv.h, hsv.s, hsv.v);
    applyDirection(reactive);
    delay(22);
  }

  // Arcoíris
  static uint8_t hue = 0;
  if(rainbowMode){
    if(!rainbowRespectDir){
      fill_rainbow(leds, NUM_LEDS, hue, 255/NUM_LEDS);
    }else{
      clearAll();
      int s=0, n=NUM_LEDS;
      if(direction==1){ s=leftStart();  n=leftLen();  }
      if(direction==2){ s=rightStart(); n=rightLen(); }
      if(direction==3){ s=centerStart();n=centerLen(); }
      for(int i=0;i<n;i++){
        leds[idx(s+i)] = CHSV(hue + (i * (255 / max(1,n))), 255, 255);
      }
    }
    FastLED.show();
    hue++;
    delay(18);
  }
}
