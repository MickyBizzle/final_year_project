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
  
  output = createWriter("output.txt"); 

  PostRequest post = new PostRequest("http://svmib26.dcs.aber.ac.uk/webapp/public/add_data");
  post.send();
  output = createWriter("output.txt");
  output.println(post.getContent());
  println(post.getContent());
}


void draw() {
  if (inString != null) {
    sendData(inString);
    inString = null;
    //int chars = inString.length();
    //String inB2 = inString.substring(0, chars-2);
    //dat = int(split(inB2, ','));
    //printArray(dat);
    //println();
    //inString = null;
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
}