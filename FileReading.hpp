/*
 ESP8266 read and write a text file by: martinayotte and Steve Nelson (it was mostly Martin! I just documented it)
 This will read and write a file to the SPIFFS file system. This particular example will write "hello world" into a text file
 that is saved in the device's file system. It then reads the file and display the words to the serial monitor.
 The file tab contains a couple functions that simplifies the process of reading and writing into a couple functions that
 handle the casting and pointers.
  
 It has been tested on an esp12e (NodeMCU development board)
 This example code is in the public domain
  Prerequisite Examples:
  https://github.com/zenmanenergy/ESP8266-Arduino-Examples/tree/master/helloworld_serial
*/
#include "FS.h"
boolean fileWrite(String name, String content);
String fileRead(String name);
boolean fileRemove(String name);

boolean fileWrite(String name, String content){
  SPIFFS.begin();
  //open the file for writing.
  //Modes:
  //"r"  Opens a file for reading. The file must exist.
  //"w" Creates an empty file for writing. If a file with the same name already exists, its content is erased and the file is considered as a new empty file.
  //"a" Appends to a file. Writing operations, append data at the end of the file. The file is created if it does not exist.
  //"r+"  Opens a file to update both reading and writing. The file must exist.
  //"w+"  Creates an empty file for both reading and writing.
  //"a+"  Opens a file for reading and appending.:

  //choosing w because we'll both write to the file and then read from it at the end of this function.
  File file = SPIFFS.open(name.c_str(), "w");

  //verify the file opened:
  if (!file) {
    //if the file can't open, we'll display an error message;
    String errorMessage = "Can't open '" + name + "' !\r\n";
    Serial.println(errorMessage);
    return false;
  } else{
    //To all you old school C++ programmers this probably makes perfect sense, but to the rest of us mere mortals, here is what is going on:
    //The file.write() method has two arguments, buffer and length. Since this example is writing a text string, we need to cast the
    //text string (it's named "content") into a uint8_t pointer. If pointers confuse you, you're not alone! 
    //I don't want to go into detail about pointers here, I'll do another example with casting and pointers, for now, this is the syntax
    //to write a String into a text file.
    file.write((uint8_t *)content.c_str(), content.length());
    file.close();
    return true;
  }
}

String fileRead(String name){
  SPIFFS.begin();
  //read file from SPIFFS and store it as a String variable
  String contents;
  File file = SPIFFS.open(name.c_str(), "r");
  if (!file) {
    String errorMessage = "Can't open '" + name + "' !\r\n";
    Serial.println(errorMessage);
    return "FILE ERROR";
  }
  else {
    
    // this is going to get the number of bytes in the file and give us the value in an integer
    int fileSize = file.size();
    int chunkSize=128;
    //This is a character array to store a chunk of the file.
    //We'll store 1024 characters at a time
    char buf[chunkSize];
    int numberOfChunks=(fileSize/chunkSize)+1;
    
    int count=0;
    int remainingChunks=fileSize;
    for (int i=1; i <= numberOfChunks; i++){
      if (remainingChunks-chunkSize < 0){
        chunkSize=remainingChunks;
      }
      file.read((uint8_t *)buf, chunkSize-1);
      remainingChunks=remainingChunks-chunkSize;
      contents+=String(buf);
    }
    file.close();
    return contents;
  }
}


boolean fileRemove(String name){
  SPIFFS.begin();
  //read file from SPIFFS and store it as a String variable
  SPIFFS.remove(name.c_str());
  return true;
}
