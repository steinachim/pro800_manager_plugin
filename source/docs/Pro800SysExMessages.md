
# Pro-800 SysEx Messages

## Overall message structure
    0xF0                   // MIDI standard, start of SysEx
   
    0x00, 0x20, 0x32,      // Brand ID (Behringer)
    0x00, 0x01, 0x24,      // Product ID (Pro-800)
    0x00                   // CPU ID
	 
    0xZZ                   // message type
                              -> defines type of following message data (see below)
   
    (optional) command parameters (normally up to 2 params)

    0xF7                   // MIDI standard, end of SysEx

Often, if a request requires dedicated response, the command bytes will be sequential (e.g. `0x77` is answered by a `0x78` message) or a generic status message (type `0x01`, see below)

## Known message types
|message type|parameters | description |
|--   |--|--|
|0x01 |status | Status Message: parameter  is `0x00` on success, `0x01` on failure
|0x02 |-  | Unknown (responds with `03 00`)
|0x03 |-  | Unknown (responds with Status OK)
|0x04 |-  | Unknown (responds with `05 50 30 45 39 49 00`)
|0x06 |-  | Request device name (responds with `07 50 52 4f 2d 38 30 30 00` - ASCII: "PRO-800"
|0x08 |version1, version2, version3 | Request firmware version:<br> complete version = "version1.version2.version3"
|0x0E |-  | Unknown (responds with Status OK)
|0x0F | [00-18] |Unknown (responds with Status OK in that range, Status Error otherwise)
|0x11 | [00-1F] |Unknown (responds with Status OK in that range, Status Error otherwise)
|0x12 | [00-02] |Unknown (responds with Status OK in that range, Status Error otherwise)
|0x13 | [00-07] |Unknown (responds with Status OK in that range, Status Error otherwise)
|0x17 | [00-01] |Unknown (responds with Status OK in that range, Status Error otherwise)
|0x19 | [00-02] |Unknown (responds with Status OK in that range, Status Error otherwise)
|0x1A | [00-04] |Unknown (responds with Status OK in that range, Status Error otherwise)
|0x1B | [00-03] |Unknown (responds with Status OK in that range, Status Error otherwise)
|0x1C | [00-05] |Unknown (responds with Status OK in that range, Status Error otherwise)
|0x1D | - |Unknown (responds with Status OK)
|0x32 | [00-7F] | - parameter 00: Reset default mode and to saved preset settings (?)<br> - any other parameter: display 8888, then keeps dots enabled in display and responds strangely to controls. MIDI still seems to work. Some debug mode maybe?<br> - always responds with Status OK
|0x35 | - | Unknown (responds with Status OK)
|0x36 |[00-7F] | - parameter 00: appears to lock synth<br> - any other parameter: unlocks it again<br> - always responds with Status OK
|0x37 |[00-7F] | - parameter 00: Turn off all button lights<br> - any other parameter: Lights up all buttons<br> - always responds with Status OK
|0x50 |[0x00-0x7F], [00-01] | Unknown (responds with Status OK in that range, Status Error otherwise)
|0x71 | [00-17] | "push" buttons:<br>  - 00-09 = numpad<br> - 0A/0B = ??? - disable performance / settings if active<br> - 0C = preset<br> - 0D = rec<br> - 0E = perf<br> - 0F = settings<br> - 10 = seq 1<br>- 11 = seq 2<br>- 12 = ?? (in performance mode, disable perf mode; in settings mode, go to Tune)<br>- 13 = sync clock<br> - 14 = sync source<br> - 15 = ??<br> - 16 = Next Setting (value wheel?)<br> - 17 = Prev Setting (value wheel?)
|0x72 | [00-19] |  Unknown (get value of param 1 - responds with 73 [param 1] [00-ff])<br>Example values (param 1, param2):<br> `00, 3e ( 62) - 01, 42 ( 66) - 02, 50 ( 80) - 03, 42 ( 66)`<br>`04, 3e ( 62) - 05, 40 ( 64) - 06, 6c (108) - 07, 39 ( 57)`<br>`08, 7f (127) - 09, 7f (127) - 0a, 26 ( 38) - 0b, 45 (117)` <br> `0c, 5d ( 93) - 0d, 41 ( 65) - 0e, 50 ( 80) - 0f, 32 ( 50)`<br>`10, 58 ( 88) - 11, 3a ( 58) - 12, 37 ( 55) - 13, 3f ( 63)`<br>`14, 68 (104) - 15, 00 (  0) - 16, 7e (126) - 17, 51 ( 81)`<br>`18, 53 ( 83)  - 19, 7f (127)`
|0x73 | [00-19], [00-7F] | Unknown (set param1 to value of param 2), responds with status OK if values in range
|0x77| [00-7F], [00-03] | Request program dump<br> param1 = program lsb,<br> param2 = program msb<br> -> response with 73 (see below)<br> -> note: 7e 03 = settings message
|0x78 |[00-7F], [00-03], [...] | Set program to value<br> param1 = program lsb,<br>param2 = program msb,<br>following bytes = program data (may be empty -> set to "uninitialized")
|0x7D | - | Factory Reset (no confirmation!)

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
|86 | 1 | Osc A Freq Pot Mode
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
|142 | 1 | *overflow byte*
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
|173 |   | first char of preset name
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
|196 | 2 | Pitchbend Range


## Settings messages

Settings messages have the same overall structure as program messages in terms of header and overflow bytes. They are also requested with a `0x77` message, specifically with the address parameters`7e 03`.

The contents are the following:

|byte number | number of bytes | description |
|--|--|--|
|6 | 2 | Current Preset Number
|8 | 1 | *overflow byte*
|9 | 1 | Preset Mode<br> --> 0 = MANUAL, 1 = LOADED, 2 = EDITED
|10 | 1 | MIDI RX Channel<br> --> 0 = ALL, 1 = dip switches, - [2-17] = channel [1-16], 18 = OFF
|11 | 1 | Voice Kill (one bit per voice)
|12 | 1 | MIDI TX Channel<br> --> 0 = THRU, 1 = dip switches, [2-17] = channel [1-16]
|13 | 1 | Sync Source<br> --> 0 = internal, 1 = MIDI, 2 = USB, 3 = external
|14 | 1 | unknown
|15 | 2 | Sync Clock BPM
|16 | 1 | *overflow byte*
|17 | 1 | unknown
|18 | 1 | unknown
|19 | 1 | Display Brightness<br> --> range: 0-16
|20 | 1 | Display Parameter Time<br> --> range: 0-100
|21 | 1 | MIDI CC Mode<br> --> 0 = OFF, 1 = TX, 2 = RX, 3 = TX&RX 
|22 | 1 | MIDI PC Mode<br> --> 0 = OFF, 1 = TX, 2 = RX, 3 = TX&RX 
|23 | 1 | unknown
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
|38 | 1 | Sync Clock Note Length<br> --> range: 5-100
|39 | 1 | Sync Clock Swing<br> --> range: 5-95
|40 | 1 | overflow
|41 | 1 | Aftertouch VCA Polarity<br> --> 0 = rise, 1 = fall, 2 = both
|42 | 1 | Aftertouch VCF Polarity<br> --> 0 = rise, 1 = fall, 2 = both
|43 | 1 | Transpose"<br> --> range: -12 - +12
|44 | 1 | Local Enable<br> --> 0 = OFF, 1 = ON
|45 | 1 | Soft Thru<br> --> 0 = OFF, 1 = ON
