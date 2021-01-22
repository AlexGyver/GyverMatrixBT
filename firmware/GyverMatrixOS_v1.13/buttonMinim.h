#pragma pack(push,1)
typedef struct {
  bool holdedFlag: 1;
  bool btnFlag: 1;
  bool pressF: 1;
  bool clickF: 1;
  bool holdF: 1;
} buttonMinimFlags;
#pragma pack(pop)

class buttonMinim {
  public:
    buttonMinim(uint8_t pin, boolean type);   // type true - pullDOWN, false - input pullUP
    boolean pressed();
    boolean clicked();
    boolean holding();
    boolean holded();
  private:
    buttonMinimFlags flags;
    void tick();
    uint32_t _btnTimer;
    byte _pin;
    boolean _type;
};

buttonMinim::buttonMinim(uint8_t pin, boolean type) {  
  _pin = pin;
  _type = type;
  if (!_type) pinMode(_pin, INPUT_PULLUP);
  else pinMode(_pin, INPUT);
}

void buttonMinim::tick() {
  boolean btnState = digitalRead(_pin) ^ _type;
  
  if (!btnState && !flags.btnFlag && ((uint32_t)millis() - _btnTimer > 90)) {
    flags.btnFlag = true;
    _btnTimer = millis();
    flags.pressF = true;
    flags.holdedFlag = true;
  }
  if (btnState && flags.btnFlag && ((uint32_t)millis() - _btnTimer < 350)) {
    flags.btnFlag = false;
    _btnTimer = millis();
    flags.clickF = true;
    flags.holdF = false;
  }
  if (flags.btnFlag && ((uint32_t)millis() - _btnTimer > 600)) {
    if (!btnState) {
      flags.holdF = true;
    } else {
      flags.btnFlag = false;
      flags.holdF = false;
      _btnTimer = millis();
    }
  }
}

boolean buttonMinim::pressed() {
  buttonMinim::tick();
  if (flags.pressF) {
    flags.pressF = false;
    return true;
  }
  else return false;
}

boolean buttonMinim::clicked() {
  buttonMinim::tick();
  if (flags.clickF) {
    flags.clickF = false;
    return true;
  }
  else return false;
}

boolean buttonMinim::holding() {
  buttonMinim::tick();
  if (flags.holdF) {
    return true;
  }
  else return false;
}

boolean buttonMinim::holded() {
  buttonMinim::tick();
  if (flags.holdF && flags.holdedFlag) {
    flags.holdedFlag = false;
    return true;
  }
  else return false;
}
