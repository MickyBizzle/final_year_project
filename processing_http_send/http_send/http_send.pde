import processing.serial.*;
import http.requests.*;

Serial port;
String inString;
int lf = 10;
int dat[];

void setup() {
  printArray(Serial.list());
  println();
  port = new Serial(this, Serial.list()[0], 115200);
  port.bufferUntil(lf);
}

void draw() {
  GetRequest get = new GetRequest("http://svmib26.dcs.aber.ac.uk/webapp/public/add_data");
  get.send();
  println("Response: " + get.getContent());
  
  
  
  
  
  //println("hello");
  //if (inString != null) {
  //  int chars = inString.length();
  //  String inB2 = inString.substring(0, chars-2);
  //  dat = int(split(inB2, ','));
  //  printArray(dat);
  //  println();
  //  inString = null;
  //}
  //else {
  //  println("no data");
  //}
}

void serialEvent(Serial p) {
  inString = p.readString();
}