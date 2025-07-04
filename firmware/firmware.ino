// CLACKADE: Compact Gamepad Efidget
// Board: XIAO RP2040 (did you switch to the board in board manager??? why are so forgetful,and waste so much complie time)

// ====== VARIABLES ======= //
const int buttonPins[4] = {7, 0, 2 ,1};
const int ledPins[4] = {26, 27, 28, 29};

const unsigned long longPressTime = 3000;

enum GameMode { SIMON, WHACK, CHASE, CODE, SELECT };
GameMode currentMode = SIMON;

bool buttonState[4] = {false, false, false, false};
bool lastButtonState[4] = {false, false, false, false};
unsigned long buttonPressTime[4] = {0, 0, 0, 0};


// ====== SETUP ======= //
void setup() {
  for (int i = 0; i < 4; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP);
    pinMode(ledPins[i], OUTPUT);
    digitalWrite(ledPins[i], LOW);
  }

  while (true) {
    for (int i = 0; i < 4; i++) {
      if (!digitalRead(buttonPins[i])) {
        delay(50); 
        randomSeed(millis());
        return;
      }
    }
  }
}


// ====== MAIN LOOP ======= //
void loop () {
    readButtons();

    if (currentMode == SELECT) {
        runGameSelector();
    } else {
        switch (currentMode) {
            case SIMON: runSimon(); break;
            case WHACK: runWhack(); break;
            case CHASE: runChase(); break;
            case CODE: runCodeCracker(); break;
            default: break;
        }
    }
}


// ====== BUTTON HANDLING ======= //
void readButtons () {
  for (int i = 0; i < 4; i++) {
    bool reading = !digitalRead(buttonPins[i]); 
    
    if (reading != lastButtonState[i]) {
      delay(5);

      if (reading == !digitalRead(buttonPins[i])) {
        if (reading) {
          buttonPressTime[i] = millis();
        } else {
          unsigned long duration = millis() - buttonPressTime[i];
          if (duration >= longPressTime) {
            onLongPress(i);
          } else {
            onShortPress(i);
          }
        }

        lastButtonState[i] = reading;
      }

    }

  }
}

// ====== BUTTON ACTIONS ======= //
void onShortPress (int i) {
    if (currentMode == SELECT) return;
    digitalWrite(ledPins[i], !digitalRead(ledPins[i]));
}

void onLongPress (int i) {
    if (i == 0) {
        currentMode = SELECT;
        clearLEDs();
    }
}

// ====== GAME SELECTOR ======= //
void runGameSelector () {
    static int selected = 0;
    static unsigned long lastChange = 0;

    if (millis() - lastChange > 1000) {
        clearLEDs();
        digitalWrite(ledPins[selected], HIGH);
        selected = (selected + 1) % 4;
        lastChange = millis();
    }

    for (int i = 0; i < 4; i++) {
        if (!digitalRead(buttonPins[i])) {
            currentMode = static_cast<GameMode>(selected);
            clearLEDs();
            delay(500);
        }
    }
}


// ====== HELPERS ======= //
void flashError() {
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 4; j++) digitalWrite(ledPins[j], HIGH);
    delay(150);
    for (int j = 0; j < 4; j++) digitalWrite(ledPins[j], LOW);
    delay(150);
  }
}

void clearLEDs() {
  for (int i = 0; i < 4; i++) {
    digitalWrite(ledPins[i], LOW);
  }
}

void flashQuick(int index) {
  digitalWrite(ledPins[index], HIGH);
  delay(100);
  digitalWrite(ledPins[index], LOW);
}

void winAnimation() {
  int clockwiseOrder[4] = {0, 1, 3, 2};
  const int rounds = 3;

  for (int r = 0; r < rounds; r++) {
    for (int i = 0; i < 4; i++) {
      digitalWrite(ledPins[clockwiseOrder[i]], HIGH);
      delay(100);
      digitalWrite(ledPins[clockwiseOrder[i]], LOW);
    }
  }
}



// ====== GAMES ======= //

// GAME: Simon Says
// HOW TO PLAY: leds sequence memory challenge

// === Simon Says Globals ===
const int maxSequenceLength = 100;
int simonSequence[maxSequenceLength];
int simonLength = 0;
int simonInputIndex = 0;
bool simonShowing = true;
unsigned long simonLastTime = 0;
int simonFlashIndex = 0;
bool simonLedOn = false;
unsigned long simonFlashDelay = 500;
bool simonInitialized = false;

void runSimon() {
  // first time only setup
  if (!simonInitialized) {
    simonSequence[0] = random(0, 4);
    simonLength = 1;
    simonInputIndex = 0;
    simonFlashIndex = 0;
    simonShowing = true;
    simonLedOn = false;
    simonLastTime = millis();
    simonInitialized = true;
    clearLEDs();
    return;
  }

  // patterns
  if (simonShowing) {
    if (millis() - simonLastTime >= simonFlashDelay) {
      if (simonLedOn) {
        digitalWrite(ledPins[simonSequence[simonFlashIndex]], LOW);
        simonLedOn = false;
        simonFlashIndex++;
        simonLastTime = millis();

        if (simonFlashIndex >= simonLength) {
          simonShowing = false;
          simonInputIndex = 0;
        }
      } else {
        digitalWrite(ledPins[simonSequence[simonFlashIndex]], HIGH);
        simonLedOn = true;
        simonLastTime = millis();
      }
    }
    return;
  }

  // player input match
  for (int i = 0; i < 4; i++) {
    if (!digitalRead(buttonPins[i])) {
      delay(20); 
      while (!digitalRead(buttonPins[i])); 

      
      digitalWrite(ledPins[i], HIGH);
      delay(200);
      digitalWrite(ledPins[i], LOW);

      if (i == simonSequence[simonInputIndex]) {
        simonInputIndex++;

        // after current completion
        if (simonInputIndex >= simonLength) {
          if (simonLength < maxSequenceLength) {
            simonSequence[simonLength++] = random(0, 4);
          }
          winAnimation();
          simonFlashIndex = 0;
          simonShowing = true;
          simonLedOn = false;
          simonLastTime = millis();
        }

      } else {
        //reset at wrong input
        flashError();
        simonSequence[0] = random(0, 4);
        simonLength = 1;
        simonInputIndex = 0;
        simonFlashIndex = 0;
        simonShowing = true;
        simonLedOn = false;
        simonLastTime = millis();
      }

      break;

    }
  }
}


// GAME: Whack a LED
// HOW TO PLAY: press the button corresponding to the led before it stops glowing

// === Whack-a-LED Globals ===
int whackTarget = -1;
unsigned long whackLastTime = 0;
unsigned long whackInterval = 1000; // time to respond
bool whackActive = false;
int whackScore = 0;
unsigned long whackStartDelay = 500;

void runWhack() {
  // create new tagret
  if (!whackActive && millis() - whackLastTime > whackStartDelay) {
    whackTarget = random(0, 4);
    digitalWrite(ledPins[whackTarget], HIGH);
    whackActive = true;
    whackLastTime = millis();
  }

  // check timeout
  if (whackActive && millis() - whackLastTime > whackInterval) {
    flashError();
    resetWhack();
    return;
  }

  // input match
  for (int i = 0; i < 4; i++) {
    if (!digitalRead(buttonPins[i])) {
      delay(20); 
      while (!digitalRead(buttonPins[i]));

      if (whackActive) {
        if (i == whackTarget) {
            // if hit correctly
            digitalWrite(ledPins[i], LOW);
            whackActive = false;
            whackScore++;
            whackInterval = max(300, whackInterval - 50);
            whackStartDelay = 300;

            if (whackScore % 5 == 0) {   // winAnimation for every 5 hits
                winAnimation();              
            }
        } else {
          // if wrong btn
          flashError();
          resetWhack();
        }
        return;
      }
    }
  }
}

void resetWhack() {
  clearLEDs();
  whackScore = 0;
  whackInterval = 1000;
  whackStartDelay = 500;
  whackTarget = -1;
  whackActive = false;
  whackLastTime = millis();
}


// GAME: LED Chase
// HOW TO PLAY: recreate a briefly showed led pattern

// === LED Chase Globals ===
int chaseIndex = 0;
unsigned long chaseLastStepTime = 0;
unsigned long chaseStepInterval = 300;
bool chaseDirection = true; // true = forward; false = backward
bool chaseRunning = true;
int chaseScore = 0;
int chaseHits = 0;

void runChase() {
  // moving LED
  if (millis() - chaseLastStepTime >= chaseStepInterval) {
    clearLEDs();

    // chase index
    if (chaseDirection) {
      chaseIndex = (chaseIndex + 1) % 4;
    } else {
      chaseIndex = (chaseIndex + 3) % 4;
    }

    digitalWrite(ledPins[chaseIndex], HIGH);
    chaseLastStepTime = millis();
  }

  // input check
  for (int i = 0; i < 4; i++) {
    if (!digitalRead(buttonPins[i])) {
      delay(20);
      while (!digitalRead(buttonPins[i]));

      if (i == chaseIndex) {
        // chase in time
        digitalWrite(ledPins[i], LOW);
        flashQuick(i);
        chaseHits++;
        chaseScore += 10;
        if (chaseHits % 5 == 0) { 
          chaseStepInterval = max(150, chaseStepInterval - 20);
          winAnimation();  // winAnimation for every 5 correct chases
        }
      } else {
        // timeout => "You LOSE"
        flashError();
        resetChase();
      }
      return;
    }
  }
}

void resetChase() {
  clearLEDs();
  chaseIndex = 0;
  chaseStepInterval = 300;
  chaseDirection = random(0, 2); 
  chaseRunning = true;
  chaseHits = 0;
  chaseScore = 0;
  chaseLastStepTime = millis();
}

// GAME: Code Cracker
// HOW TO PLAY: try to guess a randomly generated code (wordle with leds)

// === Code Cracker Globals ===
const int codeLength = 3;
int secretCode[codeLength];
int playerGuess[codeLength];
int guessIndex = 0;
bool codeCrackerInitialized = false;
unsigned long feedbackTime = 0;
bool feedbackActive = false;


void runCodeCracker() {
  // setup for first time only
  if (!codeCrackerInitialized) {
    generateSecretCode();
    guessIndex = 0;
    codeCrackerInitialized = true;
    clearLEDs();
    return;
  }

  // led on if active
  if (feedbackActive && millis() - feedbackTime >= 1000) {
    clearLEDs();
    feedbackActive = false;
    guessIndex = 0;
    return;
  }

  // storing inputs
  if (!feedbackActive) {
    for (int i = 0; i < 4; i++) {
      if (!digitalRead(buttonPins[i])) {
        delay(20);
        while (!digitalRead(buttonPins[i]));

        playerGuess[guessIndex++] = i;

        // feedback with corresp led on
        digitalWrite(ledPins[i], HIGH);
        delay(150);
        digitalWrite(ledPins[i], LOW);

        if (guessIndex >= codeLength) {
          checkCodeGuess();
          feedbackTime = millis();
          feedbackActive = true;
        }
        break;
      }
    }
  }
}

void generateSecretCode() {
  for (int i = 0; i < codeLength; i++) {
    secretCode[i] = random(0, 4);
  }
}

void checkCodeGuess() {
  bool matched[codeLength] = {false};
  bool guessed[codeLength] = {false};

  int correctPosition = 0;
  int correctButton = 0;

  // correct position
  for (int i = 0; i < codeLength; i++) {
    if (playerGuess[i] == secretCode[i]) {
      correctPosition++;
      matched[i] = true;
      guessed[i] = true;
    }
  }

  // correct button, wrong position
  for (int i = 0; i < codeLength; i++) {
    if (guessed[i]) continue;

    for (int j = 0; j < codeLength; j++) {
      if (!matched[j] && playerGuess[i] == secretCode[j]) {
        correctButton++;
        matched[j] = true;
        break;
      }
    }
  }

  // led output [Correct position: solid LED; Correct button wrong pos: blink LED]
  for (int i = 0; i < correctPosition; i++) {
    digitalWrite(ledPins[i], HIGH);
  }
  for (int i = correctPosition; i < correctPosition + correctButton; i++) {
    digitalWrite(ledPins[i], HIGH);
    delay(150);
    digitalWrite(ledPins[i], LOW);
    delay(150);
    digitalWrite(ledPins[i], HIGH);
  }

  // guessed code correctly
  if (correctPosition == codeLength) {
    delay(200);
    // for (int i = 0; i < 3; i++) {
    //   for (int j = 0; j < 4; j++) digitalWrite(ledPins[j], HIGH);
    //   delay(200);
    //   for (int j = 0; j < 4; j++) digitalWrite(ledPins[j], LOW);
    //   delay(200);
    // }
    winAnimation();
    generateSecretCode();
  }
}
