### Send Message from Controller

| PIN | DEC/HEX | Action |
| --- | --------|------- |
| A1  | 31/0x1F | Fader1 |
| A0  | 41/0x29 | Fader2 |
|     | 32/0x20 | Select-Track |
|     | 42/0x2A | Select-Track |
|     | 33/0x21 | Speed1 |
|     | 43/0x2B | Speed2 |
|     | 34/0x22 | High1  |
|     | 44/0x2C | High2  |
|     | 35/0x23 | Mid1   |
|     | 45/0x2D | Mid2   |
|     | 36/0x24 | Low1   |
|     | 46/0x2E | Low2   |
|     | 37/0x25 | Vol1   |
|     | 47/0x2F | Vol2   |

| PIN | DEC/HEX | Action |
| --- | --------|------- |
| 8   | 50/32  | Play1 |
| 9   | 51/33  | Play2 |
| 20  | 55/37  | Cue1 |
| 16  | 56/38  | Cue2 |
| 10  | 58/3A  | Select1 |
| 6   | 59/3B  | Select2 |

### Receive Message from Application

| PIN | HEX | Action    |
| --- | ----|---------- |
| 4   | 01  | Play LED1 |
| 5   | 02  | Play LED2 |
|     | 14  | Minutes   |
|     | 15  | Seconds   |
|     | 16  | 1/10 Seconds |
|     | 3A  | EOM Deck 1   |
|     | 3B  | EOM Deck 2   |
|     | 3F  | Reset/Close  |

### Documentation ###

[Controls](https://manual.mixxx.org/2.3/en/chapters/appendix/mixxx_controls.html)
[Mapping](https://github.com/mixxxdj/mixxx/wiki/Midi-Controller-Mapping-File-Format)