class buttonMinim {
  public:
    buttonMinim(uint8_t pin);
    boolean pressed(uint32_t* btnTimer);
    boolean clicked(uint32_t* btnTimer);
    boolean holded(uint32_t* btnTimer);
  private:
    boolean _btnFlag;
    byte _pin;
};

buttonMinim::buttonMinim(uint8_t pin) {
  pinMode(pin, INPUT_PULLUP);
  _pin = pin;
}

boolean buttonMinim::pressed(uint32_t* btnTimer) {
  if (!digitalRead(_pin) && !_btnFlag && ((uint32_t)millis() - *btnTimer > 80)) {
    _btnFlag = true;
    *btnTimer = millis();
    return true;
  }
  if (digitalRead(_pin) && _btnFlag && ((uint32_t)millis() - *btnTimer < 400)) {
    _btnFlag = false;
    *btnTimer = millis();
  }
  return false;
}

boolean buttonMinim::clicked(uint32_t* btnTimer) {
  if (!digitalRead(_pin) && !_btnFlag && ((uint32_t)millis() - *btnTimer > 80)) {
    _btnFlag = true;
    *btnTimer = millis();
  }
  if (digitalRead(_pin) && _btnFlag && ((uint32_t)millis() - *btnTimer < 400)) {
    _btnFlag = false;
    *btnTimer = millis();
    return true;
  }
  if (digitalRead(_pin) && _btnFlag && ((uint32_t)millis() - *btnTimer > 600)) {
    _btnFlag = false;
    *btnTimer = millis();
  }
  return false;
}

boolean buttonMinim::holded(uint32_t* btnTimer) {
  if (digitalRead(_pin) && _btnFlag && ((uint32_t)millis() - *btnTimer > 600)) {
    _btnFlag = false;
    *btnTimer = millis();
  }
  if (!digitalRead(_pin) && _btnFlag && ((uint32_t)millis() - *btnTimer > 600)) {
    return true;
  }
  return false;
}
