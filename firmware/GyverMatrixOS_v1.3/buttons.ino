
#if (USE_BUTTONS == 1)
#include "GyverButton.h"
GButton bt_left(BUTT_RIGHT);
GButton bt_right(BUTT_LEFT);
GButton bt_up(BUTT_UP);
GButton bt_down(BUTT_DOWN);
#endif

boolean checkButtons() {
  
#if (USE_BUTTONS == 1)
  bt_left.tick();
  bt_right.tick();
  bt_up.tick();
  bt_down.tick();

  if (bt_left.isPress()) {
    buttons = 3;
    controlFlag = true;
  }
  if (bt_right.isPress()) {
    buttons = 1;
    controlFlag = true;
  }
  if (bt_up.isPress()) {
    buttons = 0;
    controlFlag = true;
  }
  if (bt_down.isPress()) {
    buttons = 2;
    controlFlag = true;
  }
#endif

  // если нажали кнопку, возврат к обычному режиму
  if (controlFlag) {
    controlFlag = false;    
    
    idleTimer.reset();
    idleState = false;
    
    gameDemo = false;
    return true;
  }
  
  if (buttons != 4) return true;
  
  return false;
}
