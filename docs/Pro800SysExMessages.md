
# Pro-800 SysEx Messages

Everything below was verified on a real Pro-800 with **firmware 1.4.6** (one unit). The bench work behind
it — captures, probe scripts and a session-by-session log — lives in the reverse-engineering repository
([`pro800/docs/PROTOCOL_BehringerPro800.md`](https://github.com/steinachim/reverse_engineering), cited as
*RE §n* below); this document is the plugin-side summary and the byte tables `source/tailoring/` must match.

## Overall message structure
    0xF0                   // MIDI standard, start of SysEx
   
    0x00, 0x20, 0x32,      // Brand ID (Behringer)
    0x00, 0x01, 0x24,      // Product ID (Pro-800)
    0x00                   // CPU ID
		 
    0xZZ                   // message type
                              -> defines type of following message data (see below)
   
    (optional) command parameters (normally up to 2 params)

    0xF7                   // MIDI standard, end of SysEx

Often, if a request requires dedicated response, the command bytes will be sequential (e.g. `0x77` is answered by a `0x78` message) or a generic status message (type `0x01`, see below).

There is **no checksum, no length field and no request id**: a reply is recognised by its type and, where one exists, the echoed address or index. Only one request can usefully be in flight at a time (see "Transport behaviour" below).

## Known message types
|message type|parameters | description |
|--   |--|--|
|0x01 |00, status | Status Message. The first parameter byte is a constant `00` (not an echo), so the status is at message offset 0x0A: `0x00` on success, `0x01` on failure (as implemented in `StatusMessage`). The synth also answers every `0x78` write with it.
|0x02 |-  | Identity query, answers `03 00`. The `00` is fixed (all 128 parameter values answer the same); its meaning is unknown. `0x02`-`0x09` are a block of four identity queries (RE §2)
|0x03 |[00-7F]  | **HAZARD with parameter `0x30`**: reboots the synth into its bootloader (display `boot`, USB re-enumerates, only a power cycle brings it back — presets and firmware intact). Every other parameter, and the parameter-less form, answers Status OK and does nothing (all 128 values measured, RE §10)
|0x04 |-  | Product code, answers `05 50 30 45 39 49 00` — ASCII `"P0E9I"`, Behringer's internal product number for the Pro-800
|0x06 |-  | Request device name (responds with `07 50 52 4f 2d 38 30 30 00` - ASCII: "PRO-800"). Read-only and idempotent: the right probe for identifying the instrument
|0x08 | 0x00 | Request firmware version. The request carries a single `0x00` parameter; the reply is a `0x09` message (see below).
|0x09 |echo, version1, version2, version3 | Firmware version reply. The first parameter byte echoes the request's, so the three version bytes are at message offsets 0x0A, 0x0B, 0x0C - complete version = "version1.version2.version3"
|0x0E |[00-7F]  | **HAZARD — never send.** Writes `MIDI RX Channel` and `MIDI TX Channel` (settings offsets 10 and 12) together; the parameter is a flag, not a value: `00` sets both to `1` (DIP switches), anything else sets RX `249` / TX `2`. `249` is outside the RX field's range and makes the synth **deaf to all channel-voice MIDI** (notes, program changes, CC). The commit lands +1.57 s later and a second `0x0E` inside that window replaces the first. Recovery is a settings-block write, which needs no channel (RE §6)
|0x0F | [00-18] | Sets Transpose (settings offset 43) to `param − 12`: `00` → −12, `0C` → 0, `18` → +12. Status OK in that range, Status Error otherwise
|0x11 | [00-1F] | Sets the current preset's **Pitch Bend Range** in semitones (0–31), in the **edit buffer**: stored as `param × 2048` in program field 196 (format 111) once the preset is saved, invisible to a `0x77` read before that. Status OK in that range, Status Error otherwise
|0x12 | [00-02] | Sets Voice Priority (settings offset 30). Status OK in that range, Status Error otherwise
|0x13 | [00-07] | Sets Sync Clock Subdivision (settings offset 29). Status OK in that range, Status Error otherwise
|0x17 | [00-01] | Sets Sync In Forward Enable (settings offset 25). Status OK in that range, Status Error otherwise
|0x19 | [00-02] | Sets Sync In Polarity (settings offset 33). Status OK in that range, Status Error otherwise
|0x1A | [00-03] | Sets settings offset 18 (still unknown). Status OK in that range, Status Error otherwise — the range is `00-03`, an earlier `00-04` here was wrong
|0x1B | [00-03] | Sets Sync Source (settings offset 13). Status OK in that range, Status Error otherwise
|0x1C | [00-05] | Sets Sync In PPQN (settings offset 37). Status OK in that range, Status Error otherwise
|0x1D | - or [00-7F] | Unknown: responds with Status OK. Swept exhaustively (all 128 parameters and the parameter-less form) against the settings block, the panel state, the live parameters and the saved preset record: no effect anywhere (RE §2)
|0x32 | [00-7F] | - parameter `00`: **reload** — the synth recalls the preset its settings block points at (`Current Preset Number` / `Current Bank`, see below) into the voice engine and **discards unsaved front-panel edits** (the PRESET light stops blinking). It does not change the panel's mode. This is what makes a selection through the settings block audible, and what the plugin's *Revert* does (`ReloadMessage`)<br> - **any other parameter is a hazard**: display 8888, dots kept on, the synth responds strangely to its own controls (MIDI still seems to work). `Pro800Hazards` refuses it<br> - always responds with Status OK
|0x35 | - or [00-7F] | Unknown: responds with Status OK, no effect on any readable surface (swept twice; it served as the null control for the `0x32 00` measurements)
|0x36 |[00-7F] | - parameter 00: locks the front panel<br> - any other parameter: unlocks it again<br> - always responds with Status OK
|0x37 |[00-7F] | - parameter 00: Turn off all button lights<br> - any other parameter: Lights up all buttons<br> - always responds with Status OK
|0x50 |addr LSB, addr MSB, name bytes... | **Avoid.** Writes a preset's name directly (plain ASCII after the program address, no 7-bit packing). It leaves the record inconsistent: the name reads back one character short through `0x77`, and above 14 characters the display and every dump disagree permanently. With **only the address** it blanks the name — a blind sweep of this type once blanked the name of every preset in the library (recovered only by a factory reset). Rename by rewriting the whole record with `0x77`/`0x78` instead, which is what SynthTribe does (RE §8)
|0x70 | [00-2B] | **Reads the physical control surface**: `0x70 <index>` answers `0x71 <index> <value>` (`PanelMessage`, indices in `Pro800PanelConstants.h`). Indices `0x2C` and above, and a request without an index, answer a failure status. Read-only: the two-byte form of `0x71` does *not* write this space.<br>`00-14` = the 21 buttons, in `0x71`'s press-code numbering, **momentary** (1 only while physically held)<br>`15-17` = press codes without a button (`16`/`17` are the value wheel), always 0<br>`18-27` = the 14 panel switches, **latched**: 18 Osc A Rect, 19 Osc B Rect, 1A Filter Keyboard Full, 1B Filter Keyboard Half, 1C LFO Shape (1 = Tri/Sine/Saw, 0 = Pulse/Random/Noise), 1D LFO Dest Freq A-B, 1E LFO Dest PW A-B, 1F LFO Dest Filter, 20 Osc A Saw, 21 Osc A Tri, 22 Osc A Sync, 23 Osc B Saw, 24 Osc B Tri, 25 Poly-Mod Dest Freq A, 26 Poly-Mod Dest Filter, 27 Poly-Mod Unison Track<br>`28-2B` = the 4 rear DIP switches, each answering its weight (1/2/4/8) when on; **the sum is the 0-based MIDI channel** they select<br>Three things to know. The 3-position Filter Keyboard switch is two bits here — Off = (1A 0, 1B 0), Half = (0, 1), Full = (1, 0) — while the preset stores it as one enum. **This is the panel, not the sound**: loading a preset moves none of these, so a switch position equals the sound only in manual mode (Perf pressed twice, display `P800`), for controls touched since the last load, or after a host has sent the positions back as CC (what the plugin's "Align with panel positions" does). And the **LFO shape switch (1C) carries one bit of a six-way choice**, which is still enough to resolve it given the shape selected before: the switch has two sides (Tri/Sine/Saw = `1`, Pulse/Random/Noise = `0`) and the Performance menu decides which *pair* it toggles between — Tri/Pulse, Sine/Random or Saw/Noise. The previous shape therefore names the pair and the bit picks within it. `Pro800ProgramLfoShape` is ordered in exactly those pairs (Pulse 0 / Triangle 1, Random 2 / Sine 3, Noise 4 / Saw 5), so a shape's partner is `shape ^ 1` and its side is `shape & 1`. That pairing is inferred from the panel's layout, not measured on the wire
|0x71 | [00-17] | "push" buttons (Status OK in that range, Status Error otherwise — all 104 higher codes measured):<br>  - 00-09 = numpad<br> - 0A = Arp Up-Dn<br> - 0B = Arp Assign<br> - 0C = preset — **also recalls** the preset the settings block points at, like `0x32 00`, but as a mode button it may change the panel's mode<br> - 0D = rec<br> - 0E = perf<br> - 0F = settings<br> - 10 = seq 1<br>- 11 = seq 2<br>- 12 = tune<br>- 13 = sync clock<br> - 14 = sync source<br> - 15 = accepted, does nothing (no button has this code)<br>- 16 / 17 = the value wheel: steps the **preset selection** in preset mode, steps a setting's value while that setting is in its change state, does nothing otherwise<br>Note `0x71` is also the *reply* type of `0x70`.<br>**The synth's own save is a button sequence, so it can be driven from here**: `0D` (Rec), the slot's tens digit, its ones digit, `0C` (Preset) stores the edit buffer into that slot **of the current bank**. Three caveats measured on hardware: the sequence carries no bank (set `Current Bank` first and verify afterwards that the same slot in the other banks is unchanged), it **blanks the preset name**, and it stores the current knob positions along with everything else. The plugin does not use it
|0x72 | [00-19] | Reads one of the 26 **live parameters** — the physical **knob positions**, 7-bit (`73 <index> <value>`, value `00-7F`; the `00-FF` once claimed here cannot be carried in a SysEx data byte). Indices `1A-7F` answer a failure status.<br>00 Amp Release, 01 Amp Sustain, 02 Amp Decay, 03 Amp Attack, 04 Filter Release, 05 Filter Sustain, 06 Filter Decay, 07 Filter Attack, 08 Filter Resonance, 09 Filter Env Amount, 0A Osc A Frequency, 0B Osc B Frequency, 0C Osc B Fine, 0D Osc A Pulse Width, 0E Osc B Pulse Width, 0F Osc A Level, 10 Glide Time, 11 LFO Mod Freq, 12 LFO Mod Initial Amount, 13 Master Tune, 14 Master Volume, 15 Osc B Level, 16 Poly-Mod Source Osc B, 17 Poly-Mod Source Filter Env, 18 Noise Level, 19 Filter Cutoff<br>The numbering is unrelated to the CC numbers of the same knobs (Filter Cutoff is CC 15 but index `0x19`). Like `0x70` this is **global control-surface state, not the preset**: the values do not change when a preset is loaded, so they are the sound only in manual mode or for knobs touched since. The panel switches are not on this interface (use `0x70`)
|0x73 | [00-19], [00-7F] | Sets a live parameter (`param1` = index, `param2` = value); Status OK if in range. The write reaches the **edit buffer** (audible at once, like turning the knob) but not the stored record until the preset is saved. Index `0x12` (LFO Mod Initial Amount) is the one that does not round-trip: a value set with `0x73` reads back lower through `0x72` along a fixed lossy curve (0–12 read 0, only 0 and 127 are identity), so a read-back is not a check on that write
|0x77| [00-7F], [00-03] | Request program dump<br> param1 = program lsb,<br> param2 = program msb<br> -> response with 78 (see below)<br> -> note: 7e 03 = settings message (address 510 in the same address space; nothing else lives above the 400 presets)<br> -> an **empty** program slot answers with a bare `F0 F7`: an empty SysEx with no header, no message type and no echoed address. That, rather than a short or absent dump, is how "nothing is stored here" reads. An erased slot is indistinguishable from a never-written one<br> -> an **out-of-range** address (400–511 except 510) answers a `0x01` status with the failure code — distinguishable from an empty slot, and a round trip rather than a timeout<br> -> so a read has three answer classes; `SysExMatchers::isDumpReplyFor()` accepts all three
|0x78 |[00-7F], [00-03], [...] | Set program to value<br> param1 = program lsb,<br>param2 = program msb,<br>following bytes = program data (may be empty -> set to "uninitialized")<br> -> the synth answers with a `0x01` status (OK for a store and for an erase); reading the address back is the stronger check, and the only one that catches a write that was accepted but not stored<br> -> the synth stores exactly what it is sent and does not re-truncate, so rewriting a 190-byte record at full length leaves it 202 bytes
|0x7D | - | Factory Reset (no confirmation!)

### Never send

Named here on purpose: an unnamed hazard gets rediscovered by sending it. The Advanced tab refuses them unless "Allow hazardous messages" is ticked (`Pro800Hazards`).

| Type | Effect |
|---|---|
| `0x7D` | Factory reset. No confirmation, no undo. |
| `0x03` with parameter exactly `0x30` | Reboots into the bootloader; power cycle needed. |
| `0x32` with a non-zero parameter | The broken "8888" state. `0x32 00` is the ordinary reload and is safe. |
| `0x0E` | Writes an invalid RX channel for every non-zero parameter: the synth ignores all channel-voice MIDI until a settings write fixes it. |
| `0x50` | Writes name bytes without the record's own bookkeeping; with only an address it blanks the name. |

### Transport behaviour

Measured on macOS/CoreMIDI unless noted (RE §10, §11). `MidiHandler`/`SysExExchange` implement the consequences.

* **The synth never sends SysEx on its own.** Everything unsolicited is channel-voice CC: the knob-movement echo on its TX channel (if MIDI CC Mode is TX or TX&RX), and knobs drift across a threshold by themselves, so isolated CCs with nobody touching the synth are normal.
* **A burst of buffered CC arrives right after the port is opened** — the movements made while nothing held the port, delivered within ~340 ms, and it can contain **CC 0** (bank select). The plugin drops channel-voice input for the first 500 ms after opening.
* **MIDI Thru is on**: every channel-voice message the host sends comes back verbatim, on every channel. SysEx is not echoed. The plugin drops an inbound channel-voice message identical to one it sent within the last second.
* **The first request after opening the port is lost about one time in five**, and a port that sat idle for seconds loses two in a row; a settle delay does not help, a resend does. The exchange retries once by default and three times for the first request after connecting.
* **Any channel-voice message costs the next SysEx request one attempt** (~2.2 s until the retry answers). The session does not poll within a second of sending one.
* **A write commits on the synth's own cycle**, not with its status reply: about one settings write in fourteen is only readable ~1.6 s later. Read back by polling for ≥2.5 s, not once. The same holds for program writes.
* **The front panel is a second writer to the settings block**: closing a settings menu on the synth writes that menu's displayed value back over whatever the host wrote.
* **Only one request in flight at a time.** Requests are serialized; a reply is matched by type plus echoed address/index, and a bare `F0 F7` is accepted only as the answer to the read in flight.
* **A shared bus corrupts dump reads.** If another application reads dumps on the same port at the same time, replies get spliced: a record with our address and a foreign tail. A record longer than its preset version allows is rejected in the matcher (`SysExMatchers::outrunsDeclaredVersion()`, the "long" shape); the "short" shape (a record cut off before its name field) can only be caught by reading it twice and comparing. Identity requests and note traffic alongside a scan cost only retries.
* **Dense probing freezes the synth**: many different message types at intervals of 120 ms or less (the reverse-engineering sweeps) froze it reproducibly; 150 ms was safe. Nothing in normal use comes near this.

## Program Messages

Overall contents match the "[Preset Structure Document](https://cdn.mediavalet.com/aunsw/musictribe/q3-BVIpZck-zi_hAUX2V2Q/McqR_H7pVkW5V21CKYq9Pg/Original/Preset-Structure-Document_BE_0718-ABF_PRO800.pdf%29)" provided by Behringer, *but* there is a major difference:
SysEx messages, as per the MIDI standard, can only carry values up to `0x7F` = 127. In order to set bytes to the max value of `0xFF`=255, Behringer inserts "overflow bytes" every 8 bytes to hold the most significant bit of the following bytes.

Note: Overflow bytes can appear in the middle of multi-byte values!


To calculate the full byte value of any byte, execute the following:
          
    byteValue = message.at(bytePosition)
    
    overflowBytePosition = (bytePosition / 8) * 8 // position in message
    overflowBitPosition = (bytePosition % 8) - 1) // bit position in overflow byte
    
    overflowValue = message.at(overflowBytePosition)
    overflowValue = (overflowValue & (1 << overflowBit)) ? 1 : 0;
    
    resultValue = byteValue | (uint8_t)(overflowValue << 7);

### Message Structure

Two things about the table below that are easy to misread:

* **The "number of bytes" column counts *value* bytes, not positions.** A
  multi-byte value steps over any overflow byte inside it, so the 2-byte
  value listed at 15 occupies positions 15 and **17**, not 15 and 16.
* **Overflow bytes always fall on multiples of 8**, so they can be computed
  rather than looked up.

Records are also **variable length**: the synth truncates a message after its
last meaningful byte. In preset version 109 the name is the final field, so the
record ends right after it and its length tracks the name - a preset called
"Harp" comes back as 190 bytes where "Classical Brass" comes back as 202.
Versions 110 and 111 append fields *after* the name, so those records run to
their full length regardless. A shorter message is a normal record, not a
truncated or invalid one.

Two rules follow: **length says nothing about occupancy** (only a bare `F0 F7`
means an empty slot, see `0x77`), and **a preset may have no name at all** and
still be a preset. The longest record each version can produce (payload after
the address bytes, overflow bytes included) is:

| Preset version | Last field | Max payload bytes | Complete message |
|--|--|--|--|
| 109 | preset name (ends at 189) | 190 | 202 |
| 110 | LFO Aftertouch Amount (190..191) | 192 | 204 |
| 111 | Pitchbend Range (196..197) | 198 | 210 (`ProgramMessage::PROGRAM_MESSAGE_SIZE`) |

A record longer than its own version allows cannot have come from the synth as
one message (see "Transport behaviour"). The synth writes the newest format when
it saves; format 109 is what factory presets arrive in.

**How the synth converts an older record** (measured: factory preset A00 "Organ I",
version 109, 193 bytes, stored through the front panel to A99 and both dumped):
every field the old record had is kept byte for byte, the version byte becomes
111, the record runs to its full 210 bytes, and the fields the old layout did
not have are filled as follows:

| Field | Value the synth writes |
|--|--|
| LFO Aftertouch Amount | 0 |
| Voice Spread Enable | 0 (off) |
| Key Tracking Ref Note | 3 (C4) |
| Glide Mode | 0 (time) |
| Pitchbend Range | 24576 = 12 × 2048 (12 semitones, see the version 111 table) |

The front-panel save also blanks the name (a known firmware issue). The plugin
converts an older record the same way when it reads one
(`ProgramMessage::upgradeOlderPresetVersion()`, values in
`PRO800_PROGRAM_UPGRADE_DEFAULTS`), except that it keeps the name; so a record
read, upgraded and written back sounds like one the synth converted itself, and
an exported `.syx` holds the converted record, not the bytes the synth sent.

The message structure is the following:
* Pro800 header as described above
* LSB of program number
* MSB of program number

Then, counting again from 0:

|byte number | number of bytes | description |
|--|--|--|
|0 | 1 | *overflow byte*
|1 | 4 | Storage Code
|5 | 1 | Program version
|6 | 2 | Osc A Frequency
|8 | 1 | *overflow byte*
|9 | 2 | Osc A Level
|11 | 2 | Osc A Pulse Width
|13 | 2 | Osc B Frequency
|15 | 2 | Osc B Level
|16 | 1 | *overflow byte*
|18 | 2 | Osc B Pulse Width
|20 | 2 | Osc B Fine Frequency
|22 | 2 | Filter Cutoff
|24 | 1 | *overflow byte*
|25 | 2 | Filter Resonance
|27 | 2 | Filter Envelope Amount
|29 | 2 | Filter Release
|31 | 2 | Filter Sustain
|32 | 1 | *overflow byte*
|34 | 2 | Filter Decay
|36 | 2 | Filter Attack
|38 | 2 | Amp Release
|40 | 1 | *overflow byte*
|41 | 2 | Amp Sustain
|43 | 2 | Amp Decay
|45 | 2 | Amp Attack
|47 | 2 | Poly-Mod Source Filter Env
|48 | 1 | *overflow byte*
|50 | 2 | Poly-Mod Source Osc B
|52 | 2 | LFO Frequency
|54 | 2 | LFO Amount
|56 | 1 | *overflow byte*
|57 | 2 | Glide Amount
|59 | 2 | Amp Velocity
|61 | 2 | Filter Velocity
|63 | 1 | Osc A Shape Saw
|64 | 1 | *overflow byte*
|65 | 1 | Osc A Shape Tri
|66 | 1 | Osc A Shape Rect
|67 | 1 | Osc B Shape Saw
|68 | 1 | Osc B Shape Tri
|69 | 1 | Osc B Shape Rect
|70 | 1 | Osc A Sync
|71 | 1 | Poly-Mod Dest Freq A
|72 | 1 | *overflow byte*
|73 | 1 | Poly-Mod Dest Filter
|74 | 1 | LFO Shape
|75 | 1 | LFO Speed
|76 | 1 | LFO Destination
|77 | 1 | Filter Keyboard Tracking
|78 | 1 | Filter Envelope Shape
|79 | 1 | Filter Envelope Speed
|80 | 1 | *overflow byte*
|81 | 1 | Amp Envelope Shape
|82 | 1 | Poly-Mod Unison Track
|83 | 1 | Pitchbend Target
|84 | 1 | Mod Wheel Range
|85 | 1 | Osc A Freq Pot Mode
|86 | 1 | Osc B Freq Pot Mode
|87 | 2 | Modulation Delay
|88 | 1 | *overflow byte*
|90 | 2 | Vibrato Freq
|92 | 2 | Vibrato Amount
|94 | 2 | Unison Detune
|96 | 1 | *overflow byte*
|97 | 1 | Mod Wheel Target
|98 | 1 | reserved
|99 | 1 | Voice 1 Offset
|100 | 1 | Voice 2 Offset
|101 | 1 | Voice 3 Offset
|102 | 1 | Voice 4 Offset
|103 | 1 | Voice 5 Offset
|104 | 1 | *overflow byte*
|105 | 1 | Voice 6 Offset
|106 | 1 | Voice 7 Offset
|107 | 1 | Voice 8 Offset
|108 | 4 | Tune Per Note - C
|112 | 1 | *overflow byte*
|113 | 4 | Tune Per Note - C#
|117 | 4 | Tune Per Note - D
|120 | 1 | *overflow byte*
|122 | 4 | Tune Per Note - D#
|126 | 4 | Tune Per Note - E
|128 | 1 | *overflow byte*
|131 | 4 | Tune Per Note - F
|135 | 4 | Tune Per Note - F#
|136 | 1 | *overflow byte*
|140 | 4 | Tune Per Note - G
|144 | 1 | *overflow byte*
|145 | 4 | Tune Per Note - G#
|149 | 4 | Tune Per Note - A
|152 | 1 | *overflow byte*
|154 | 4 | Tune Per Note - A#
|158 | 4 | Tune Per Note - B
|160 | 1 | *overflow byte*
|163 | 2 | Noise Amount
|165 | 2 | Amp Aftertouch Amount
|167 | 2 | Filter Aftertouch Amount
|168 | 1 | *overflow byte*
|170 | 1 | Amp Envelope Speed
|171 | 1 | ARP Mode
|172 |   | first char of preset name (16 characters, ending at 189)
|176 | 1 | *overflow byte*, not used by name
|184 | 1 | *overflow byte*, not used by name
|189 |   | last char of preset name

only in preset version 110 and newer:
|byte number | number of bytes | description |
|--|--|--|
|190 | 2 | LFO Aftertouch Amount
|192 | 1 | *overflow byte*

only in preset version 111 and newer:

|byte number | number of bytes | description |
|--|--|--|
|193 | 1 | Voice Spread Enable
|194 | 1 | Key Tracking Ref Note
|195 | 1 | Glide Mode
|196 | 2 | Pitchbend Range<br> --> 0-31 semitones, stored scaled to 0-65535 like the other 2-byte parameters (24 semitones = 50737). Note: the `0x11` message writes the same field as `param × 2048` (24 semitones = 49152, RE §2), and so does the synth itself when it converts an older record (12 semitones = 24576, see "How the synth converts an older record") — two writers, two scales; the CC path's 50737 is the one to re-measure


## Settings messages

Settings messages have the same overall structure as program messages in terms of header and overflow bytes. They are also requested with a `0x77` message, specifically with the address parameters`7e 03` (address 510), and answered with a `0x78` at that address: 46 payload bytes, a 58-byte message (`SettingsMessage::SETTINGS_MESSAGE_SIZE`).

The contents are the following. Ranges marked "on the front panel" are what the synth's own UI allows and what this plugin's settings tab enforces. Via SysEx the synth does not validate: it stores whatever arrives, reads it back unchanged and usually even displays it (see the individual rows for what the value then does).

|byte number | number of bytes | description |
|--|--|--|
|0 | 1 | *overflow byte*
|1 | 5 | unknown (probably settings version number/storage code: one captured block holds `25 16 61 00` followed by `111`, the same shape as a preset's storage code and version)
|6 | 2 | Current Preset Number<br> --> the **full 0-399 number** of the preset the synth is on (143 = B43), as the synth itself stores it. Only the value modulo 100 is used, the bank comes from byte 23. Not validated: 560 is stored, kept across a power cycle and shown as D60. With byte 23 this is the **selection pointer**, see below
|8 | 1 | *overflow byte*
|9 | 1 | Preset Mode<br> --> records that a Program Change selected the current preset: reads `1` in every state firmware 1.4.6 produces, including manual mode and after front-panel edits (0 = "manual" and 2 = "edited", as documented earlier, are never written by it). Freely writable and kept, but ignored. **It is not an edit flag**; nothing readable over MIDI says whether the current preset has unsaved edits
|10 | 1 | MIDI RX Channel<br> --> 0 = ALL, 1 = dip switches, - [2-17] = channel [1-16], 18 = OFF. The DIP position can be resolved with `0x70` indices `28-2B` (the sum of the readings is the 0-based channel). Not validated: any byte is stored and kept, and **every value above 18 behaves as OFF** (the synth is deaf) while the front panel displays a meaningless channel number for it. Closing the front-panel menu for this setting writes the displayed value back over whatever was sent
|11 | 1 | Voice Kill (one bit per voice, bit 0 = voice 1)<br> --> despite the name a **set bit means the voice is enabled**: the plugin's Voice Enable checkboxes match the synth's own menu (measured)
|12 | 1 | MIDI TX Channel<br> --> 0 = THRU, 1 = dip switches, [2-17] = channel [1-16]
|13 | 1 | Sync Source<br> --> 0 = internal, 1 = MIDI, 2 = USB, 3 = external
|14 | 1 | unknown
|15 | 2 | Sync Clock BPM × 10 (1200 = 120.0 BPM) — the two value bytes are positions 15 and **17**, stepping over the overflow byte at 16 (see "easy to misread" above); there is no separate field at 17
|16 | 1 | *overflow byte*
|18 | 1 | unknown — written by `0x1A` (0-3)
|19 | 1 | Display Brightness<br> --> range: 1-16. Any value 0-127 can be set via SysEx, is read back unchanged and is shown on the display, but the actual brightness stays within what 1-16 give (a stored 0 was observed as full brightness). RE §6 lists 0-16; to be aligned
|20 | 1 | Display Parameter Time<br> --> range: 0-100
|21 | 1 | MIDI CC Mode<br> --> 0 = OFF, 1 = TX, 2 = RX, 3 = TX&RX 
|22 | 1 | MIDI PC Mode<br> --> 0 = OFF, 1 = TX, 2 = RX, 3 = TX&RX 
|23 | 1 | Current Bank<br> --> 0-3 = A-D, the bank of the preset the synth is on. Writing it alone moves the selection (B43 → D43); byte 6 and this byte do not cross-validate each other
|24 | 1 | *overflow byte*
|25 | 1 | Sync In Forward Enable<br> --> 0 = OFF, 1 = ON
|26 | 2 | External CV Amount
|28 | 1 | unknown
|29 | 1 | Sync Clock Subdivision<br> --> 0 = 1/4, 1 = 1/4T, 2 = 1/8, 3 = 1/8T, 4 = 1/16, 5 = 1/16T, 6 = 1/32, 7 = 1/32T
|30 | 1 | Voice Priority<br> --> 0 = last, 1 = low, 2 = high
|31 | 1 | Show Preset Name<br> --> 0 = OFF, 1 = ON
|32 | 1 | *overflow byte*
|33 | 1 | Sync In Polarity<br> --> 0 = rise, 1 = fall, 2 = both
|34 | 1 | unknown
|35 | 1 | Tuner Precision<br> --> 0 = 0.5ct, 1 = 1.0ct, 2 = 1.5ct, 3 = 2.0ct
|36 | 1 | Sync In Start-Stop<br> --> 0 = OFF, 1 = ON
|37 | 1 | Sync In PPQN<br> --> 0 = 1PPS, 1 = 1PPQN, 2 = 2PPQN, 3 = 4PPQN, 4 = 24PPQN, 5 = 48PPQN
|38 | 1 | Sync Clock Note Length<br> --> range: 1-100 on the front panel. Any value 0-127 can be set via SysEx, is read back unchanged and is shown on the display; whether values outside 1-100 have an audible effect is unknown. RE §6 lists 5-100; to be aligned
|39 | 1 | Sync Clock Swing<br> --> range: 50-95 on the front panel. Any value 0-127 can be set via SysEx, is read back unchanged and is even shown on the display; whether values outside 50-95 have an audible effect is unknown. RE §6 lists 5-95; to be aligned
|40 | 1 | overflow — its two highest bits govern value bytes that do not exist (the block ends at 45) and read 0; patch fields in place rather than decoding and re-encoding the whole block, which is what `Pro800DataMessage::setValue()` does
|41 | 1 | Aftertouch VCA Polarity<br> --> 0 = rise, 1 = fall, 2 = both
|42 | 1 | Aftertouch VCF Polarity<br> --> 0 = rise, 1 = fall, 2 = both
|43 | 1 | Transpose<br> --> range: -35 - +35, two's complement (-1 = 0x7F with the overflow bit set). Values outside that range are stored and read back unchanged via SysEx (e.g. 37), but the front panel display clamps them to +/-35. The `0x0F` message reaches only -12..+12 of it
|44 | 1 | Local Enable<br> --> 0 = OFF, 1 = ON
|45 | 1 | Soft Thru<br> --> 0 = OFF, 1 = ON

### Selecting a preset via the settings block

Bytes 6 and 23 together are the synth's **selection pointer**, and writing them is a way to select a preset that needs no MIDI channel at all (unlike bank select + program change, see [Pro800CCMessages.md](Pro800CCMessages.md)). It is what the plugin's *Load* does (`SynthSession::selectProgram()`, `SettingsMessage::setCurrentProgram()`):

1. read the block, set `Current Preset Number` to the full program number and `Current Bank` to its bank, write the block back **in one write** (the synth tolerates an inconsistent pair, but never leave one behind);
2. read the block back until both fields show the new values — polling for up to 2.5 s, not once (see "Transport behaviour");
3. send `0x32 00`.

**Moving the pointer is not recalling the preset.** After step 1 the display and every readable field show the new preset while the voice engine keeps playing the old one (SynthTribe behaves the same way). Only step 3 — or a channel-voice Program Change, a Preset button press, or a power cycle — makes the synth load the record into the voice engine, and it discards unsaved edits when it does. Nothing readable over MIDI reports whether a recall happened: the `0x72` knob values do not follow a preset load, and the synth sends nothing when one happens.

Reading the block is also how a host learns which preset the synth is on; the synth does not announce a selection made on its front panel, so the plugin polls the block every 1.5 s.
