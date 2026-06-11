#include <TouchKey.h>
#include <string.h>

// ── CTF Flags ─────────────────────────────────────────────────────────────
#define FLAG_EASY   "FLAG-UART1"
#define FLAG_HARD   "FLAG-PENTA"
#define FLAG_MORSE  "FLAG-CW"
#define FLAG_SHELL  "FLAG-SH3LL"    // bonus: découverte du shell

// ── Hardware ──────────────────────────────────────────────────────────────
#define LED_PIN    11
#define CH1        (1<<2)   // TIN2 = TP1 = "bouton 1"
#define CH2        (1<<3)   // TIN3 = TP2 = "bouton 2"

// ── Morse timing (ms) ─────────────────────────────────────────────────────
#define DOT    150
#define DASH   450
#define IGAP   150
#define CGAP   300
#define WGAP   1050

// ── Temporisations ────────────────────────────────────────────────────────
#define TOUCH_TIMEOUT   3000UL
#define MORSE_PERIOD   30000UL

// ── État touch ────────────────────────────────────────────────────────────
static uint8_t  tp1_count    = 0;
static uint8_t  tp1_prev     = 0;
static uint8_t  tp2_prev     = 0;
static uint32_t last_tp1_ms  = 0;
static uint32_t last_morse_ms = 0;

// ── Shell ─────────────────────────────────────────────────────────────────
#define CMD_BUF 20
static char    cmd_buf[CMD_BUF];
static uint8_t cmd_len = 0;

// ── Morse ─────────────────────────────────────────────────────────────────
static void m_dot() {
  digitalWrite(LED_PIN, HIGH); delay(DOT);
  digitalWrite(LED_PIN, LOW);  delay(IGAP);
}
static void m_dash() {
  digitalWrite(LED_PIN, HIGH); delay(DASH);
  digitalWrite(LED_PIN, LOW);  delay(IGAP);
}

static void morse_char(char c) {
  if (c >= 'a' && c <= 'z') c -= 32;
  switch (c) {
    case 'A': m_dot(); m_dash(); break;
    case 'B': m_dash(); m_dot(); m_dot(); m_dot(); break;
    case 'C': m_dash(); m_dot(); m_dash(); m_dot(); break;
    case 'D': m_dash(); m_dot(); m_dot(); break;
    case 'E': m_dot(); break;
    case 'F': m_dot(); m_dot(); m_dash(); m_dot(); break;
    case 'G': m_dash(); m_dash(); m_dot(); break;
    case 'H': m_dot(); m_dot(); m_dot(); m_dot(); break;
    case 'I': m_dot(); m_dot(); break;
    case 'J': m_dot(); m_dash(); m_dash(); m_dash(); break;
    case 'K': m_dash(); m_dot(); m_dash(); break;
    case 'L': m_dot(); m_dash(); m_dot(); m_dot(); break;
    case 'M': m_dash(); m_dash(); break;
    case 'N': m_dash(); m_dot(); break;
    case 'O': m_dash(); m_dash(); m_dash(); break;
    case 'P': m_dot(); m_dash(); m_dash(); m_dot(); break;
    case 'Q': m_dash(); m_dash(); m_dot(); m_dash(); break;
    case 'R': m_dot(); m_dash(); m_dot(); break;
    case 'S': m_dot(); m_dot(); m_dot(); break;
    case 'T': m_dash(); break;
    case 'U': m_dot(); m_dot(); m_dash(); break;
    case 'V': m_dot(); m_dot(); m_dot(); m_dash(); break;
    case 'W': m_dot(); m_dash(); m_dash(); break;
    case 'X': m_dash(); m_dot(); m_dot(); m_dash(); break;
    case 'Y': m_dash(); m_dot(); m_dash(); m_dash(); break;
    case 'Z': m_dash(); m_dash(); m_dot(); m_dot(); break;
    case '0': m_dash(); m_dash(); m_dash(); m_dash(); m_dash(); break;
    case '1': m_dot(); m_dash(); m_dash(); m_dash(); m_dash(); break;
    case '2': m_dot(); m_dot(); m_dash(); m_dash(); m_dash(); break;
    case '3': m_dot(); m_dot(); m_dot(); m_dash(); m_dash(); break;
    case '4': m_dot(); m_dot(); m_dot(); m_dot(); m_dash(); break;
    case '5': m_dot(); m_dot(); m_dot(); m_dot(); m_dot(); break;
    case '6': m_dash(); m_dot(); m_dot(); m_dot(); m_dot(); break;
    case '7': m_dash(); m_dash(); m_dot(); m_dot(); m_dot(); break;
    case '8': m_dash(); m_dash(); m_dash(); m_dot(); m_dot(); break;
    case '9': m_dash(); m_dash(); m_dash(); m_dash(); m_dot(); break;
    default: delay(WGAP); return;
  }
  delay(CGAP);
}

static void play_morse(const char *s) {
  uint8_t i = 0;
  while (s[i]) { morse_char(s[i]); i++; }
  delay(WGAP * 2);
}

// ── Shell ─────────────────────────────────────────────────────────────────
static void shell_exec() {
  if (strcmp(cmd_buf, "help") == 0) {
    USBSerial_println("Available commands:");
    USBSerial_println("  help    - show this message");
    USBSerial_println("  info    - chip and firmware info");
    USBSerial_println("  ping    - connectivity check");
    USBSerial_println("  uptime  - milliseconds since boot");
    USBSerial_println("  touch   - current touch pad state");
    USBSerial_println("  morse   - trigger Morse sequence now");
    USBSerial_println("  reset   - reset touch counters");

  } else if (strcmp(cmd_buf, "info") == 0) {
    USBSerial_println("Board  : CH552G (8051, 24 MHz)");
    USBSerial_println("SDK    : ch55xduino 0.0.25");
    USBSerial_println("CTF    : v1.0  |  3 flags hidden");

  } else if (strcmp(cmd_buf, "ping") == 0) {
    USBSerial_println("pong");

  } else if (strcmp(cmd_buf, "uptime") == 0) {
    uint32_t ms = millis();
    uint32_t s  = ms / 1000;
    USBSerial_print("Uptime: ");
    USBSerial_print(s);
    USBSerial_println("s");

  } else if (strcmp(cmd_buf, "touch") == 0) {
    TouchKey_Process();
    uint8_t t = TouchKey_Get();
    USBSerial_print("TP1: ");
    USBSerial_println((t & CH1) ? "touched" : "open");
    USBSerial_print("TP2: ");
    USBSerial_println((t & CH2) ? "touched" : "open");

  } else if (strcmp(cmd_buf, "morse") == 0) {
    USBSerial_println("Triggering Morse on LED...");
    play_morse(FLAG_MORSE);
    last_morse_ms = millis();

  } else if (strcmp(cmd_buf, "reset") == 0) {
    tp1_count = 0;
    USBSerial_println("Touch counters reset.");

  } else if (strcmp(cmd_buf, "flag") == 0) {
    // Commande cachée — non listée dans help
    USBSerial_println(FLAG_SHELL);

  } else {
    USBSerial_print("Unknown command: ");
    USBSerial_println(cmd_buf);
    USBSerial_println("Type 'help' for available commands.");
  }
}

static void shell_tick() {
  while (USBSerial_available()) {
    char c = (char)USBSerial_read();

    if (c == '\r' || c == '\n') {
      USBSerial_println("");
      if (cmd_len > 0) {
        cmd_buf[cmd_len] = '\0';
        shell_exec();
        cmd_len = 0;
      }
      USBSerial_print("> ");

    } else if ((c == 8 || c == 127) && cmd_len > 0) {
      cmd_len--;
      USBSerial_print("\b \b");

    } else if (c >= 32 && cmd_len < CMD_BUF - 1) {
      cmd_buf[cmd_len++] = c;
      USBSerial_write(c);   // echo
    }
  }
}

// ── Setup ─────────────────────────────────────────────────────────────────
void setup() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  TouchKey_begin(CH1 | CH2);
  delay(500);
  USBSerial();

  USBSerial_println("================================");
  USBSerial_println("  CH552 Business Card CTF v1.0  ");
  USBSerial_println("================================");
  USBSerial_println("3 flags to find. Good luck!");
  USBSerial_println("Type 'help' for shell commands.");
  USBSerial_println("");
  USBSerial_print("> ");

  for (uint8_t i = 0; i < 3; i++) {
    digitalWrite(LED_PIN, HIGH); delay(80);
    digitalWrite(LED_PIN, LOW);  delay(80);
  }

  last_morse_ms = millis();
}

// ── Loop ──────────────────────────────────────────────────────────────────
void loop() {
  shell_tick();

  TouchKey_Process();
  uint8_t touched = TouchKey_Get();
  uint8_t t1      = (touched & CH1) ? 1 : 0;
  uint8_t t2      = (touched & CH2) ? 1 : 0;
  uint32_t now    = millis();

  // ── TP1 : compteur 5 touches ──────────────
  if (tp1_count > 0 && (now - last_tp1_ms) > TOUCH_TIMEOUT) {
    tp1_count = 0;
  }
  if (t1 && !tp1_prev) {
    last_tp1_ms = now;
    tp1_count++;
    for (uint8_t b = 0; b < tp1_count; b++) {
      digitalWrite(LED_PIN, HIGH); delay(30);
      digitalWrite(LED_PIN, LOW);  delay(60);
    }
    if (tp1_count >= 5) {
      USBSerial_println("");
      USBSerial_println(FLAG_HARD);
      USBSerial_print("> ");
      tp1_count = 0;
    }
  }
  tp1_prev = t1;

  // ── TP2 : flag immédiat ───────────────────
  if (t2 && !tp2_prev) {
    USBSerial_println("");
    USBSerial_println(FLAG_EASY);
    USBSerial_print("> ");
  }
  tp2_prev = t2;

  // ── Morse périodique ─────────────────────
  if ((now - last_morse_ms) >= MORSE_PERIOD) {
    last_morse_ms = now;
    play_morse(FLAG_MORSE);
  }
}
