import processing.serial.*;
import http.requests.*;

Serial port;
String inString;
int lf = 10;
int dat[];

PrintWriter output;


void setup() {
  printArray(Serial.list());
  println();
  port = new Serial(this, Serial.list()[0], 115200);
  port.bufferUntil(lf);
  
  // Save output to file for debugging
  output = createWriter("output.txt"); 
}


void draw() {
  if (inString != null) {
    sendData(inString);
    inString = null;
  }
}


void serialEvent(Serial p) {
  inString = p.readString();
}


void sendData(String data) {
  PostRequest post = new PostRequest("http://svmib26.dcs.aber.ac.uk/webapp/public/add_data");
  post.addData("data", data);
  post.send();
  println(post.getContent());
  output.println(post.getContent());
}