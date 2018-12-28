/*
  Serial PDU Encoder/Decoder
 
 Encodes a line of readable text to PDU (ascii/hex) text.
 Also decodes a line of PDU hex text to 7-bits readable text.
 Entered text must be terminate by a newline or a carriage return or both characters.
 Default as PDU decoder.
 Enter '\E' to change to encoder mode and print result in hex text.
 Enter '\e' to change to encoder mode and print result with original ascii text.
 Enter '\D' to change to decoder mode.
 It's recognize '\t', '\n', '\r', and '\\' characters.
 Any single character after '\' other than 7 characters above will be ignored.
 
 Created 19 August 2017
 @Gorontalo, Indonesia
 by ZulNs
 
 This example code is in the public domain.
 */

char strBuffer[9]; // for storing max 8 chrs plus null termination chr
uint16_t wordData;
byte counter;
byte chrCtr = 0;
byte blockCtr = 0;
boolean isAvoided = false;
boolean isEncoding = false;
boolean isEncode2Hex;
boolean isSpecialChr = false;
boolean isSecondByte = false;
boolean isFirstLine = true;

void setup() {
  Serial.begin(115200);    // the GPRS baud rate 
  Serial.println(F("*** Serial PDU Encoder/Decoder by ZulNs ***"));
  Serial.println(F("\n[ Decode PDU Mode ]\n"));
  strBuffer[8] = 0;
}

void loop() {
  byte chr;
  boolean isHexChr;
  while (Serial.available()) {
    chr = Serial.read();
    if (chr == '\n' || chr == '\r') {
      endCodec();
      isAvoided = false;
      isFirstLine = true;
      break;
    }
    if (isAvoided)
      continue;
    if (chr == '\\' && !isSpecialChr) {
      isSpecialChr = true;
      continue;
    }
    if (isSpecialChr) {
      isSpecialChr = false;
      if (isFirstLine) {
        isFirstLine = false;
        if (chr == 'D') {
          isEncoding = false;
          Serial.println(F("\n[ Decode PDU Mode ]\n"));
          continue;
        }
        else if (chr == 'E') {
          isEncoding = true;
          isEncode2Hex = true;
          Serial.println(F("\n[ Encode to PDU Hex ]\n"));
          continue;
        }
        else if (chr == 'e') {
          isEncoding = true;
          isEncode2Hex = false;
          Serial.println(F("\n[ Encode to PDU ASCII ]\n"));
          continue;
        }
      }
      isFirstLine = false;
      if (chr == 't')
        chr = '\t';
      else if (chr == 'n')
        chr = '\n';
      else if (chr == 'r')
        chr = '\r';
      else if (chr != '\\')
        continue;
    }
    isFirstLine = false;
    if (isEncoding) {
      strBuffer[blockCtr++] = chr;
      if (blockCtr == 8)
        codecFull();
    }
    else {
      // decoder mode
      if (chr >= 'a' && chr <= 'f')
        chr -= 32; // make upper case
      isHexChr = chr >= '0' && chr <= '9' || chr >= 'A' && chr <= 'F';
      if (isHexChr) {
        chr -= 48;
        if (chr > 9)
          chr -= 7;
        if (isSecondByte) {
          strBuffer[blockCtr++] |= chr;
          if (blockCtr == 7)
            codecFull();
        }
        else
          strBuffer[blockCtr] = chr << 4;
        isSecondByte = !isSecondByte;
      }
      else {
        endCodec();
        isAvoided = true;
      }
    }
  }
}

void endCodec() {
  if (blockCtr > 0)
    codecLess();
  if (chrCtr > 0) {
    Serial.print(F("\nLength: "));
    Serial.print(chrCtr, DEC);
    Serial.print(F(" chrs\n\n"));
    chrCtr = 0;
  }
  isSpecialChr = false;
  isSecondByte = false;
}

void codecLess() {
  strBuffer[blockCtr] = 0;
  codecPdu();
  chrCtr += blockCtr;
  blockCtr = 0;
}

void codecFull() {
  codecPdu();
  chrCtr += 8;
  blockCtr = 0;
}

void codecPdu() {
  if (isEncoding)
    encode2Pdu();
  else
    decodePdu();
  if (isEncoding && isEncode2Hex)
    printHex();
  else
    Serial.print(strBuffer);
}

void printHex() {
  counter = 0;
  while (strBuffer[counter] != 0) {
    Serial.print((byte)strBuffer[counter] >> 4, HEX);
    Serial.print((byte)strBuffer[counter++] & 0x0F, HEX);
  }
}

void printStr() {
  counter = 0;
  while (strBuffer[counter] != 0)
    print2Console(strBuffer[counter++]);
}

void print2Console(char chr) {
  if (chr == '\t')
    Serial.print(F("\\t"));
  else if (chr == '\n')
    Serial.print(F("\\n"));
  else if (chr == '\n')
    Serial.print(F("\\r"));
  Serial.print(chr);
}

void encode2Pdu() {
  counter = 0;
  while (strBuffer[counter + 1] != 0) {
    wordData = strBuffer[counter + 1] << 8;
    wordData >>= counter + 1;
    strBuffer[counter] |= lowByte(wordData);
    strBuffer[++counter] = highByte(wordData);
  }
}

void decodePdu() {
  counter = 0;
  while (strBuffer[counter] != 0) {
    wordData = strBuffer[counter] << 8 | lowByte(wordData);
    wordData >>= 8 - counter;
    strBuffer[counter] = lowByte(wordData) & 0x7F;
    wordData >>= counter++;
    if (counter == 7) {
      strBuffer[counter] = lowByte(wordData) >> 1;
      return;
    }
  }
}

