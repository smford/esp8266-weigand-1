bool findUser(String foundrfid) {
  //Serial.print(foundrfid); Serial.print(": ");
  if (userlist.indexOf(foundrfid) >= 0) {
    //Serial.println("Card found");
    return true;
  } else {
    //Serial.println("Unknown card");
    return false;
  }
}

/*
// https://stackoverflow.com/questions/111928/is-there-a-printf-converter-to-print-in-binary-format
void print_byte(uint8_t byte) {
    Serial.printf("Binary: %s%s\n", bit_rep[byte >> 4], bit_rep[byte & 0x0F]);
}

// https://stackoverflow.com/questions/111928/is-there-a-printf-converter-to-print-in-binary-format
void print_hex(uint8_t byte) {
    Serial.printf("Hex: %s%s\n", bit_hex[byte >> 4], bit_hex[byte & 0x0F]);
}
*/

/*
void readablerfid(char* temp) {
  char rfidstring[strlen(temp)*2];
  char tempchar[2];
  Serial.print("length="); Serial.println(strlen(temp));
  for (int i = 0; i < strlen(temp); i++) {
    sprintf(tempchar, "%x%x", temp[i] >> 4, temp[i] & 0x0F);
    strcat(rfidstring, tempchar);
  }
  for (int i = 0; i < strlen(rfidstring); i++) {
    rfidstring[i] = toupper(rfidstring[i]);
  }
  Serial.println(rfidstring);
}
*/

/*
const char* readablerfid2(char* temp) {
  char rfidstring[strlen(temp)*2];
  char tempchar[2];
  Serial.print("length="); Serial.println(strlen(temp));
  for (int i = 0; i < strlen(temp); i++) {
    sprintf(tempchar, "%x%x", temp[i] >> 4, temp[i] & 0x0F);
    strcat(rfidstring, tempchar);
  }
  for (int i = 0; i < strlen(rfidstring); i++) {
    rfidstring[i] = toupper(rfidstring[i]);
  }
  Serial.print("readablerfid2=");Serial.println(rfidstring);
  return rfidstring;
}
*/

/*
  char * readablerfid3(char* temp) {
  char * rfidstring = (char *) malloc(strlen(temp)*2);
  //char rfidstring[strlen(temp)*2];
  char tempchar[2];
  Serial.print("length="); Serial.println(strlen(temp));
  for (int i = 0; i < strlen(temp); i++) {
    sprintf(tempchar, "%x%x", temp[i] >> 4, temp[i] & 0x0F);
    strcat(rfidstring, tempchar);
  }
  for (int i = 0; i < strlen(rfidstring); i++) {
    rfidstring[i] = toupper(rfidstring[i]);
  }
  Serial.print("readablerfid3=");Serial.println(rfidstring);
  return rfidstring;
}
*/

String readablerfid4(char* temp) {
  char * rfidstring = (char *) malloc(strlen(temp)*2);
  //char rfidstring[strlen(temp)*2];
  char tempchar[2];
  //Serial.print("length="); Serial.println(strlen(temp));
  for (int i = 0; i < strlen(temp); i++) {
    sprintf(tempchar, "%x%x", temp[i] >> 4, temp[i] & 0x0F);
    strcat(rfidstring, tempchar);
  }
  for (int i = 0; i < strlen(rfidstring); i++) {
    rfidstring[i] = toupper(rfidstring[i]);
  }
  //Serial.print("readablerfid4=");Serial.println(rfidstring);
  return rfidstring;
}

void spiffslog(String string1, String message) {
  //Serial.print(string1); Serial.println(": " + message);
  if (spiffsActive) {
    if (SPIFFS.exists(logfilename)) {
      File f = SPIFFS.open(logfilename, "a");
      if (!f) {
        Serial.print("ERROR: Unable To Open '"); Serial.print(logfilename); Serial.println("' for Logging");
        //Serial.println();
      } else {
        f.println(string1 + " " + message);
        f.close();
      }
    } else {
      Serial.print("ERROR: Cannot find '"); Serial.print(logfilename); Serial.println("' for Logging");
    }
  } else {
    Serial.println("ERROR: SPIFFS not active, cannot write log");
  }
}

void unlockDoor(String rfidcard) {
  syslog.logf(LOG_INFO, "Door:Open Card=%s", rfidcard.c_str());
  // fire relay for X seconds
}

void lockDoor(String rfidcard) {
  syslog.logf(LOG_INFO, "Door:Lock Card=%s", rfidcard.c_str());
  // ensure that relay is NOT fired
}

void printLogs() {
  if (spiffsActive) {
    if (SPIFFS.exists(logfilename)) {
      File f = SPIFFS.open(logfilename, "r");
      if (!f) {
        Serial.print("Unable To Open '"); Serial.print(logfilename); Serial.println("' for Reading");
        Serial.println();
      } else {
        String s;
        Serial.print("Contents of file '"); Serial.print(logfilename); Serial.println("'");
        Serial.println();
        while (f.position() < f.size())
        {
          s = f.readStringUntil('\n');
          s.trim();
          Serial.println(s);
        }
        f.close();
      }
      Serial.println();
    }
  }
}

/*
int convertFromHex(int ascii){ 
  if(ascii > 0x39) ascii -= 7; // adjust for hex letters upper or lower case
  ascii = ascii >> 4;
  //char temp[5];
  //sprintf(temp, "%x", &ascii);
  //return(temp & 0xf);
  //return(ascii & 0xf);
}
*/

/*
String giveback(int ascii) {
  String temp;
  temp = (ascii >> 4) + (ascii & 0xf);
  Serial.print("giveback= "); Serial.println(temp);
  return temp;
}
*/
