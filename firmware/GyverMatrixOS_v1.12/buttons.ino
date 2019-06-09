
#if (USE_BUTTONS == 1)
#include "buttonMinim.h"

buttonMinim bt_left(BUTT_LEFT, false);
buttonMinim bt_right(BUTT_RIGHT, false);
buttonMinim bt_up(BUTT_UP, false);
buttonMinim bt_down(BUTT_DOWN, false);

#if (MCU_TYPE == 1)   // у esp8266 пин 8 подтянут к земле
buttonMinim bt_set(BUTT_SET, true);
#else
buttonMinim bt_set(BUTT_SET, false);
#endif


timerMinim stepTimer(100);
#endif

boolean checkButtons() {
#if (USE_BUTTONS == 1)
  if (!gameDemo) {
    if (bt_left.pressed()) {
      buttons = 3;
      controlFlag = true;
    }
    if (bt_right.pressed()) {
      buttons = 1;
      controlFlag = true;
    }
    if (bt_up.pressed()) {
      buttons = 0;
      controlFlag = true;
    }
    if (bt_down.pressed()) {
      if (stepTimer.isReady())
        buttons = 2;
    }

    if (bt_left.holding()) {
      if (stepTimer.isReady())
        buttons = 3;
    }
    if (bt_right.holding()) {
      if (stepTimer.isReady())
        buttons = 1;
    }
    if (bt_up.holding()) {
      if (stepTimer.isReady())
        buttons = 0;
    }
    if (bt_down.holding()) {
      if (stepTimer.isReady())
        buttons = 2;
    }
  }
#endif

  // если нажали кнопку, возврат к обычному режиму
  if (controlFlag) {
    idleTimer.reset();
    controlFlag = false;
    idleState = false;
    gameDemo = false;
    return true;
  }

  if (buttons != 4) return true;
  return false;
}
