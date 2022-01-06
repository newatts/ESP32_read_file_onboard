//to read another file onboarded to the ESP it must be saved in a new folder called 'data' with the file named anything that you want.
// the said file must first be onboarded to the ESP by:
//1.  using VSCode, go the PIO icon, > Platform > select 'Build Filesystem Image' followed by selecting 'Upload FileSystem Image'

#include <Arduino.h>
#include "SPIFFS.h"
void readAnotherFileOnESP(void); 

 
void setup() {
  Serial.begin(9600);
  //chcek the file isa actually there before you try to use it!  
  readAnotherFileOnESP();   //be sure to change the file name to match that in data folder!!!!!
}
 
void loop() {

}

void readAnotherFileOnESP()  //code to actually read a file.
{

   delay(2000);
  Serial.println("your file: ");
   if(!SPIFFS.begin(true)){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
  
  File file = SPIFFS.open("/text.txt");    //adjust folder name as required, dont forget / for the folder path
  if(!file){
    Serial.println("Failed to open file for reading");
    return;
  }
  
  Serial.println("File Content:");
  while(file.available()){
    Serial.write(file.read());
  }
  file.close();


}
