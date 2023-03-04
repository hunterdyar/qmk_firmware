/* Copyright 2020 imchipwood
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "v1x_oled_calc.h"

#include <stdio.h>
// #include <stdio.h>
char wpm_str[10];

#ifdef ENCODER_ENABLE
bool encoder_update_user(uint8_t index, bool clockwise) {
    // Right encoder
    if (index == 0) {
        if (clockwise) {
            tap_code(KC_PGUP);
        } else {
            tap_code(KC_PGDN);
        }
    // Left encoder
    } else if (index == 1) {
        if (clockwise) {
            tap_code(KC_WH_U);
        } else {
            tap_code(KC_WH_D);
        }
    }
    return true;
}
#endif

#ifdef OLED_ENABLE

double buffA = 0;
char operator = ' ';
double buffB = 0;
double answer;
char aLine [16];
char bLine [16];
char oLine [16];
char ansLine [16];
bool normalKeypad = true;
// bool dividedByZero = false;
int placeEntry = 0;

static uint16_t zero_timer = 9999;//initialize higher than timer so frown doesn't appear on boot

uint16_t lastKeycode;
void enterNumber(int num)
{
    if(placeEntry == 0){
        buffB = ((buffB)*10) + num;
    }else{
        buffB = (buffB) + (num * (1/(pow(10,placeEntry))));
        placeEntry++;
    }
}

void clear(void)
{
    buffA = 0;
    buffB = 0;
    operator = ' ';
    answer = 0;
    placeEntry = 0;
}

void calculate(void)
{
    if(operator == '+')
    {
        answer = buffA + buffB;
    }else if(operator == '*')
    {
        answer = buffA * buffB;
    }else if(operator == '-')
    {
        answer = buffA - buffB;
    }else if(operator == '/')
    {
        if(buffB != 0){
            answer = buffA / buffB;
        }else{
            // dividedByZero = true;
            zero_timer = timer_read();
            clear();
            return;
        }
    } if(operator == ' ')
    {
        buffA = buffB;
        answer = buffB;
        buffB = 0;
        placeEntry = 0;
        return;
    }

    buffA = answer;
    buffB = 0;
    operator = ' ';
    placeEntry = 0;
}
void setOperator(char newOp)
{
    if(operator != ' '){
        calculate();
        buffA = answer;
        buffB = 0;
    }else if(newOp != ' '){
        //doing an operation, hitting enter (op becomes ' '), then hitting another op - should fill a with the answer value and prepare to type into B.
        //BUT typing a value and JUST hitting = with no operation should shift that value (b) into answer, and 0 into a. thats in Calculate().
        if(lastKeycode == KC_KP_ENTER){
            //found an answer last (because we just slapped enter). buffA probably already is answer, and we can basically do nothing.
            buffA = answer;
            buffB = 0;
        }else{
            //typing a value last (because op isn't set and we haven't hit enter last), so we want to operate on buffer b.
            buffA = buffB;
            buffB = 0;
            answer = buffA;
        }

    }

    placeEntry = 0;
    operator = newOp;
}



bool process_record_user(uint16_t keycode, keyrecord_t *record) {
  switch (keycode) {
    case KC_KP_1:
      if (record->event.pressed) {
        enterNumber(1);
      }
      break;
    case KC_KP_2:
      if (record->event.pressed) {
        enterNumber(2);
      }
      break;
    case KC_KP_3:
      if (record->event.pressed) {
        enterNumber(3);
      }
      break;
      case KC_KP_4:
      if (record->event.pressed) {
        enterNumber(4);
      }
      break;
    case KC_KP_5:
      if (record->event.pressed) {
        enterNumber(5);
      }
      break;
    case KC_KP_6:
      if (record->event.pressed) {
        enterNumber(6);
      }
      break;
    case KC_KP_7:
      if (record->event.pressed) {
        enterNumber(7);
      }
      break;
    case KC_KP_8:
      if (record->event.pressed) {
        enterNumber(8);
      }
      break;
    case KC_KP_9:
      if (record->event.pressed) {
        enterNumber(9);
      }
      break;
    case KC_KP_0:
      if (record->event.pressed) {
        enterNumber(0);
      }
      break;
     case KC_KP_DOT:
      if (record->event.pressed) {
        if(placeEntry == 0)
        {
            placeEntry++;
        }
        //no else. placeEntry is only non-zero when we are already entering decimals. so we just ignore extra dots. "1....3" -> "1.3".
      }
      break;
    case KC_KP_ENTER:
        // Do the calculation
        if (record->event.pressed) {
            if(lastKeycode == KC_KP_ENTER)
            {
                clear();
            }else{
                //switch for operator.
                calculate();
            }
        }
        break;
    case KC_KP_ASTERISK:
        // Do the calculation
        if (record->event.pressed) {
           setOperator('*');
        }
        break;
    case KC_KP_PLUS:
        // Do the calculation
        if (record->event.pressed) {
           setOperator('+');
        }
        break;
    case KC_KP_SLASH:
        // Do the calculation
        if (record->event.pressed) {
            setOperator('/');
        }
        break;
    case KC_KP_MINUS:
        // Do the calculation
        if (record->event.pressed) {
            setOperator('-');
        }
        break;
    case CALC_TOGGLE_PAD:
          if (record->event.pressed) {
                normalKeypad = !normalKeypad;
          }
        break;
    default:
      break; // Process all other keycodes normally
  }
  lastKeycode = keycode;
  return normalKeypad;
}

//rotate! ignore errors here it doesnt know about oled....h
oled_rotation_t oled_init_user(oled_rotation_t rotation){
    oled_clear();
    return OLED_ROTATION_270;
}

//return the number of chars printed.
//forceDot will draw the "." even if its not clear (like when we first tap "." and the buffer is still "0").
int oled_print_double(double val, bool forceDot)
{
    int chars = 0;
    int whole = (int)val;
    int places = (whole != 0) ? 1000 : 10000;//one extra place for showing ".4444" instead of "0.444"
    int fraction = (int)(val*places);
    fraction = fraction-(whole *places);

    //set fraction to abs(fraction). because -3 / 2 is not equal to "-1.-5".
    fraction = fraction < 0 ? -fraction : fraction;

    if(fraction == 0)
    {
        if(forceDot)
        {
            if(whole == 0){
                chars = snprintf(ansLine,sizeof(ansLine),".");
            }else{
                chars = snprintf(ansLine,sizeof(ansLine),"%d.",whole);
            }
        }else{
            chars = snprintf(ansLine,sizeof(ansLine),"%d",whole);//works when whole is 0 too.
        }
    }else if(whole == 0)
    {
        if(fraction % 10000 == 0)
        {
            chars = snprintf(ansLine,sizeof(ansLine),".%d",fraction/10000);
        }else if(fraction % 1000 == 0)
        {
            chars = snprintf(ansLine,sizeof(ansLine),".%d",fraction/1000);
        }else if(fraction % 100 == 0)
        {
            chars = snprintf(ansLine,sizeof(ansLine),".%d",fraction/100);
        }else if(fraction % 10 == 0)
        {
            chars = snprintf(ansLine,sizeof(ansLine),".%d",fraction/10);
        }
        else
        {
            chars = snprintf(ansLine,sizeof(ansLine),".%d",fraction);
        }
    }
    else{
        //I love copy and pasting code, it's my favorite hobby.
        if(fraction % 1000 == 0)
        {
            chars = snprintf(ansLine,sizeof(ansLine),"%d.%d",whole,fraction/1000);
        }else if(fraction % 100 == 0)
        {
            chars = snprintf(ansLine,sizeof(ansLine),"%d.%d",whole,fraction/100);
        }else if(fraction % 10 == 0)
        {
            chars = snprintf(ansLine,sizeof(ansLine),"%d.%d",whole,fraction/10);
        }
        else
        {
            chars = snprintf(ansLine,sizeof(ansLine),"%d.%d",whole,fraction);
        }
    }
    //write the number
    oled_write_ln(ansLine,false);

    //print the padding too.
    if(chars < 5)
    {
        // snprintf(oLine,sizeof(oLine),"--%d",chars);
        // oled_write_ln(oLine,false);
        oled_write_ln("    ",false);
    }

    return chars;
}



// Used to draw on to the oled screen
bool oled_task_user(void) {
    //how do we make this not happen on boot without a second timer?

    int totalChars = 0;
    int pushed = 0;
    totalChars += oled_print_double(buffA,false);
    pushed += totalChars >= 10 ? 1 : 0;
    //print operator
    snprintf(oLine,sizeof(oLine),"%c    ",operator);
    oled_write_ln(oLine,false);

    // oled_write_ln(bLine,false);
    totalChars =  oled_print_double(buffB,placeEntry>0);
    pushed += totalChars >= 10 ? 1 : 0;

    oled_write_ln("_____", false);

   if(timer_elapsed(zero_timer) < 750){
        //You tried to divide by zero. :(
        oled_write_ln(" :(  ", false);
   }else{
    totalChars = oled_print_double(answer,false);
    pushed += totalChars >= 10 ? 1 : 0;
   }
    //total lines
    pushed = 3-pushed;//padd 3, 2 or 1 lines depending on whats been done above.
    for(int i = 0;i<pushed;i++)
    {
        oled_write_ln("    ", false);
    }


    if(normalKeypad)
    {
      oled_write_ln("=   #", false);
    }else{
      oled_write_ln("=   .", false);
    }

    return false;
}

 #endif
